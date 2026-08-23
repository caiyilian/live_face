import os, sys, cv2, torch, numpy as np
import atexit
import mediapipe as mp
from flask import Flask, render_template, Response

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "openface3"))
os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"

from emotiefflib.facial_analysis import EmotiEffLibRecognizerTorch

app = Flask(__name__)

print("Loading EmotiEffLib model (CUDA)...")
model = EmotiEffLibRecognizerTorch(model_name="enet_b0_8_best_vgaf", device="cuda")
model.model.eval()
print("Model loaded.")

# MediaPipe Face Detection (CPU, 比 MTCNN 快非常多)
mp_face_detection = mp.solutions.face_detection
face_detector = mp_face_detection.FaceDetection(
    model_selection=0, min_detection_confidence=0.5
)

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
                results = face_detector.process(rgb)
                new_faces = []
                if results.detections:
                    h_img, w_img = frame.shape[:2]
                    for det in results.detections:
                        bbox = det.location_data.relative_bounding_box
                        x = int(bbox.xmin * w_img)
                        y = int(bbox.ymin * h_img)
                        bw = int(bbox.width * w_img)
                        bh = int(bbox.height * h_img)
                        x, y = max(0, x), max(0, y)
                        bw = min(bw, w_img - x)
                        bh = min(bh, h_img - y)
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
                last_faces = new_faces
            except Exception:
                last_faces = []

        for face in last_faces:
            x, y, bw, bh = face["box"]
            emotion = face.get("emotion", "?")
            conf = face.get("confidence_label", 0.0)

            cv2.rectangle(frame, (x, y), (x + bw, y + bh), (0, 255, 0), 2)
            label = f"{emotion} ({conf:.0%})"
            (tw, th), _ = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.65, 2)
            cv2.rectangle(frame, (x, y - th - 8), (x + tw, y), (0, 255, 0), -1)
            cv2.putText(frame, label, (x, y - 4), cv2.FONT_HERSHEY_SIMPLEX, 0.65, (0, 0, 0), 2)

        ret, jpeg = cv2.imencode(".jpg", frame, [cv2.IMWRITE_JPEG_QUALITY, 75])
        if not ret:
            continue
        yield b"--frame\r\nContent-Type: image/jpeg\r\n\r\n" + jpeg.tobytes() + b"\r\n"


@app.route("/")
def index():
    return render_template("index.html")


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
    app.run(host="127.0.0.1", port=5000, debug=False, threaded=True)