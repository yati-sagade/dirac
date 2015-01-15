#include "com_ysag_dirac_MainActivity.h"
#include "com_ysag_dirac_MainActivity_BlurType.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <algorithm>
#include <numeric>
#include <utility>
#include <vector>
#include <sstream>
#include <android/log.h>

using namespace std;

#define  LOG_TAG    "dirac"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

const cv::Scalar WHITE {255, 255, 255};
const cv::Scalar RED {255, 0, 0};
const cv::Scalar BLUE {0, 0, 255};

const size_t NUM_COMPARTMENTS = 12;


#ifdef __cplusplus
extern "C" {
#endif


static void draw_compartments(cv::Mat& img) {
    auto width = img.cols;
    auto height = img.rows;
    auto compartment_width = width / NUM_COMPARTMENTS;
    auto x = 0;
    for (auto i = 0; i < NUM_COMPARTMENTS + 1; ++i) {
        cv::line(img, cv::Point {x, 0}, cv::Point {x, height - 1}, WHITE, 2);
        x += compartment_width;
    }
}


static cv::Point center_of_gravity(const std::vector<cv::Point>& points) {
    size_t x = 0, y = 0;
    
    for (auto& point : points) {
        x += point.x;
        y += point.y;
    }

    return cv::Point {x / points.size(), y / points.size()};
}

static void activate_compartment(cv::Mat& img, unsigned short comp_width, size_t comp_index) {
    auto start = comp_width * comp_index;
    cv::Point top_left { start, 0 },
              bot_right { start + comp_width - 1, img.rows - 1 };
    cv::rectangle(img, top_left, bot_right, cv::Scalar {255, 0, 0, 100}, -1);
}

JNIEXPORT jint JNICALL Java_com_ysag_dirac_MainActivity_process(JNIEnv *env, jobject obj, jlong ptrToImgMat, jlong ptrToResultMat) {
    cv::Mat &img = *(cv::Mat*) ptrToImgMat; 
    cv::Mat *result = (cv::Mat*) ptrToResultMat;
    result->create(1, 1, CV_32SC1);
    result->at<long>(0, 0) = -1;

    auto compartment_width = img.cols / NUM_COMPARTMENTS;

    cv::Mat gray;
    cv::cvtColor(img, gray, CV_BGR2GRAY);
    cv::blur(gray, gray, cv::Size {3, 3});

    cv::Mat canny;
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::Canny(gray, canny, 50, 150, 3);
    cv::findContours(canny, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point {0, 0});

    auto largest_contour = std::max_element(contours.begin(), contours.end(),
                                            [] (std::vector<cv::Point>& a, std::vector<cv::Point>& b) {
                                                return a.size() < b.size();
                                            });

    if (largest_contour != contours.end()) {
        auto largest_index = largest_contour - contours.begin();
        cv::drawContours(img, contours, largest_index, cv::Scalar {0, 255, 0}, 2, 8, hierarchy, 0, cv::Point {});
        cv::Point cog = center_of_gravity(*largest_contour);
        cv::circle(img, cog, 2, BLUE, -1);

        auto comp_index = cog.x / compartment_width;
        result->at<long>(0, 0) = comp_index;
        cv::Point mid {img.cols / 2, img.rows / 2};
        std::stringstream ss;
        ss << comp_index;
        cv::putText(img, ss.str(), mid, cv::FONT_HERSHEY_PLAIN, 1.0, BLUE);
        activate_compartment(img, compartment_width, comp_index);
    }
    draw_compartments(img);
    return -1;
}

#ifdef __cplusplus
} // extern "C"
#endif
