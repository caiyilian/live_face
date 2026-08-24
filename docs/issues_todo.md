# 待解决问题记录

> 更新时间: 2026-08-24

## 问题 1: 识别链路效率低（base64 POST 轮询）

### ✅ 已解决 (2026-08-24)
- 改为 **WebSocket 二进制帧**：
  - 后端 `flask-sock` 新增 `/ws` 端点，接收二进制 JPEG，逐帧识别，返回 JSON
  - 前端 `sendFrame()` 用 `canvas.toBlob()` 直接发 JPEG 二进制，省掉 base64（膨胀 33%）和 HTTP 握手
  - `/api/analyze` 保留作为备用（不再被前端默认使用）
- 依赖新增：`flask-sock`
- 前端发送间隔 200ms

### 业界做法
| 方案 | 适用场景 | 特点 |
|------|----------|------|
| **WebSocket (二进制帧)** | 浏览器 → 后端推理 | ✅ 当前采用，免 base64 和握手 |
| **WebRTC** | 直播级实时视频 | 真正视频流级，P2P 传输，实现最复杂 |
| **MJPEG over HTTP** | 服务端设备摄像头 | server push，不适用于手机摄像头 |
| **浏览器端跑模型** (MediaPipe/TFLite) | 客户端推理 | 无需后端，但精度/资源受限 |

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

### ⚠️ 踩坑记录：Edge 浏览器无法解析 mDNS（2026-08-24）
现象：手机和其他浏览器都能访问 `https://liveface.local`，唯独**常用 Edge** 打不开。
尝试了以下方法**都不行**：
1. Edge InPrivate 无痕模式
2. `edge://net-internals/#dns` 清 DNS 缓存
3. 任务管理器结束所有 Edge 后台进程（含启动加速）
4. `edge://settings/clearBrowserData` 清缓存

**最终解决办法**：**重启电脑** ✅

原因：Windows 底层的 mDNS 解析栈 + Bonjour 状态在浏览器层面清理不到，只有重启整个网络解析栈才生效。

**后续再遇（2026-08-24 被只重启后端再次触发）**：
- 重启后端后 Edge 又打不开 `liveface.local`，而 Chrome 依旧正常
- **Edge 里直接用 IP**：`https://192.168.0.103`（证书 SAN 已覆盖 IP，同样免警告）— 永远可用
- 结论：`liveface.local` 适合 Chrome/手机；Edge 有 mDNS 毛病，直接用 IP 最省事

经验：
- 遇到 Edge + mDNS(.local) 打不开，先别折腾浏览器缓存，直接重启电脑最快
- 或者换一个没用过的浏览器/无痕窗口确认是不是 mDNS 本身的问题

### 历史方案（已不需要）
| 方案 | 说明 |
|------|------|
| 安装 **Apple Bonjour**（Windows） | ✅ 已完成（系统已有） |
| hosts 文件手动加条目 | 不采用 |
| **不处理** | 已过时 |