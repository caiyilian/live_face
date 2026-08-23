# 人脸表情识别 (Facial Expression Recognition) 调研报告

> 调研时间: 2026-08-23
> 目标: 不做重复造轮子，尽量复用现有开源项目和模型

---

## 1. 整体 pipeline 框架

绝大多数开源 FER 系统遵循以下流程:

```
输入图像/视频 → 人脸检测 → 人脸对齐 → 特征提取 → 表情分类
```

- **人脸检测**: OpenCV Haar Cascade, MTCNN, RetinaFace, YOLO, MediaPipe, SSD
- **人脸对齐**: 基于面部关键点 (landmarks) 进行仿射变换
- **特征提取**: CNN / ViT / MobileNet / EfficientNet 等 backbone
- **表情分类**: 全连接层 + Softmax 输出 7/8 类表情概率

---

## 2. 开源项目对比

### 2.1 ⭐ DeepFace (Python, 23k+ stars)

| 项目 | 值 |
|------|-----|
| 仓库 | https://github.com/serengil/deepface |
| Stars | 23,324 |
| 语言 | Python |
| 许可 | MIT |

**简介**: 轻量级人脸识别 + 面部属性分析 (年龄、性别、表情、种族) 框架。**同时支持人脸检测、识别、表情分析**，是最全面的综合库。

**表情分析能力**:
- 识别 7 种表情: `angry, fear, neutral, sad, disgust, happy, surprise`
- 底层使用 VGG-Face backbone 做迁移学习
- 年龄模型 MAE 4.65; 性别准确率 97.44%; 表情模型准确率 ~66%

**人脸检测后端 (支持 18 种)**: opencv, ssd, dlib, mtcnn, fastmtcnn, retinaface, mediapipe, yolov8n/m/l, yolov11n/s/m/l, yolov12n/s/m/l, yunet, centerface

**人脸识别模型 (支持 11 种)**: VGG-Face, FaceNet, FaceNet512, OpenFace, DeepFace, DeepID, ArcFace, Dlib, SFace, GhostFaceNet, Buffalo_L

**使用方式**:
```python
from deepface import DeepFace
objs = DeepFace.analyze(img_path="img.jpg", actions=['emotion', 'age', 'gender', 'race'])
```

**API 服务**: 内置 Flask/Gunicorn, 支持 Docker 部署, 也提供云托管版本 deepface.dev

**结论**: 如果只需要快速集成表情识别功能，DeepFace 是首选，一行代码即可完成。

---

### 2.2 ⭐ face-api.js (JavaScript, 17.9k+ stars)

| 项目 | 值 |
|------|-----|
| 仓库 | https://github.com/justadudewhohacks/face-api.js |
| Stars | 17,943 |
| 语言 | JavaScript |
| 许可 | MIT |

**简介**: 基于 tensorflow.js 的浏览器/Node.js 人脸识别 API。

**能力**:
- 人脸检测 (SSD MobileNet V1 / Tiny Face Detector)
- 68 点面部关键点检测
- 人脸识别 (128维特征向量, LFW 99.38%)
- **表情识别** (7 类: angry, disgust, fear, happy, neutral, sad, surprise)
- 年龄估计 + 性别识别

**使用方式**:
```javascript
const detections = await faceapi
  .detectAllFaces(input)
  .withFaceLandmarks()
  .withFaceExpressions()
```

**结论**: 适合浏览器端/前端应用，纯 JS 方案，无需后端。

---

### 2.3 ⭐ WuJie1010/Facial-Expression-Recognition.Pytorch (2k stars)

| 项目 | 值 |
|------|-----|
| 仓库 | https://github.com/WuJie1010/Facial-Expression-Recognition.Pytorch |
| Stars | 1,977 |
| 语言 | Python |
| 框架 | PyTorch |

**简介**: 基于 CNN 的 PyTorch 表情识别实现，在 FER2013 上达到 SOTA。

**性能**:
- FER2013: PublicTest 71.50%, PrivateTest **73.11%**
- CK+: 10-fold **94.65%**
- 模型: VGG19, ResNet18

