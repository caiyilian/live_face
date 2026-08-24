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
- **电脑**浏览器之前无法解析 `liveface.local`
- 原因：Windows 对 `.local` mDNS 主机名支持不稳定

### ✅ 已解决 (2026-08-24)
- 确认 Apple Bonjour 已安装且 `Bonjour Service` 运行中
- 现在电脑可正常解析：
  - `Resolve-DnsName liveface.local` → `192.168.0.103` ✅
  - `https://liveface.local` → HTTP 200 ✅
- 电脑浏览器可直接访问 `https://liveface.local`
- 注意：若之前打不开，可能是 DNS 缓存未刷新，或后端当时未在广播

### 历史方案（已不需要）
| 方案 | 说明 |
|------|------|
| 安装 **Apple Bonjour**（Windows） | ✅ 已完成（系统已有） |
| hosts 文件手动加条目 | 不采用 |
| **不处理** | 已过时 |