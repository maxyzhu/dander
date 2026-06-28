#include <onnxruntime_cxx_api.h>
#include <fstream>
#include <array>
#include <iostream>
#include <memory>

int main() {
    const char* model_path = DANDER_PROJECT_ROOT "/models/dander_field.onnx";

    // 1. Create environment with warning level： process (log/thread) warnings only
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "dander");
    // 2. Create default session options
    Ort::SessionOptions opts;

    // 3. Create session
    std::unique_ptr<Ort::Session> session;
    try {
        session=std::make_unique<Ort::Session>(env, model_path, opts);
    } catch (const Ort::Exception& e) {
        std::cerr << "Failed to create session: " << e.what() << std::endl;
        return 1;
    }

    std::array<float, 3> input{0.3f, 0.5f, 0.5f};
    std::array<int64_t, 2> in_shape{1, 3};
    // 4. Describe memory allocation: CPU-based arena allocator
    Ort::MemoryInfo mem=Ort::MemoryInfo::CreateCpu(
        OrtArenaAllocator, 
        OrtMemTypeDefault
    );
    // 5. Create input tensor value: shape (1,3), type float32
    Ort::Value in=Ort::Value::CreateTensor<float>(
        mem,
        input.data(),
        input.size(),
        in_shape.data(),
        in_shape.size()
    );

    // Run inference
    const char* in_names[]={"coords"};
    const char* out_names[]={"field"};
    auto out=session->Run(
        Ort::RunOptions{nullptr},
        in_names,
        &in,
        1,
        out_names,
        1
    );

    // Print output
    float* y=out[0].GetTensorMutableData<float>();
    auto shape=out[0].GetTensorTypeAndShapeInfo().GetShape();
    std::cout << "Output shape: [";
    for (auto d : shape) std::cout << d << " ";
    std::cout << "]\nStandard Output(8-channel): ";
    for (int i=0; i<8; i++) std::cout << y[i] << " ";
    std::cout << "\n";
    return 0;
}