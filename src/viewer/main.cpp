#include "storage/sqlite_reader.h"
#include <opencv2/opencv.hpp>
#include "viewer/floorplan.h"
#include "viewer/marker.h"
#include <cstdio>
#include <string>
#include <thread>

int main() {
    // Root directory of the project
    const std::string root=DANDER_PROJECT_ROOT;

    // Open database
    dander::SqliteReader db_reader;
    if (!db_reader.open("dander.db")) {
        fprintf(stderr, "Failed to open database\n");
        return 1;
    }
    printf("dander-viewer: database opened\n");

    // Load locations from DB
    auto locations = db_reader.active_locations();
    if (locations.empty()) {
        fprintf(stderr, "No active sensor locations in DB — did you INSERT into sensor_locations?\n");
        return 1;
    }

    // Load floorplan image
    const std::string floorplan_path=root+"/assets/floorplan.png";  // avoid danging pointer
    const char* floorplan_p=floorplan_path.c_str(); // for cv::imread
    cv::Mat floorplan_base=cv::imread(floorplan_p);
    if (floorplan_base.empty()) {
        fprintf(stderr, "Imread failed: %s\n", floorplan_p);
        return 1;
    }

    dander::Floorplan fp{
        /*width_m=*/  14.1,
        /*height_m=*/ 7.7,
        /*width_px=*/ floorplan_base.cols,
        /*height_px=*/floorplan_base.rows,
    };

    while (true) {
        auto rows=db_reader.latest_per_sensor(5000);
        // Index readings by sensor_id
        std::unordered_map<uint8_t, dander::LatestReading> by_id;
        for (const auto& r : rows) by_id[r.sensor_id]=r;
        // Clean copy of the floor plan
        cv::Mat canvas=floorplan_base.clone();
        
        for (const auto& loc : locations) {
            cv::Point p=fp.to_pixel(loc.x_m, loc.y_m);
            auto it=by_id.find(loc.sensor_id);

            if (it!=by_id.end()) {
                // Live data: red filled circle + label
                const auto& r=it->second;
                auto m=dander::compute_marker(r);
                cv::circle(canvas, p, m.radius, m.color, -1);
                cv::circle(canvas, p, m.radius+1, cv::Scalar(0, 0, 0), 1);
                // Label
                char buf[128];
                snprintf(buf, sizeof(buf), 
                    "S%u PM1=%u, PM2.5=%u, PM10=%u,\ncnt03=%u, cnt10=%u, cnt25=%u,\ncnt50=%u, cnt100=%u",
                    loc.sensor_id, r.pm1_0_atm, r.pm2_5_atm, r.pm10_atm, r.cnt_03, r.cnt_10, r.cnt_25, r.cnt_50, r.cnt_100);
                    cv::putText(canvas, buf, p+cv::Point(m.radius+5,5),
                            cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(0,0,0), 5, cv::LINE_AA);
                    cv::putText(canvas, buf, p+cv::Point(m.radius+5,5),
                            cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(255,255,255), 2, cv::LINE_AA);
            } else {
                // No data: gray outline circle
                cv::circle(canvas, p, 20, cv::Scalar(128,128,128),2);
            }
        }
        cv::imshow("Dander", canvas);
        int key=cv::waitKey(33);    // ~30fps, also drives the GUI event loop
        if (key=='q'||key==27) break;   // q or ESC
    }
    return 0;
}