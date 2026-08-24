# 待解决问题记录

> 更新时间: 2026-08-24

## 问题 1: 识别链路效率低（base64 POST 轮询）

### 现状
- 前端每 400ms 截一帧手机摄像头画面 → canvas → base64 → POST `/api/analyze` → 后端识别 → 返回 JSON
- 效率问题：
  - base64 编码把图片体积膨胀约 **33%**
  - 每帧一次 HTTP 请求/响应，含 TCP 握手开销
  - JSON 解析 + 响应体往返，增加延迟
  - 实际帧率有限，实时性差

### 业界做法
| 方案 | 适用场景 | 特点 |
|------|----------|------|
| **WebSocket (二进制帧)** | 浏览器 → 后端推理 | 省掉 base64 膨胀和 HTTP 握手，单向持续推送，延迟低 |
| **WebRTC** | 直播级实时视频 | 真正视频流级，P2P 传输，实现最复杂 |
| **MJPEG over HTTP** | 服务端设备摄像头（如我们之前的 PC 摄像头方案） | server push，不适用于手机摄像头 |
| **浏览器端跑模型** (MediaPipe/TFLite) | 客户端推理 | 无需后端，但精度/资源受限 |

> 参考：网页直播间和实时 AI 滤镜（如 Google 的 web 摄像头 demo）普遍用 WebSocket 或 WebRTC，不是 base64 POST。

### 建议方案
改成 **WebSocket**：
- 前端连接 `ws://`（HTTPS 页面配对用 `wss://`）
- 每帧以二进制 `Blob`/`ArrayBuffer` 发送 JPEG
- 后端 `flask-sock` 或 `websockets` 处理，返回 JSON 表情结果
- 需求：`pip install flask-sock`（或 websockets）

**状态**: 待实现（用户确认后再动手）

---

## 问题 2: Windows 无法解析 liveface.local（mDNS）

### 现状
- 后端广播 `liveface.local` 正常
- **手机**能解析 `liveface.local` → 正常访问
- **电脑**浏览器无法解析 `liveface.local`
- 原因：Windows 对 `.local` mDNS 主机名支持不稳定（原生 mDNS 解析时好时坏）

### 影响评估
- **实际无影响**，因为：
  - ESP8266 重定向用的是 **IP**（`https://192.168.0.103`），不是主机名
  - PC 浏览器直接用 `https://192.168.0.103` 即可访问，不需要 `.local`
  - `liveface.local` 只被 ESP 的 `MDNS.queryService()` 内部使用（那是 ESP 自己解析，与 Windows 无关）
- `esp8266.local` 电脑也访问不了，同理——直接用 ESP 的 IP 访问

### 彻底解决办法（可选）
| 方案 | 说明 |
|------|------|
| 安装 **Apple Bonjour**（Windows） | 系统级稳定支持 `.local` 解析，最可靠 |
| hosts 文件手动加条目 | `192.168.0.103 liveface.local`，但 IP 变了就要改，违背初衷 |
| **不处理**（推荐） | 反正用 IP 就行，mDNS 只是 ESP 内部找 PC 用的 |

**状态**: 建议不处理，直接用 IP