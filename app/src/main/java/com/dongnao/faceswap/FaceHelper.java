package com.dongnao.faceswap;

import android.view.Surface;

/**
 * Created by xiang on 2017/7/9.
 * 动脑学院 版权所有
 */

public class FaceHelper {

    static {
        System.loadLibrary("opencv_java3");
        System.loadLibrary("native-lib");
    }

    //这里不做成类了 static方便调用了
    public static native void loadModel(String detectModel, String landmarkModel);

    public static native void setSurface(Surface surface, int w, int h);

    public static native void startTracking();

    public static native void stopTracking();

    public static native void destory();

    public static native void processFrame(byte[] data, int w, int h, int rotation, int
            camera_id, boolean
            isStart);

}
