# Phase 2: Framework Integration (Months 4-6)

**Goal:** Understand the internals of modern AI frameworks (PyTorch), write custom operators, and learn how to deploy models using high-performance inference engines (ONNX Runtime/TensorRT).

---

## Month 4: PyTorch Internals & Custom C++ Extensions

### Learning Objectives
*   Understand the architecture of PyTorch (Python frontend -> Pybind11 -> ATen (C++ Tensor Library) -> CUDA/CPU kernels).
*   Learn how to write custom PyTorch operators in C++ and CUDA.
*   Understand PyTorch's Autograd system and how to write custom backward passes.

### Learning Resources
*   **Tutorial (Official):** [Custom C++ and CUDA Extensions (PyTorch Docs)](https://pytorch.org/tutorials/advanced/cpp_extension.html)
*   **Blog Post:** [A Tour of PyTorch Internals (Edward Z. Yang)](http://blog.ezyang.com/2019/05/pytorch-internals/) (Essential reading for understanding ATen and Autograd).
*   **YouTube:** [PyTorch Developer Conference Talks (Internals Track)](https://www.youtube.com/c/PyTorch)

### Project 4: Custom PyTorch Activation Function
**Task:** Implement a novel or optimized activation function (e.g., a fused Swish or GELU) as a PyTorch extension.
1.  **Forward Pass:** Write the C++ and CUDA kernel for the forward pass of the activation function.
2.  **Backward Pass:** Derive the derivative and write the C++/CUDA kernel for the backward pass (gradient computation).
3.  **PyTorch Integration:** Use `torch::RegisterOperators` or the `TORCH_LIBRARY` macro to register your custom op.
4.  **Training Loop:** Write a Python script that trains a small neural network (e.g., on MNIST or CIFAR-10) using your custom activation function. Compare the training speed and memory usage against the native PyTorch implementation.

---

## Month 5: LibTorch (PyTorch C++ API) & Standalone Inference

### Learning Objectives
*   Learn how to use LibTorch (the C++ backend of PyTorch) without Python.
*   Understand how to export PyTorch models using TorchScript.
*   Build standalone, high-performance C++ applications for ML inference.

### Learning Resources
*   **Tutorial (Official):** [Loading a TorchScript Model in C++](https://pytorch.org/tutorials/advanced/cpp_export.html)
*   **Documentation:** [LibTorch C++ API Reference](https://pytorch.org/cppdocs/)
*   **GitHub Repo:** [prabhuomkar/pytorch-cpp](https://github.com/prabhuomkar/pytorch-cpp) (Great examples of using LibTorch).

### Project 5: High-Performance C++ Inference Server
**Task:** Build a multi-threaded C++ application that serves a pre-trained model (e.g., ResNet50 for image classification).
1.  **Model Export:** Train or download a ResNet50 model in Python and export it to TorchScript (`.pt` file).
2.  **C++ Application:** Write a C++ program using LibTorch that loads the `.pt` model.
3.  **Multi-threading (Your Expertise):** Implement a custom thread pool (similar to your work at HPE) to handle incoming inference requests concurrently.
4.  **Batching:** Implement dynamic batching: group multiple incoming single-image requests into a single batch before passing them to the GPU to maximize throughput.
5.  **Benchmarking:** Measure the latency and throughput (images/second) of your C++ server compared to a simple Python Flask/FastAPI server.

---

## Month 6: Model Optimization (ONNX & TensorRT)

### Learning Objectives
*   Understand the ONNX (Open Neural Network Exchange) format.
*   Learn how to use ONNX Runtime (ORT) in C++ for cross-platform inference.
*   Learn NVIDIA TensorRT for maximizing inference performance on NVIDIA GPUs (kernel fusion, precision calibration).

### Learning Resources
*   **Documentation:** [ONNX Runtime C++ API](https://onnxruntime.ai/docs/api/c/)
*   **Documentation:** [NVIDIA TensorRT Developer Guide](https://docs.nvidia.com/deeplearning/tensorrt/developer-guide/index.html)
*   **Course (Free):** [NVIDIA DLI - Optimization and Deployment of Deep Learning Models with TensorRT](https://courses.nvidia.com/courses/course-v1:DLI+C-IV-02+V1/)

### Project 6: The Ultimate Inference Pipeline
**Task:** Take a complex model (e.g., YOLOv8 for object detection or a small Transformer) and optimize it for maximum GPU throughput.
1.  **Export to ONNX:** Export the PyTorch model to the ONNX format.
2.  **ONNX Runtime C++:** Write a C++ application using the ONNX Runtime API to run inference.
3.  **TensorRT Optimization:** Use `trtexec` or the TensorRT C++ API to compile the ONNX model into a highly optimized TensorRT engine. Experiment with FP16 (half-precision) and INT8 quantization.
4.  **End-to-End Pipeline:** Build a complete C++ pipeline: Read video frames (using OpenCV) -> Preprocess (resize, normalize) -> TensorRT Inference -> Postprocess (Non-Maximum Suppression for YOLO) -> Draw bounding boxes.
5.  **Performance Analysis:** Document the latency reduction and memory savings achieved by moving from PyTorch (FP32) -> ONNX Runtime -> TensorRT (FP16/INT8).