**数据集**: FER2013 (48x48 灰度, 7 类, 28,709 训练), CK+ (327 视频序列)

**结论**: 适合想自己训练/微调表情识别模型的场景，代码结构清晰。

---

### 2.4 ⭐ EmotiEffLib (HSEmotion, 1k+ stars)

| 项目 | 值 |
|------|-----|
| 仓库 | https://github.com/sb-ai-lab/EmotiEffLib |
| Stars | 1,054 |
| 语言 | Python/C++ |
| 许可 | Apache-2.0 |
| pip | `pip install emotiefflib` |

**简介**: 高效表情识别库，在 AffectNet 上达到 SOTA，支持 PyTorch 和 ONNX 推理。在 ABAW 竞赛中多次获得第一名。

**性能**:
| 模型 | AffectNet-7 | AffectNet-8 | AFEW | VGAF |
|------|:-----------:|:-----------:|:----:|:----:|
| enet_b0_8 | 64.63% | 60.95% | 59.89% | 66.80% |
| enet_b2_8 | 66.29% | 63.03% | 57.78% | 70.23% |
| enet_b2_7 | 66.34% | - | 59.63% | 69.84% |
| mobilenet_7 | 64.71% | - | 55.35% | 68.92% |

**特点**:
- 基于 EfficientNet backbone，VGGFace2 预训练
- 在 AffectNet 上微调，支持 7 类或 8 类表情
- 支持 Android 移动端部署
- 提供 C++ 版本
- 有论文发表 (ICML 2023, IEEE TAC 2022)

**结论**: 学术性能最强，适合对精度要求高的场景。

---

### 2.5 ⭐ Human (vladmandic, 3.2k+ stars)

| 项目 | 值 |
|------|-----|
| 仓库 | https://github.com/vladmandic/human |
| Stars | 3,258 |
| 语言 | TypeScript/JavaScript |
| 许可 | MIT |

**简介**: 全功能 AI 人体分析库，包括 3D 人脸检测、面部描述、表情识别、姿态估计、手势识别等。

**表情模型**: 使用 Oarriaga Emotion 模型 (同 face-api.js 的底层模型)

**结论**: 前端全能方案，比 face-api.js 功能更全面，包含身体/手部追踪。

---

### 2.6 ⭐ thoughtworksarts/EmoPy (965 stars)

| 项目 | 值 |
|------|-----|
| 仓库 | https://github.com/thoughtworksarts/EmoPy |
| Stars | 965 |
| 语言 | Python |

**简介**: 基于深度神经网络的表情分析工具包。

**结论**: 比较传统的实现，已不再活跃维护。

---

### 2.7 ⭐ face_classification (oarriaga, 5.7k+ stars)

| 项目 | 值 |
|------|-----|
| 仓库 | https://github.com/oarriaga/face_classification |
| Stars | 5,736 |
| 语言 | Python/Keras |
| 状态 | **已弃用** → 迁移至 https://github.com/oarriaga/paz |

**简介**: 实时人脸检测 + 表情/性别分类，使用 Keras CNN + OpenCV。

**性能**: FER2013 表情 66%，IMDB 性别 96%

**结论**: 历史项目，但很多其他库 (如 FER, face-api.js) 都引用了它的模型权重。

---

### 2.8 ⭐ FER (justinshenk, 427 stars)

| 项目 | 值 |
|------|-----|
| 仓库 | https://github.com/justinshenk/fer |
| Stars | 427 |
| pip | `pip install fer` |
| 许可 | MIT |

**简介**: 封装为 PyPI 包的表情识别库，支持图片、视频、摄像头。

**特点**:
- 底层使用 Keras 模型 (源自 oarriaga/face_classification)
- 支持 MTCNN 人脸检测
- 支持视频分析 + 输出 Pandas DataFrame
- 支持 TF Serving 部署

**使用方式**:
```python
from fer import FER
detector = FER(mtcnn=True)
emotion, score = detector.top_emotion(img)
```

