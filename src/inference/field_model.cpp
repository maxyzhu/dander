#include "field_model.h"
#include <onnxruntime_cxx_api.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <cmath>
#include <stdexcept>

namespace dander {

FieldModel::FieldModel(const std::string& model_path, const std::string& norm_path)
: env_(ORT_LOGGING_LEVEL_WARNING, "dander") {
    // Load norm.json
    std::ifstream f(norm_path);
    if (!f) {throw std::runtime_error("Failed to open norm.json");}

    // Parse norm.json
    auto j=nlohmann::json::parse(f);
    t0_=j["t0"];
    t1_=j["t1"];
    width_m_=j["width_m"];
    height_m_=j["height_m"];
    auto m=j["y_mean"].get<std::vector<float>>();
    auto s=j["y_std"].get<std::vector<float>>();
    std::copy_n(m.begin(), 8, y_mean_.begin());
    std::copy_n(s.begin(), 8, y_std_.begin());

    // Create session
    Ort::SessionOptions opts;
    session_=std::make_unique<Ort::Session>(env_, model_path.c_str(), opts);
    if (!session_) {throw std::runtime_error("Failed to create session");}
}

std::array<float, 8> FieldModel::predict(double x, double y, double t) const {
    // Normalize input
    std::array<float, 3> in{
        float(x/width_m_), float(y/height_m_), float((t-t0_)/(t1_-t0_))
    };
    std::array<int64_t, 2> in_shape{1, 3};

    // Create input tensor value
    auto mem=Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value tin=Ort::Value::CreateTensor<float>(
        mem, in.data(), in.size(), in_shape.data(), in_shape.size()
    );
    if (!tin) {throw std::runtime_error("Failed to create input tensor value");}

    // Run inference
    const char* in_names[]={"coords"};
    const char* out_names[]={"field"};
    auto out=session_->Run(
        Ort::RunOptions{nullptr},
        in_names,
        &tin, 1,
        out_names, 1
    );
    if (out.empty()) {throw std::runtime_error("Failed to run inference");}
    
    const float* raw=out[0].GetTensorData<float>();

    // Denormalize output
    std::array<float, 8> real;
    for (int i=0; i<8; i++) {
        real[i]=std::expm1(raw[i]*y_std_[i]+y_mean_[i]);
    }
    return real;
}

} // namespace dander