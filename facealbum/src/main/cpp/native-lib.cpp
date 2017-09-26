#include <jni.h>
#include <android/bitmap.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "FaceSwap.hpp"

//android编译dlib 使用gnustl_static round没有定义在std命名空间中 在image_transforms/random_cropper.h中定义
//把config.h 的 DLIB_NO_GUI_SUPPORT 打开




extern "C" {


void bitmap2Mat(JNIEnv *env, jobject bitmap, Mat &dst) {
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
}

ANativeWindow *nativeWindow = 0;
FaceSwap *faceSwap = 0;
JNIEXPORT void JNICALL
Java_com_dongnao_facealbum_MainActivity_setSurface(JNIEnv *env, jobject instance, jobject surface,
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
Java_com_dongnao_facealbum_MainActivity_loadModel(JNIEnv *env, jclass type, jstring detectModel_,
                                                  jstring landmarkModel_) {
    const char *detectModel = env->GetStringUTFChars(detectModel_, 0);
    const char *landmarkModel = env->GetStringUTFChars(landmarkModel_, 0);

    faceSwap = new FaceSwap(detectModel, landmarkModel);

    env->ReleaseStringUTFChars(detectModel_, detectModel);
    env->ReleaseStringUTFChars(landmarkModel_, landmarkModel);
}

JNIEXPORT void JNICALL
Java_com_dongnao_facealbum_MainActivity_process(JNIEnv *env, jobject instance, jobject bitmap) {

    Mat src;
    bitmap2Mat(env, bitmap, src);
    if (faceSwap && faceSwap->isInit) {
        vector<Rect> faces;
        faceSwap->detectorFace(src, faces);
        LOGI("找到%d张脸", faces.size());
        if (faces.size() >= 2) {
            //交换
            LOGI("交换====");
            faceSwap->swapFaces(src, faces[0], faces[1]);
        }
    }
    if (!nativeWindow) {
        LOGI("native window null");
        goto end;
    }
    ANativeWindow_Buffer windowBuffer;
    if (ANativeWindow_lock(nativeWindow, &windowBuffer, 0)) {
        LOGI("native window lock fail");
        goto end;
    }

    //直接画了 不传到java了
    cvtColor(src, src, CV_BGR2RGBA);
    resize(src, src, Size(windowBuffer.width, windowBuffer.height));
    memcpy(windowBuffer.bits, src.data, windowBuffer.height * windowBuffer.stride * 4);
    ANativeWindow_unlockAndPost(nativeWindow);

    end:
    src.release();
}


JNIEXPORT void JNICALL
Java_com_dongnao_facealbum_MainActivity_destory(JNIEnv *env, jobject instance) {

    if (faceSwap) {
        delete faceSwap;
        faceSwap = 0;
    }

    if (nativeWindow)
        ANativeWindow_release(nativeWindow);
    nativeWindow = 0;
}

}

