#include <jni.h>
#include <string>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "FaceSwap.hpp"

//android编译dlib 使用gnustl_static round没有定义在std命名空间中 在image_transforms/random_cropper.h中定义
//把config.h 的 DLIB_NO_GUI_SUPPORT 打开


extern "C" {

FaceSwap *faceSwap = 0;
ANativeWindow *nativeWindow = 0;

JNIEXPORT void JNICALL
Java_com_dongnao_faceswap_FaceHelper_loadModel(JNIEnv *env, jclass type, jstring detectModel_,
                                               jstring landmarkModel_) {
    const char *detectModel = env->GetStringUTFChars(detectModel_, 0);
    const char *landmarkModel = env->GetStringUTFChars(landmarkModel_, 0);
    faceSwap = new FaceSwap(detectModel, landmarkModel);
    env->ReleaseStringUTFChars(detectModel_, detectModel);
    env->ReleaseStringUTFChars(landmarkModel_, landmarkModel);
}


JNIEXPORT void JNICALL
Java_com_dongnao_faceswap_FaceHelper_startTracking(JNIEnv *env, jclass type) {
    if (faceSwap && faceSwap->isInit) {
        faceSwap->startTracking();
    }
}

JNIEXPORT void JNICALL
Java_com_dongnao_faceswap_FaceHelper_stopTracking(JNIEnv *env, jclass type) {
    if (faceSwap && faceSwap->isInit) {
        faceSwap->stopTracking();
    }
}


JNIEXPORT void JNICALL
Java_com_dongnao_faceswap_FaceHelper_processFrame(JNIEnv *env, jclass type, jbyteArray data_,
                                                  jint w, jint h, jint rotation, jint camera_id,
                                                  jboolean isStart) {
    //摄像头数据
    jbyte *data = env->GetByteArrayElements(data_, NULL);
    //把摄像头数据放入opencv的mat
    Mat nv21Mat(h + h / 2, w, CV_8UC1, data);
    Mat rgbMat;
    //摄像头是nv21 转成bgr
    cvtColor(nv21Mat, rgbMat, CV_YUV2BGR_NV21);
    //需要逆时针旋转90度
    if (rotation == 0) {
        int angle;
        //后置
        if (camera_id == 0) {
            angle = -90;
        } else {
            angle = 90;
        }
        //旋转
        Mat matrix = getRotationMatrix2D(Point2f(w / 2, h / 2), angle, 1);
        warpAffine(rgbMat, rgbMat, matrix, Size(w, h));
        //旋转后宽高交换 会有黑边 截取
        getRectSubPix(rgbMat, Size(h, h),
                      Point2f(w / 2, h / 2), rgbMat);
    }
    if (isStart && faceSwap && faceSwap->isInit) {
        LOGI("process frame");
        vector<Rect> faces;
        //检测人脸
        faceSwap->detectorFace(rgbMat, faces);
        LOGI("找到%d张脸", faces.size());
        if (faces.size() >= 2) {
            //交换

            faceSwap->swapFaces(rgbMat, faces[0], faces[1]);
        }
        //画出检测到的人脸的框框
        for (int i = 0; i < faces.size(); ++i) {
            Rect face = faces[i];
            rectangle(rgbMat, face.tl(), face.br(), Scalar(0, 255, 255));
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
    cvtColor(rgbMat, rgbMat, CV_BGR2RGBA);
    resize(rgbMat, rgbMat, Size(windowBuffer.width, windowBuffer.height));
    memcpy(windowBuffer.bits, rgbMat.data, windowBuffer.height * windowBuffer.stride * 4);
    ANativeWindow_unlockAndPost(nativeWindow);


    end:
    nv21Mat.release();
    rgbMat.release();
    env->ReleaseByteArrayElements(data_, data, 0);
}


JNIEXPORT void JNICALL
Java_com_dongnao_faceswap_FaceHelper_destory(JNIEnv *env, jclass type) {
    if (faceSwap) {
        delete faceSwap;
        faceSwap = 0;
    }
    if (nativeWindow)
        ANativeWindow_release(nativeWindow);
    nativeWindow = 0;
}

JNIEXPORT void JNICALL
Java_com_dongnao_faceswap_FaceHelper_setSurface(JNIEnv *env, jclass type, jobject surface, jint w,
                                                jint h) {
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
}