**结论**: 轻量级、易用性高，适合快速集成。

---

### 2.9 ⭐ OpenFace 3.0 (CMU, 190 stars)

| 项目 | 值 |
|------|-----|
| 仓库 | https://github.com/CMU-MultiComp-Lab/OpenFace-3.0 |
| Stars | 190 |
| 语言 | Python (PyTorch) |
| 许可 | 开源 |

**简介**: CMU 开发的综合面部行为分析工具包，支持面部关键点、AU (动作单元)、表情识别、视线估计。

**技术栈**:
- 人脸检测: RetinaFace
- 关键点: STAR 模型 (98/68 点)
- 多任务模型: 同时预测 **表情 (8 类, AffectNet) + 视线 (yaw/pitch) + AU 强度**

**表情分类 (8 类)**: Neutral, Happy, Sad, Surprise, Fear, Disgust, Anger, Contempt

**结论**: 学术研究级别，功能最全面 (AU + 表情 + 视线)，适合需要 AU 分析的研究场景。

---

### 2.10 ⭐ EfficientFace (AAAI 2021, 233 stars)

| 项目 | 值 |
|------|-----|
| 仓库 | https://github.com/zengqunzhao/EfficientFace |
| Stars | 233 |
| 框架 | PyTorch |
| 论文 | AAAI 2021 |

**简介**: 轻量级表情识别网络，使用标签分布训练 (Label Distribution Training)。

**性能**:
| 数据集 | 准确率 |
|--------|:------:|
| RAF-DB | 88.36% |
| CAER-S | 85.87% |
| AffectNet-7 | 63.70% |
| AffectNet-8 | 59.89% |

**结论**: 轻量级模型，适合移动端/边缘部署。

---

### 2.11 ⭐ Deep-Emotion (286 stars)

| 项目 | 值 |
|------|-----|
| 仓库 | https://github.com/omarsayed7/Deep-Emotion |
| Stars | 286 |
| 框架 | PyTorch |

**简介**: 使用注意力卷积网络 (Attentional Convolutional Network) 的表情识别。

**结论**: 注意力机制引入，适合研究参考。

---

### 2.12 ⭐ FacialExpressionRecognition (luanshiyinyang, 987 stars)

| 项目 | 值 |
|------|-----|
| 仓库 | https://github.com/luanshiyinyang/FacialExpressionRecognition |
| Stars | 987 |
| 框架 | TensorFlow/Keras |

**简介**: 中文社区热门项目，提供 GUI 界面 + 实时摄像头识别。

**特点**:
- 传统方法: LBP, Gabor + SVM
- 深度方法: CNN + 全连接
- 数据集: FER2013, JAFFE, CK+
- 提供 GUI (PyQT) 和实时摄像头识别

**结论**: 适合入门学习，中文文档完善，提供 GUI。

---

## 3. 表情数据集

| 数据集 | 样本数 | 类别 | 说明 |
|--------|:------:|:----:|------|
| **FER2013** | 35,887 | 7类 | Kaggle 竞赛数据集，48x48 灰度图，标签有噪声 |
| **AffectNet** | 450,000+ | 8类 | 最大规模表情数据集，手动标注，学术界标准基准 |
| **CK+** | 981 (327视频) | 7类 | 实验室采集，表情顶峰帧，准确率高 |
| **RAF-DB** | 30,000 | 7类 | 真实场景，基本表情 + 复合表情 |
| **JAFFE** | 213 | 6类 | 日本女性人脸表情，实验室环境 |
| **SFEW** (AFEW 静态) | 700+ | 7类 | 影视剧照，真实场景 |
| **AFEW** | 1,800+ 视频 | 7类 | 影视片段，时序数据 |
| **ExpW** | 91,793 | 7类 | 网络图片自然表情 |
| **DFEW** | 16,000+ 视频 | 7类 | 大规模动态表情数据库 |

**7类表情映射**: `0=Angry, 1=Disgust, 2=Fear, 3=Happy, 4=Sad, 5=Surprise, 6=Neutral`

