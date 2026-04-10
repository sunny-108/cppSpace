# P5 — Real-Time Computer Vision + TensorRT Pipeline

> **Domain**: Computer Vision / Robotics / Autonomous Vehicles | **Effort**: 3–4 weeks | **GPU required**: NVIDIA (or CPU fallback with ONNX Runtime)

---

## Job Market Relevance

Every autonomous vehicle, warehouse robot, and drone runs a **real-time perception pipeline** in
C++. The ability to build one end-to-end is one of the most differentiating skills for robotics
and AV roles:

- **Waymo, Tesla, Cruise, Mobileye** — Sensor fusion, object detection, lane segmentation
- **Figure AI, Boston Dynamics** — Embodied AI: "where is my hand relative to the object?"
- **NVIDIA** — Deepstream SDK team, Jetson platform, DriveOS
- **DJI, Shield AI** — Drone perception, visual navigation

**Target Roles**: Autonomous Systems Engineer, Perception Engineer, Computer Vision Engineer  
**Salary Signal**: $160K–$320K; senior AV/robotics roles at Tesla/Waymo up to $400K+ TC

---

## What You'll Build

A real-time, multi-stage perception pipeline:

1. **Camera input** via V4L2 (Linux) / AVFoundation (macOS) — or from video file
2. **Preprocessing** — resize, normalize, BGR→RGB, CPU→GPU copy (CUDA pinned memory)
3. **YOLOv8 inference** via TensorRT 10 engine — with INT8 calibration support
4. **Post-processing** — NMS, bounding box decoding, confidence filtering on GPU
5. **Results rendering** — OpenCV overlay: boxes, labels, FPS counter
6. **Async pipeline** — 3-stage double-buffered pipeline achieving camera FPS (30+)

On macOS without NVIDIA GPU: falls back to ONNX Runtime (Core ML EP on Apple Silicon).

---

## Architecture

```
Camera Thread           Inference Thread            Display Thread
──────────────          ─────────────────────       ──────────────
 V4L2 / AVF             BoundedQueue<Frame>          OpenCV imshow
   capture   ──push──►  [Frame] [Frame] ...  ──push──►  render + FPS
                         │
                         │  GPU upload (pinned mem)
                         ▼
                    TensorRT Engine
                    (YOLOv8 / INT8)
                         │
                    NMS + decode
                         │
                    DetectionResult
                    { boxes, scores, classes }
```

### Buffer Strategy (Avoid Copies)

```
CPU pinned memory ──DMA──► GPU input buffer ──TRT──► GPU output buffer
       ▲                                                     │
       │                                                  memcpy
       └──────────── zero-copy if UVA (Unified Virtual) ───┘
```

---

## Key C++ Concepts Practiced

| Concept | Effective C++ Item | Where Used |
|---------|-------------------|------------|
| `unique_ptr` + custom deleter | Item 18 | `TrtEngine` owning `nvinfer1::IExecutionContext*` |
| `shared_ptr` for detection results | Item 19 | Results shared between inference + display threads |
| `make_unique` as factory | Item 21 | `TrtEngineBuilder::build()` returns `unique_ptr<TrtEngine>` |
| Pimpl hiding TensorRT headers | Item 22 | `pipeline.hpp` compiles without TRT installed |
| `weak_ptr` for frame observer | Item 20 | Metrics thread observes frames without extending lifetime |

---

## File Structure

```
p5-cv-pipeline/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── camera.hpp             # Abstract camera interface
│   ├── trt_engine.hpp         # RAII TensorRT engine (Pimpl)
│   ├── onnx_engine.hpp        # CPU fallback via ONNX Runtime
│   ├── preprocessor.hpp       # GPU preprocessing kernels
│   ├── postprocessor.hpp      # NMS + decode
│   ├── detection_result.hpp   # Result type
│   ├── bounded_queue.hpp      # Thread-safe bounded queue
│   └── pipeline.hpp           # Top-level pipeline
├── src/
│   ├── camera_v4l2.cpp        # Linux camera
│   ├── camera_file.cpp        # Video file source (OpenCV VideoCapture)
│   ├── trt_engine.cpp
│   ├── onnx_engine.cpp
│   ├── preprocessor.cu        # CUDA kernel
│   ├── postprocessor.cu       # CUDA NMS
│   └── pipeline.cpp
├── models/
│   └── yolov8n.onnx           # Download from Ultralytics
└── tools/
    └── build_trt_engine.cpp   # One-time ONNX→TRT engine build
```

