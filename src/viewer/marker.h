#pragma once
#include "storage/sqlite_reader.h"
#include <opencv2/core.hpp>
#include <algorithm>
#include <cmath>

namespace dander {

struct MarkerStyle {
    int        radius;   // px
    cv::Scalar color;    // BGR
};

// Encode the 6 PMS size bins into a marker:
//   radius = log-scaled total particle count
//   color  = dominant size band (combustion / allergen / coarse)
inline MarkerStyle compute_marker(const LatestReading& r) {
    // PMS cnt_NN is CUMULATIVE: "particles >= NN.NN µm per 0.1L".
    // Convert to per-bin (non-negative).
    auto safe_sub = [](uint16_t a, uint16_t b) -> int {
        int v = static_cast<int>(a) - static_cast<int>(b);
        return v < 0 ? 0 : v;
    };
    int b_03_05 = safe_sub(r.cnt_03, r.cnt_05);
    int b_05_10 = safe_sub(r.cnt_05, r.cnt_10);
    int b_10_25 = safe_sub(r.cnt_10, r.cnt_25);
    int b_25_50 = safe_sub(r.cnt_25, r.cnt_50);
    int b_50_100= safe_sub(r.cnt_50, r.cnt_100);
    int b_100   = r.cnt_100;

    int total = b_03_05 + b_05_10 + b_10_25 + b_25_50 + b_50_100 + b_100;

    // Radius: log scale 10..58 px. total=0 -> 10, total=1000 -> ~58.
    int radius = 10;
    if (total > 0) {
        double l = std::log10(static_cast<double>(total) + 1.0);
        radius = 10 + static_cast<int>(l * 12);
    }
    radius = std::min(radius, 40);

    // Color by dominant size band (physical interpretation):
    //   small  (0.3-1.0 µm)  = combustion / cooking fumes
    //   medium (1.0-2.5 µm)  = cat allergen carrier (the band we care about)
    //   large  (>2.5 µm)     = skin flakes, textile fibers, dust
    int small  = b_03_05 + b_05_10;
    int medium = b_10_25;
    int large  = b_25_50 + b_50_100 + b_100;

    cv::Scalar color;
    if (small >= medium && small >= large) {
        color = cv::Scalar(255, 100, 0);    // blue: combustion
    } else if (medium >= small && medium >= large) {
        color = cv::Scalar(0, 200, 255);    // yellow: allergen carrier
    } else {
        color = cv::Scalar(0, 80, 200);     // red: coarse dust
    }
    return {radius, color};
}

} // namespace dander