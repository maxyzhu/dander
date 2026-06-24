#include <opencv2/opencv.hpp>
#include <storage/sqlite_reader.h>
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

    while (true) {
        rows=db_reader.latest_per_sensor(5000);
        for (const auto& row : rows) {
            printf("sensor=%u ts=%llu recv=%llu PM2.5=%u PM10=%u | counts: "
                "0.3=%u 0.5=%u 1.0=%u 2.5=%u 5.0=%u 10.0=%u\n",
                row.sensor_id, (unsigned long long)row.epoch_ms,
                (unsigned long long)row.recv_ms,
                row.pm2_5_atm, row.pm10_atm,
                row.cnt_03, row.cnt_05, row.cnt_10, row.cnt_25, row.cnt_50, row.cnt_100);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // Load floorplan image
    const std::string floorplan_path=root+"/assets/floorplan.png";  // avoid danging pointer
    const char* floorplan_p=floorplan_path.c_str(); // for cv::imread
    cv::Mat floorplan=cv::imread(floorplan_p);
    if (floorplan.empty()) {
        fprintf(stderr, "Imread failed: %s\n", floorplan_p);
        return 1;
    }
    cv::imshow("Floorplan", floorplan);
    cv::waitKey(0);
    return 0;
}