---

## Starter Code

### `include/trt_engine.hpp` — RAII TensorRT (Item 22)

```cpp
#pragma once
#include <memory>
#include <string>
#include <vector>

// No TensorRT headers here — implementation hidden via Pimpl (Item 22)
// Callers compile this header without needing TensorRT SDK.

struct TrtInferResult {
    std::vector<float> outputData;   // raw network output (host)
    float inferMs;
};

class TrtEngine {
public:
    struct Config {
        std::string enginePath;      // serialized .trt engine file
        std::string inputName  = "images";
        std::string outputName = "output0";
        int batchSize = 1;
    };

    static std::unique_ptr<TrtEngine> fromFile(Config cfg);  // factory (Item 21)
    ~TrtEngine();

    // Input: preprocessed float data on host (NCHW, normalized)
    // Output: copies result to CPU for post-processing
    TrtInferResult infer(const float* hostInput, std::size_t inputBytes);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    explicit TrtEngine(std::unique_ptr<Impl>);
};
```

### `include/bounded_queue.hpp` — Lock-free producer/consumer

```cpp
#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(std::size_t cap) : cap_(cap) {}

    // Blocks if full; returns false if queue is shutting down
    bool push(T item) {
        std::unique_lock lk(mu_);
        notFull_.wait(lk, [this]{ return q_.size() < cap_ || closed_; });
        if (closed_) return false;
        q_.push(std::move(item));
        notEmpty_.notify_one();
        return true;
    }

    // Blocks if empty; returns nullopt on shutdown
    std::optional<T> pop() {
        std::unique_lock lk(mu_);
        notEmpty_.wait(lk, [this]{ return !q_.empty() || closed_; });
        if (q_.empty()) return std::nullopt;
        T item = std::move(q_.front()); q_.pop();
        notFull_.notify_one();
        return item;
    }

    void close() {
        std::scoped_lock lk(mu_);
        closed_ = true;
        notFull_.notify_all();
        notEmpty_.notify_all();
    }

private:
    std::queue<T> q_;
    std::mutex mu_;
    std::condition_variable notFull_, notEmpty_;
    std::size_t cap_;
    bool closed_ = false;
};
```

### GPU Preprocessing Kernel

```cuda
// src/preprocessor.cu
// Converts BGR uint8 (H,W,3) → RGB float32 (1,3,H,W) normalized
__global__ void bgr_to_nchw_normalized(
    const uint8_t* __restrict__ src,   // HWC BGR
    float*         __restrict__ dst,   // 1CHW RGB
    int H, int W,
    float meanR, float meanG, float meanB,
    float stdR,  float stdG,  float stdB)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= W || y >= H) return;

    int srcIdx = (y * W + x) * 3;
    float b = src[srcIdx + 0] / 255.f;
    float g = src[srcIdx + 1] / 255.f;
    float r = src[srcIdx + 2] / 255.f;   // OpenCV is BGR

    int planeSize = H * W;
    dst[0 * planeSize + y * W + x] = (r - meanR) / stdR;
    dst[1 * planeSize + y * W + x] = (g - meanG) / stdG;
    dst[2 * planeSize + y * W + x] = (b - meanB) / stdB;
}
```

### `include/detection_result.hpp`

```cpp
#pragma once
#include <vector>
#include <string>
#include <memory>

struct BoundingBox {
    float x1, y1, x2, y2;  // pixel coordinates
    float score;
    int   classId;
    std::string label;
};

// Results shared between inference and display threads (Item 19)
struct DetectionResult {
    std::vector<BoundingBox> boxes;
    float inferMs;
    float totalMs;  // including pre/post processing
    int   frameId;
};

using DetectionResultPtr = std::shared_ptr<const DetectionResult>;
```

---

## YOLOv8 NMS (Non-Maximum Suppression) CPU Fallback

