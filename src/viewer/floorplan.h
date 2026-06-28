#pragma once
#include <opencv2/core.hpp>

namespace dander {

// Maps physical meters to image pixels.
// Origin = top-left of the floor plan PNG; +x right, +y down (matches OpenCV).
struct Floorplan {
    double width_m;
    double height_m;
    int    width_px;
    int    height_px;

    cv::Point to_pixel(double x_m, double y_m) const {
        return cv::Point(
            static_cast<int>(x_m * width_px  / width_m),
            static_cast<int>(y_m * height_px / height_m)
        );
    }
};

} // namespace dander