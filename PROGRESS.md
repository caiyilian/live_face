# 项目进度记录 (测试进行中)

> 更新时间: 2026-08-23 (下午暂停，准备外出)
> 环境: Windows | uv | Python 3.11 + torch 2.5.1+cu124

## 已完成

### 1. 调研
- 调研报告已写入 `research_face_expression_recognition.md`
- 已推送到 GitHub: https://github.com/caiyilian/live_face

### 2. 克隆项目 (全部到 `E:\case\live_face\`)
| 目录 | 项目 | 说明 |
|------|------|------|
| `deepface/` | serengil/deepface | tensorflow 系 |
| `emotiefflib/` | sb-ai-lab/EmotiEffLib | torch ✅ |
| `fer/` | justinshenk/fer | tensorflow 系 |
| `face_classification/` | oarriaga/face_classification | tf1 锁死 |
| `EmoPy/` | thoughtworksarts/EmoPy | tf1 锁死 |
| `EfficientFace/` | zengqunzhao/EfficientFace | torch ✅ |
| `Deep-Emotion/` | omarsayed7/Deep-Emotion | torch ✅ |
| `openface3/` | CMU-MultiComp-Lab/OpenFace-3.0 | torch ✅ (核心) |
| `WU_FER/` | WuJie1010/Facial-Expression-Recognition.Pytorch | torch ✅ |
| `FacialExpressionRecognition/` | luanshiyinyang | tensorflow 系 |
| `xiongfer/` | xionghc | tensorflow1 老项目 |
| `faceapi-js/` | face-api.js | JS (不需要 uv) |
| `humanjs/` | vladmandic/human | JS (不需要 uv) |

> 注意: `openface3/` 是**用 curl 下载 zip 解压的**（git clone 一直 TLS 失败），没有 .git 目录。

### 3. 环境搭建
- 目录: `E:\case\live_face\.venv` (Python 3.11.14)
- torch: `torch-2.5.1+cu124-cp311-cp311-win_amd64.whl` (本地 whl，位于 `whl/` 目录)
- GPU: RTX 2060, CUDA 可用 ✅
- 已装包: torchvision 0.20.1, torchaudio 2.5.1, numpy 2.4, opencv-python 5.0, pillow, tqdm, timm 0.9.16, onnx, onnxruntime, matplotlib, scikit-learn, scipy, tensorflow-cpu 2.21, keras 3.15

### 4. 测试结果
| 项目 | 状态 | 详情 |
|------|------|------|
| `Deep-Emotion/` | ✅ 跑通 | 模型前向 (1,7) 输出，SpatialTransformer 有 align_corners warning 无害 |
| `emotiefflib/` | ✅ 跑通 | 真实图片推理出 Neutral (0.424)，torch 后端 CUDA |
| `EfficientFace/` | ✅ 跑通 | 需 **224x224** 输入(不是112!)，前向 (1,7)。无预训练权重，未做真实推理 |
| `openface3/` | ✅ 核心通 | MLT 多任务模型 (emotion 8类 + gaze + AU)，权重加载零缺失。dlib 未装，完整 demo 流程(start 需 dlib+STAR权重)未测 |
| `WU_FER/` | ✅ 跑通 | VGG19 前向 20M 参数, (1,7) 输出 |
| `deepface/` | ⚠️ 受限 | 已装好，但 `from deepface import DeepFace` 因 retinaface 包在 keras3 下报错，需 `pip install tf-keras` 修复 |
| `fer/` | ⏸ 跳过 | tensorflow 系，用户要求不纠结 tf |
| `face_classification/` | ⏸ 跳过 | 锁 tf1.x (keras==2.0.5, tensorflow==1.1.0)，py3.11 无法安装 |
| `EmoPy/` | ⏸ 跳过 | 锁 tf1.x (tensorflow==1.13.1) |
| `FacialExpressionRecognition/` | ⏸ 跳过 | tensorflow 系 |
| `xiongfer/` | ⏸ 跳过 | tensorflow1 老项目 |
| `faceapi-js/` `humanjs/` | ⏸ 跳过 | JS 项目，非 python/uv |

### 5. 遇到的问题
- **网络**: git 走代理 (7890) 时 TLS 经常失败，需要重试或改用 curl 下载 zip
- **uv 网络抖动**: `uv pip install` 走代理需设 `$env:UV_HTTP_TIMEOUT=300`，失败就重试
- **git 全局代理已配置**: `git config --global http.proxy/http.sslVerify`

## 待办 (回家继续)
1. (可选) `pip install tf-keras` 修复 deepface import，测试 `DeepFace.analyze` 表情
2. (可选) 解决 openface3 的 dlib (Windows 无预编译 wheel，需源码编译) + STAR alignment 权重下载，跑完整 demo
3. **汇总测试结果** 写入新 md (如 `test_report.md`)，提交推送到 GitHub
4. genshin 不需要了...（不缺发）记得有 openface3 的 STAR 权重下载点没落地

## 遗留文件
- 测试脚本在 `C:\Users\30215\AppData\Local\Temp\opencode\test_openface.py` 和 `test_wujie.py`
- `whl/` 目录下有 torch 的本地 whl (2.4GB，已用完，可删可留)