```cpp
// Returns filtered boxes after NMS
std::vector<BoundingBox> nms(
    std::vector<BoundingBox> boxes, float iouThreshold = 0.45f)
{
    std::sort(boxes.begin(), boxes.end(),
              [](const auto& a, const auto& b){ return a.score > b.score; });

    std::vector<bool> suppressed(boxes.size(), false);
    std::vector<BoundingBox> result;

    for (std::size_t i = 0; i < boxes.size(); ++i) {
        if (suppressed[i]) continue;
        result.push_back(boxes[i]);
        for (std::size_t j = i+1; j < boxes.size(); ++j) {
            if (suppressed[j]) continue;
            // Compute IoU
            float interX1 = std::max(boxes[i].x1, boxes[j].x1);
            float interY1 = std::max(boxes[i].y1, boxes[j].y1);
            float interX2 = std::min(boxes[i].x2, boxes[j].x2);
            float interY2 = std::min(boxes[i].y2, boxes[j].y2);
            float inter = std::max(0.f, interX2-interX1) * std::max(0.f, interY2-interY1);
            float areaI = (boxes[i].x2-boxes[i].x1) * (boxes[i].y2-boxes[i].y1);
            float areaJ = (boxes[j].x2-boxes[j].x1) * (boxes[j].y2-boxes[j].y1);
            float iou = inter / (areaI + areaJ - inter + 1e-6f);
            if (iou > iouThreshold) suppressed[j] = true;
        }
    }
    return result;
}
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.24)
project(CvPipeline CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_ARCHITECTURES 86 89 90)

find_package(OpenCV REQUIRED)
find_package(CUDAToolkit REQUIRED)

# TensorRT paths (adjust to your installation)
set(TRT_ROOT "/usr/local/TensorRT" CACHE PATH "TensorRT root")
find_library(TRT_INFER  nvinfer  PATHS ${TRT_ROOT}/lib)
find_library(TRT_ONNX   nvonnxparser PATHS ${TRT_ROOT}/lib)

add_executable(cv_pipeline
    src/camera_file.cpp
    src/trt_engine.cpp
    src/preprocessor.cu
    src/postprocessor.cu
    src/pipeline.cpp
    src/main.cpp)

target_include_directories(cv_pipeline PRIVATE
    include
    ${OpenCV_INCLUDE_DIRS}
    ${TRT_ROOT}/include
    ${CUDAToolkit_INCLUDE_DIRS})

target_link_libraries(cv_pipeline PRIVATE
    ${OpenCV_LIBS}
    ${TRT_INFER} ${TRT_ONNX}
    CUDA::cudart)
```

---

## Getting Started (macOS — No GPU)

```bash
# Install OpenCV + ONNX Runtime (CPU / CoreML backend)
brew install opencv
vcpkg install onnxruntime

# Download YOLOv8n ONNX model
pip install ultralytics
yolo export model=yolov8n.pt format=onnx

# Build with ONNX fallback (no CUDA required)
cmake -B build -DUSE_TENSORRT=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# Run on webcam or video
./build/cv_pipeline --source 0          # webcam
./build/cv_pipeline --source video.mp4  # file
```

---

## Performance Targets

| Platform | Model | Precision | FPS |
|----------|-------|-----------|-----|
| M2 Pro (macOS, CoreML) | YOLOv8n | FP16 | ~40 FPS |
| RTX 4090 (TensorRT) | YOLOv8n | INT8 | ~1200 FPS |
| RTX 4090 (TensorRT) | YOLOv8x | FP16 | ~120 FPS |
| Jetson Orin NX (TensorRT) | YOLOv8s | INT8 | ~60 FPS |

---

## Stretch Goals

| Goal | Skill Gained |
|------|-------------|
| Multi-camera sync with ROS2 topics | Real robotics integration |
| Instance segmentation (YOLOv8-seg) | Mask decoding, polygon rendering |
| Object tracking (ByteTrack) | Kalman filter, data association |
| DeepStream integration | NVIDIA's production video analytics SDK |
| Depth estimation (DepthPro / MiDaS) | 3D scene understanding |
| Point cloud from stereo cameras | 3D perception fundamentals |

---

## Interview Topics This Covers

- "How do you pipeline camera → preprocess → inference to avoid stalling?"
- "What is memory coalescing and why does it matter for image preprocessing?"
- "How would you calibrate a TensorRT INT8 engine?"
- "What is NMS and what is its time complexity?"
- "How do you ensure thread safety when the detection results are read by the display thread?"
