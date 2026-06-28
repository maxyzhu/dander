#include "inference/field_model.h"
#include "viewer/floorplan.h"
#include <opencv2/opencv.hpp>

int main() {
    const std::string root = DANDER_PROJECT_ROOT;
    dander::FieldModel model(root + "/models/dander_field.onnx",
                             root + "/models/dander_field.norm.json");

    cv::Mat base = cv::imread(root + "/assets/floorplan.png");
    const double W = 14.1, H = 7.7;
    const int GW = 70, GH = 38;          // coarse grid (~0.2m) for speed
    const double t = 1782522300000.0;    // an epoch_ms near cooking time (randomly chosen)
    const int ch = 1;                    // channel 1 = PM2.5

    // fill CH=1 float field with model predictions
    cv::Mat field(GH, GW, CV_32F);
    for (int j = 0; j < GH; ++j)
        for (int i = 0; i < GW; ++i) {
            double x = (i + 0.5) * W / GW;
            double y = (j + 0.5) * H / GH;
            field.at<float>(j, i) = model.predict(x, y, t)[ch];
        }

    // normalize → color → resize → blend
    cv::Mat u8, color, blended;
    cv::normalize(field, field, 0, 255, cv::NORM_MINMAX);
    field.convertTo(u8, CV_8U);
    cv::applyColorMap(u8, color, cv::COLORMAP_JET);
    cv::resize(color, color, base.size());
    cv::addWeighted(base, 0.5, color, 0.5, 0, blended);

    cv::imshow("Dander Field (PM2.5)", blended);
    cv::waitKey(0);
    return 0;
}