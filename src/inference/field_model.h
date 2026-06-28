#pragma once

#include <onnxruntime_cxx_api.h>
#include <string>
#include <array>

namespace dander {

class FieldModel {
public:
    explicit FieldModel(const std::string& model_path, const std::string& norm_path);
    
    // Predict the air quality at a given position and time
    std::array<float, 8> predict(double x, double y, double t) const;
private:
    Ort::Env env_;
    std::unique_ptr<Ort::Session> session_;
    double t0_, t1_, width_m_, height_m_;
    std::array<float, 8> y_mean_, y_std_;
};

} // namespace dander