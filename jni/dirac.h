#include <jni.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <set>
#include <algorithm>
#include <numeric>
#include <utility>
#include <vector>
#include <sstream>
#include <android/log.h>


#ifndef _Included_dirac_h
#define _Included_dirac_h

#define  LOG_TAG    "dirac"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_ysag_dirac_MainActivity
 * Method:    process
 * Signature: (J)V
 */
JNIEXPORT jint JNICALL Java_com_ysag_dirac_MainActivity_process
  (JNIEnv *, jobject, jlong, jlong, jboolean, jfloat, jfloat);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _Included_dirac_h
