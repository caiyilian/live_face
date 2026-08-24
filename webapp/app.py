# 过滤不必要的 warning——必须在任何 import 之前设置
import os
os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"
os.environ["TF_ENABLE_ONEDNN_OPTS"] = "0"

import sys, cv2, torch, numpy as np
import atexit
import base64
import re
import socket
import logging
import requests
import mediapipe as mp
from concurrent.futures import ThreadPoolExecutor
from PIL import Image, ImageDraw, ImageFont
from flask import Flask, render_template, Response, request, jsonify
from flask_sock import Sock
from zeroconf import Zeroconf, ServiceInfo, IPVersion

logging.getLogger("absl").setLevel(logging.ERROR)

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "openface3"))

from emotiefflib.facial_analysis import EmotiEffLibRecognizerTorch
from mediapipe.tasks.python.vision import FaceDetector, FaceDetectorOptions, RunningMode
from mediapipe.tasks.python import BaseOptions

app = Flask(__name__)
sock = Sock(app)

print("Loading EmotiEffLib model (CUDA)...")
model = EmotiEffLibRecognizerTorch(model_name="enet_b0_8_best_vgaf", device="cuda")
model.model.eval()
print("Model loaded.")

# MediaPipe Face Detection (CPU, 新版 tasks API)
model_path = os.path.join(os.path.dirname(__file__), "blaze_face_short_range.tflite")
options = FaceDetectorOptions(
    base_options=BaseOptions(model_asset_path=model_path),
    running_mode=RunningMode.IMAGE,
    min_detection_confidence=0.5,
)
face_detector = FaceDetector.create_from_options(options)

# 表情中文映射
EMOTION_CN = {
    "Neutral": "中性", "Happy": "开心", "Sad": "悲伤",
    "Surprise": "惊讶", "Fear": "恐惧", "Disgust": "厌恶",
    "Anger": "愤怒", "Contempt": "轻蔑",
    "Happiness": "开心", "Sadness": "悲伤",
    "Angry": "愤怒", "Surprised": "惊讶",
    "Fearful": "恐惧", "Disgusted": "厌恶",
}

# 中文字体（Windows 系统自带）
FONT_SIZE = 20
font_cn = None

# ESP8266 配置
ESP_IP = ""
EMOTION_TO_ESP = {
    "Anger": "ANGRY", "Angry": "ANGRY",
    "Contempt": "HAPPY",
    "Disgust": "DISGUST", "Disgusted": "DISGUST",
    "Fear": "FEAR", "Fearful": "FEAR",
    "Happiness": "HAPPY", "Happy": "HAPPY",
    "Neutral": "NEUTRAL",
    "Sadness": "SAD", "Sad": "SAD",
    "Surprise": "SURPRISE", "Surprised": "SURPRISE",
}

_zeroconf = None


def _get_lan_ip():
    """获取本机局域网 IP"""
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]
    except Exception:
        return "127.0.0.1"
    finally:
        s.close()


def start_mdns():
    """广播 liveface.local，让 ESP8266 通过主机名找到本机"""
    global _zeroconf
    try:
        lan_ip = _get_lan_ip()
        info = ServiceInfo(
            "_http._tcp.local.",
            "liveface._http._tcp.local.",
            addresses=[socket.inet_aton(lan_ip)],
            port=443,
            properties={},
            server="liveface.local.",
        )
        _zeroconf = Zeroconf(ip_version=IPVersion.V4Only)
        _zeroconf.register_service(info)
        print(f"mDNS advertised: liveface.local -> {lan_ip}:443")
    except Exception as e:
        print(f"mDNS 广播失败: {e}")


def resolve_esp_ip():
    """通过 mDNS 解析 esp8266.local -> IP"""
    global ESP_IP
    try:
        zc = Zeroconf(ip_version=IPVersion.V4Only)
        info = zc.get_service_info("_http._tcp.local.", "esp8266._http._tcp.local.")
        zc.close()
        if info and info.addresses:
            ESP_IP = socket.inet_ntoa(info.addresses[0])
            return True
    except Exception:
        pass
    return False


