#include <jni.h>
#include <android/bitmap.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <opencv2/opencv.hpp>


#include <android/log.h>

#define LOG_TAG "native"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

extern "C" {

using namespace cv;
using namespace std;

void bitmap2Mat(JNIEnv *env, jobject bitmap, Mat &dst) {
#if 0
    AndroidBitmapInfo info;
    void *pixels = 0;
    //获得bitmap信息
    CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
    //必须是 rgba8888 rgb565
    CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888);
    //lock 获得数据
    CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
    CV_Assert(pixels);
    dst.create(info.height, info.width, CV_8UC3);
    LOGI("bitmap2Mat: RGBA_8888 bitmap -> Mat");
    Mat tmp(info.height, info.width, CV_8UC4, pixels);
    cvtColor(tmp, dst, COLOR_RGBA2BGR);
    // cvtColor(dst, dst, COLOR_RGBA2RGB);
    tmp.release();
    AndroidBitmap_unlockPixels(env, bitmap);
#else
    AndroidBitmapInfo info;
    void *pixels = 0;

    try {
        LOGI("nBitmapToMat");
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                  info.format == ANDROID_BITMAP_FORMAT_RGB_565);
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        CV_Assert(pixels);
        dst.create(info.height, info.width, CV_8UC4);
        if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            LOGI("nBitmapToMat: RGBA_8888 -> CV_8UC4");
            Mat tmp(info.height, info.width, CV_8UC4, pixels);
//            if (needUnPremultiplyAlpha) cvtColor(tmp, dst, COLOR_mRGBA2RGBA);
//            else
//            cvtColor(tmp, dst, COLOR_mRGBA2RGBA);
            tmp.copyTo(dst);
            cvtColor(dst, dst, COLOR_RGBA2BGR);
            tmp.release();
        } else {
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            LOGI("nBitmapToMat: RGB_565 -> CV_8UC4");
            Mat tmp(info.height, info.width, CV_8UC2, pixels);
            cvtColor(tmp, dst, COLOR_BGR5652BGR);
            tmp.release();
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    } catch (const cv::Exception &e) {
        AndroidBitmap_unlockPixels(env, bitmap);
        LOGI("nBitmapToMat catched cv::Exception: %s", e.what());
        jclass je = env->FindClass("org/opencv/core/CvException");
        if (!je) je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return;
    } catch (...) {
        AndroidBitmap_unlockPixels(env, bitmap);
        LOGI("nBitmapToMat catched unknown exception (...)");
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "Unknown exception in JNI code {nBitmapToMat}");
        return;
    }
#endif
}

ANativeWindow *nativeWindow = 0;
CascadeClassifier *faceClassifier = 0;

JNIEXPORT void JNICALL
Java_com_dongnao_facedetection_MainActivity_setSurface(JNIEnv *env, jobject instance,
                                                       jobject surface,
                                                       jint w, jint h) {
    if (surface && w && h) {
        if (nativeWindow) {
            LOGI("release old native window");
            ANativeWindow_release(nativeWindow);
            nativeWindow = 0;
        }
        LOGI("new native window");
        nativeWindow = ANativeWindow_fromSurface(env, surface);
        if (nativeWindow) {
            LOGI("set new native window buffer");
            ANativeWindow_setBuffersGeometry(nativeWindow, w, h,
                                             WINDOW_FORMAT_RGBA_8888);
        }
    } else {
        if (nativeWindow) {
            LOGI("release old native window");
            ANativeWindow_release(nativeWindow);
            nativeWindow = 0;
        }
    }
}

JNIEXPORT void JNICALL
Java_com_dongnao_facedetection_MainActivity_loadModel(JNIEnv *env, jclass type,
                                                      jstring detectModel_) {
    const char *detectModel = env->GetStringUTFChars(detectModel_, 0);

    faceClassifier = new CascadeClassifier(detectModel);

    env->ReleaseStringUTFChars(detectModel_, detectModel);
}

JNIEXPORT jboolean JNICALL
Java_com_dongnao_facedetection_MainActivity_process(JNIEnv *env, jobject instance, jobject bitmap) {

    int ret = 1;
    Mat src;
    bitmap2Mat(env, bitmap, src);
//    imwrite("/sdcard/img/a.png",src);
    if (faceClassifier) {
        vector<Rect> faces;
        Mat grayMat;
        cvtColor(src, grayMat, CV_BGR2GRAY);
        //直方图均衡化 增强对比效果
        equalizeHist(grayMat, grayMat);
//        imwrite("/sdcard/img/a.png",grayMat);
        //识别
        faceClassifier->detectMultiScale(grayMat, faces);
        grayMat.release();

        for (int i = 0; i < faces.size(); ++i) {
            Rect face = faces[i];
            rectangle(src, face.tl(), face.br(), Scalar(255, 0, 0));
        }
    }
    if (!nativeWindow) {
        LOGI("native window null");
        ret = 0;
        goto end;
    }
    ANativeWindow_Buffer windowBuffer;
    if (ANativeWindow_lock(nativeWindow, &windowBuffer, 0)) {
        LOGI("native window lock fail");
        ret = 0;
        goto end;
    }

//    imwrite("/sdcard/img/b.png",src);
    //直接画了 不传到java了
    cvtColor(src, src, CV_BGR2RGBA);
//    imwrite("/sdcard/img/c.png",src);
//    LOGI("%d-%d-%d-%d-%d", src.cols, src.rows, windowBuffer.stride, windowBuffer.width,
//         windowBuffer.height);
    resize(src, src, Size(windowBuffer.width, windowBuffer.height));
//    imwrite("/sdcard/img/d.png",src);
    memcpy(windowBuffer.bits, src.data, windowBuffer.height * windowBuffer.width * 4);

    ANativeWindow_unlockAndPost(nativeWindow);

    end:
    src.release();
    return ret;
}


JNIEXPORT void JNICALL
Java_com_dongnao_facedetection_MainActivity_destory(JNIEnv *env, jobject instance) {

    if (faceClassifier) {
        delete faceClassifier;
        faceClassifier = 0;
    }

    if (nativeWindow)
        ANativeWindow_release(nativeWindow);
    nativeWindow = 0;
}

}

