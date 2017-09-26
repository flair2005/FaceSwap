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


class CascadeDetectorAdapter : public DetectionBasedTracker::IDetector {
public:
    CascadeDetectorAdapter(cv::Ptr<CascadeClassifier> detector) :
            IDetector(),
            detector(detector) {
        CV_Assert(detector);
    }

    void detect(const Mat &Image, vector<Rect> &objects) {
        detector->detectMultiScale(Image, objects, scaleFactor, minNeighbours, 0, minObjSize,
                                   maxObjSize);
    }

    ~CascadeDetectorAdapter() {
    }

private:
    CascadeDetectorAdapter();

    Ptr<CascadeClassifier> detector;
};


class FaceSwap {
public:
    FaceSwap(const char *detectModel, const char *landmarkModel);

    ~FaceSwap();

    void detectorFace(Mat &mat, vector<Rect> &faces);

    void swapFaces(Mat &mat, Rect &rect_face1, Rect &rect_face2);

    void startTracking();

    void stopTracking();

public:
    bool isInit = false;
private:
    dlib::full_object_detection faceLandmarkDetection(Mat mat, Rect rectangle);
    void faceLandmarkDetection(Mat frame, Rect rect, vector<Point2i>& landmark);

private:
    Ptr<DetectionBasedTracker> tracker;
    //dlib 人脸特征提取
    dlib::shape_predictor shapePredictor;

};

#endif /* FaceDetector_hpp */
