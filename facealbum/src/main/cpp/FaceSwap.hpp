//
//  FaceSwap.hpp
//  OpenCVLearn
//
//  Created by liuxiang on 2017/7/7.
//  Copyright © 2017年 liuxiang. All rights reserved.
//

#ifndef FaceDetector_hpp
#define FaceDetector_hpp

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include "dlib/opencv.h"
#include "dlib/image_processing.h"
#include "common.hpp"

using namespace cv;
using namespace std;


class FaceSwap {
public:
    FaceSwap(const char *detectModel, const char *landmarkModel);

    ~FaceSwap();

    void detectorFace(Mat mat, vector<Rect> &faces);

    void swapFaces(Mat &mat, Rect &rect_face1, Rect &rect_face2);

public:
    bool isInit = false;
private:
    dlib::full_object_detection faceLandmarkDetection(Mat mat, Rect rectangle);

    void faceLandmarkDetection(Mat frame, Rect rect, vector<Point2i> &landmark);

private:
    Ptr<CascadeClassifier> faceClassifier;
    //dlib 人脸特征提取
    dlib::shape_predictor shapePredictor;

};

#endif /* FaceDetector_hpp */