def _probe_esp(ip):
    """探测指定 IP 是否为 ESP8266（/esp_ip 必须返回一个 IP 格式的文本）"""
    try:
        r = requests.get(f"http://{ip}/esp_ip", timeout=0.4)
        if r.status_code == 200:
            txt = r.text.strip()
            if re.match(r"^\d{1,3}(\.\d{1,3}){3}$", txt):
                return ip
    except Exception:
        pass
    return None


def discover_esp():
    global ESP_IP
    ip_file = os.path.join(os.path.dirname(__file__), "esp_ip.txt")
    if os.path.exists(ip_file):
        with open(ip_file) as f:
            ESP_IP = f.read().strip()
            print(f"ESP IP from file: {ESP_IP}")
            return
    # 优先 mDNS（重试几次，等 ESP 的 mDNS 服务广播）
    for _ in range(5):
        if resolve_esp_ip():
            print(f"ESP discovered via mDNS: {ESP_IP}")
            return
        import time
        time.sleep(1)
    # 回退：全段扫描，且校验响应为 IP 格式（排除路由器等设备）
    base = "192.168.0."
    with ThreadPoolExecutor(max_workers=80) as ex:
        found = [ip for ip in ex.map(_probe_esp, [base + str(i) for i in range(1, 255)]) if ip]
    if found:
        ESP_IP = found[0]
        print(f"ESP discovered via scan: {ESP_IP}")
    else:
        print("ESP 未找到（可手动写入 webapp/esp_ip.txt）")


def send_emotion_to_esp(emotion_name):
    global ESP_IP
    if not ESP_IP and not resolve_esp_ip():
        return
    esp_name = EMOTION_TO_ESP.get(emotion_name, "HAPPY")
    try:
        requests.post(
            f"http://{ESP_IP}/emotion",
            json={"emotion": esp_name},
            timeout=0.5,
        )
    except Exception:
        pass

discover_esp()
start_mdns()

for fp in ["C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/simhei.ttf", "C:/Windows/Fonts/simsun.ttc"]:
    try:
        font_cn = ImageFont.truetype(fp, FONT_SIZE, encoding="unic")
        break
    except Exception:
        continue

camera = None
frame_skip = 2
frame_count = 0
last_faces = []


def release_camera():
    global camera
    if camera:
        camera.release()
        camera = None


atexit.register(release_camera)


def get_camera():
    global camera
    if camera is None:
        camera = cv2.VideoCapture(0)
        camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
        if not camera.isOpened():
            camera = None
            raise RuntimeError("Cannot open camera")
    return camera


def generate_frames():
    global frame_count, last_faces
    try:
        cap = get_camera()
    except RuntimeError:
        return

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        frame_count += 1
        do_detect = (frame_count % frame_skip == 0)

        if do_detect:
            rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            try:
                mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb)
                detection_result = face_detector.detect(mp_image)
                new_faces = []
                if detection_result.detections:
                    for det in detection_result.detections:
                        bbox = det.bounding_box
                        x, y = max(0, bbox.origin_x), max(0, bbox.origin_y)
                        bw = min(bbox.width, frame.shape[1] - x)
                        bh = min(bbox.height, frame.shape[0] - y)
                        if bw <= 0 or bh <= 0:
                            continue
                        face_crop = frame[y : y + bh, x : x + bw]
                        if face_crop.size == 0:
                            continue
                        try:
                            labels, probs = model.predict_emotions(face_crop, logits=False)
                            emotion = labels[0]
                            prob_arr = np.array(probs[0] if isinstance(probs, list) else probs).flatten()
                            conf_val = float(np.max(prob_arr))
                        except Exception:
                            emotion = "?"
                            conf_val = 0.0
                        new_faces.append({"box": (x, y, bw, bh), "emotion": emotion, "confidence_label": conf_val})
                if new_faces:
                    send_emotion_to_esp(new_faces[0]["emotion"])
                last_faces = new_faces
            except Exception:
                last_faces = []

        if last_faces:
            # 全部在 PIL 上绘制（OpenCV 不支持中文，且避免画框被覆盖）
            pil_img = Image.fromarray(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))
            draw = ImageDraw.Draw(pil_img)

            for face in last_faces:
                x, y, bw, bh = face["box"]
                emotion = face.get("emotion", "?")
                conf = face.get("confidence_label", 0.0)
                emotion_cn = EMOTION_CN.get(emotion, emotion)
                label = emotion_cn

                draw.rectangle([x, y, x + bw, y + bh], outline=(0, 255, 0), width=2)
                if font_cn:
                    bbox = draw.textbbox((0, 0), label, font=font_cn)
                    tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
                    draw.rectangle([x, y - th - 6, x + tw + 4, y], fill=(0, 255, 0))
                    draw.text((x + 2, y - th - 4), label, font=font_cn, fill=(0, 0, 0))

            frame = cv2.cvtColor(np.array(pil_img), cv2.COLOR_RGB2BGR)

        ret, jpeg = cv2.imencode(".jpg", frame, [cv2.IMWRITE_JPEG_QUALITY, 75])
        if not ret:
            continue
        yield b"--frame\r\nContent-Type: image/jpeg\r\n\r\n" + jpeg.tobytes() + b"\r\n"


