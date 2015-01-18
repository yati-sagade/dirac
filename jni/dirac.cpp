#include "dirac.h"

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

JNIEXPORT jint JNICALL Java_com_ysag_dirac_MainActivity_find(JNIEnv *env,
                                                             jobject obj,
                                                             jlong ptrToImgMat)
{
    cv::Mat &img = *(cv::Mat*) ptrToImgMat;
    cv::cvtColor(img, img, CV_RGB2HSV); 
    cv::Scalar lo {165, 240, 143}, hi {178, 255, 255};
    cv::Mat bw;
    cv::inRange(img, lo, hi, bw);
    *((cv::Mat*) ptrToImgMat) = bw;
}

JNIEXPORT jint JNICALL Java_com_ysag_dirac_MainActivity_process(JNIEnv *env,
                                                                jobject obj,
                                                                jlong ptrToImgMat,
                                                                jlong ptrToResultMat,
                                                                jboolean onlyRects,
                                                                jfloat minAreaFraction,
                                                                jfloat maxAreaFraction) {
    cv::Mat &img = *(cv::Mat*) ptrToImgMat; 
    cv::Mat *result = (cv::Mat*) ptrToResultMat;

    auto compartment_width = img.cols / NUM_COMPARTMENTS;

    cv::Mat gray;
    cv::cvtColor(img, gray, CV_BGR2GRAY);
    cv::blur(gray, gray, cv::Size {3, 3});

    cv::Mat canny;
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::Canny(gray, canny, 50, 150, 3);
    cv::findContours(canny, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point {0, 0});

    std::set<int> active_compartments;

    cv::Point mid {img.cols / 2, img.rows / 2};
    std::stringstream ss;
    for (int i = 0; i < contours.size(); ++i) {
        auto& contour = contours[i];

        auto maxdist = cv::arcLength(contour, /* closed= */ true) * 0.02;
        cv::approxPolyDP(contour, contour, maxdist, /* closed= */ true);
        
        if (contour.size() == 4) {
            cv::drawContours(img, contours, i, cv::Scalar {0, 255, 0}, 2, 8, hierarchy, 0, cv::Point {});
            cv::Point cog = center_of_gravity(contour);
            cv::circle(img, cog, 2, BLUE, -1);
            auto comp_index = cog.x / compartment_width;
            if (active_compartments.find(comp_index) == active_compartments.end()) {
                active_compartments.insert(comp_index);
                activate_compartment(img, compartment_width, comp_index);
                ss << comp_index;
                if (i != contours.size() - 1) {
                    ss << ",";
                }
            }
        }
    }
    cv::putText(img, ss.str(), mid, cv::FONT_HERSHEY_PLAIN, 1.0, BLUE);
    draw_compartments(img);

    result->create(active_compartments.size(), 1, CV_32SC1);
    int i = 0;
    for (auto active_compartment : active_compartments) {
        result->at<long>(i++, 0) = active_compartment;
    }

    return -1;
}

#ifdef __cplusplus
} // extern "C"
#endif