**8类表情映射** (AffectNet): 上述 7 类 + `7=Contempt`

---

## 4. HuggingFace 上的预训练模型

| 模型 | 下载量 | 框架 | 说明 |
|------|:------:|:----:|------|
| `abhilash88/face-emotion-detection` | 372 | TF | 基础表情检测 |
| `dima806/face_emotions_image_detection` | 91 | PyTorch | 表情分类 |
| `pat229988/AffectNet-face-to-emotion-tflite` | 411 | TFLite | AffectNet 训练，适合移动端 |
| `leeyunjai/yolo11-face-emotion-fer2013-cls` | 36 | YOLOv11 | 基于 YOLO 的分类 |
| `mo-thecreator/vit-Facial-Expression-Recognition` | 1,840 | ViT | Vision Transformer 表情识别 |
| `ElenaRyumina/face_emotion_recognition` | - | PyTorch | 视频表情识别 |

---

## 5. 技术方案选型建议

### 场景 A: 快速集成，不想训练模型

**推荐: DeepFace (Python) 或 face-api.js (前端)**

```python
# DeepFace 一行代码
from deepface import DeepFace
result = DeepFace.analyze("face.jpg", actions=['emotion'])
```

### 场景 B: 学术/工业级精度

**推荐: EmotiEffLib 或 OpenFace 3.0**

```python
# EmotiEffLib
from emotiefflib.facial_expression import FacialExpressionRecognizer
recognizer = FacialExpressionRecognizer()
emotions, scores = recognizer.predict_emotions(face_img)
```

### 场景 C: 前端/浏览器端

**推荐: face-api.js 或 Human**

```javascript
// face-api.js
const detections = await faceapi.detectAllFaces(video).withFaceExpressions()
```

### 场景 D: 自训练/微调

**推荐: WuJie1010 (PyTorch) 或 EfficientFace + AffectNet 数据集**

### 场景 E: 移动端/边缘设备

**推荐: EmotiEffLib (TFLite/ONNX) 或 EfficientFace**

---

## 6. 各大项目关键特征对比表

| 项目 | 语言 | 框架 | 人脸检测 | 表情 | 其他属性 | 支持视频 | 实时 | 移动端 |
|------|:----:|:----:|:--------:|:----:|:--------:|:--------:|:----:|:------:|
| DeepFace | Python | TF/Keras | 18种 | ✅ | 年龄/性别/种族 | ✅ | ✅ | ❌ |
| face-api.js | JS | tfjs | 2种 | ✅ | 年龄/性别 | ✅ | ✅ | ✅ |
| EmotiEffLib | Python | PyTorch/ONNX | MTCNN | ✅ | 专注度 | ✅ | ✅ | ✅ |
| OpenFace 3.0 | Python | PyTorch | RetinaFace | ✅ | AU/视线/关键点 | ✅ | ✅ | ❌ |
| FER | Python | Keras | Haar/MTCNN | ✅ | ❌ | ✅ | ✅ | ❌ |
| Human | JS | tfjs | BlazeFace | ✅ | 身体/手势/视线 | ✅ | ✅ | ✅ |
| WuJie1010 | Python | PyTorch | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ |
| EfficientFace | Python | PyTorch | ❌ | ✅ | ❌ | ❌ | ❌ | ✅ |

---

## 7. 总结与建议

1. **DeepFace 是最适合快速集成的方案** — 23k+ stars, 活跃维护, 一行代码搞定人脸检测 + 表情识别 + 年龄/性别/种族分析
2. **EmotiEffLib 学术精度最高** — AffectNet SOTA, ABAW 竞赛冠军, 提供 ONNX 便于跨平台
3. **OpenFace 3.0 功能最全面** — 唯一同时提供 AU + 表情 + 视线估计的完整工具包
4. **face-api.js / Human 最适合前端** — 纯浏览器端运行, 无需后端
5. **如果自己做项目, 推荐方案**: DeepFace (快速原型) → 验证后换 EmotiEffLib 或 OpenFace 3.0 (提升精度)