def analyze_frame(frame):
    """检测人脸 + 识别表情，返回 faces 列表，并同步到 ESP8266"""
    faces_out = []
    try:
        # 缩小帧加速 MediaPipe 检测（骨骼模型对尺寸不敏感）
        h, w = frame.shape[:2]
        if w > 640:
            scale = 640.0 / w
            frame_small = cv2.resize(frame, (int(w * scale), int(h * scale)),
                                     interpolation=cv2.INTER_AREA)
        else:
            frame_small = frame
        rgb = cv2.cvtColor(frame_small, cv2.COLOR_BGR2RGB)
        mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb)
        detection_result = face_detector.detect(mp_image)
        h_small, w_small = frame_small.shape[:2]
        sx, sy = w / w_small, h / h_small
        for det in detection_result.detections:
            bbox = det.bounding_box
            x, y = max(0, int(bbox.origin_x * sx)), max(0, int(bbox.origin_y * sy))
            bw = int(bbox.width * sx)
            bh = int(bbox.height * sy)
            bw = min(bw, frame.shape[1] - x)
            bh = min(bh, frame.shape[0] - y)
            if bw <= 0 or bh <= 0:
                continue
            face_crop = frame[y : y + bh, x : x + bw]
            if face_crop.size == 0:
                continue
            try:
                labels, probs = model.predict_emotions(face_crop, logits=False)
                emotion = labels[0]
            except Exception:
                emotion = "?"
            faces_out.append({
                "box": [x, y, bw, bh],
                "emotion": EMOTION_TO_ESP.get(emotion, "HAPPY"),
                "emotion_cn": EMOTION_CN.get(emotion, emotion),
            })
            if emotion != "?":
                send_emotion_to_esp(emotion)
    except Exception:
        pass
    return faces_out


@sock.route("/ws")
def ws_recognize(ws):
    """WebSocket 实时识别：浏览器发二进制 JPEG 帧，返回 JSON 表情结果"""
    import json as _json
    while True:
        data = ws.receive()  # 二进制帧
        if data is None:
            break
        try:
            nparr = np.frombuffer(data, np.uint8)
            frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
        except Exception:
            continue
        if frame is None:
            continue
        faces = analyze_frame(frame)
        ws.send(_json.dumps({"faces": faces}))


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/analyze", methods=["POST"])
def api_analyze():
    """预留：接收浏览器摄像头帧（bas64），检测人脸 + 识别表情（WebSocket 为推荐方式）"""
    try:
        data = request.get_json(force=True)
        img_b64 = data.get("image", "")
        if not img_b64:
            return jsonify({"faces": []})
        img_bytes = base64.b64decode(img_b64)
        nparr = np.frombuffer(img_bytes, np.uint8)
        frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    except Exception:
        return jsonify({"faces": []})

    if frame is None:
        return jsonify({"faces": []})

    return jsonify({"faces": analyze_frame(frame)})


@app.route("/video_feed")
def video_feed():
    return Response(
        generate_frames(),
        mimetype="multipart/x-mixed-replace; boundary=frame",
    )


@app.route("/stop")
def stop():
    release_camera()
    return "ok"


if __name__ == "__main__":
    # HTTPS 自签名证书，让局域网设备能使用摄像头（浏览器显示警告后点继续）
    cert_dir = os.path.dirname(os.path.abspath(__file__))
    app.run(
        host="0.0.0.0", port=443,
        debug=False, threaded=True,
        ssl_context=(os.path.join(cert_dir, "cert.pem"), os.path.join(cert_dir, "key.pem")),
    )