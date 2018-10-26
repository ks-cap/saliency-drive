//
//  hogTools.hpp
//  saliency-drive
//
//  Created by 佐藤賢 on 2018/10/24.
//

#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxDLib.h"

#define PYRAMID 6

// Detector setting
typedef dlib::scan_fhog_pyramid<dlib::pyramid_down<PYRAMID> > image_scanner_type;
typedef dlib::object_detector<image_scanner_type> FHog;

class HogTool {
public:
    struct FHogData {
        cv::Rect rect;
        unsigned long id = 0;
        double confidence = -1;
    };

    struct Face {
        ofPoint center;
        float width = 0.0;
        float height = 0.0;
    };

    struct SaliencyRange {
        ofPoint center;
        float width = 0.0;
        float height = 0.0;
    };
    
    cv::Rect dlibRectangleToOpenCV(dlib::rectangle r) {
        return cv::Rect(cv::Point2i(r.left(),
                                    r.top()),
                        cv::Point2i(r.right() + 1, r.bottom() + 1));
    }
    dlib::rectangle openCVRectToDlib(cv::Rect r) {
        return dlib::rectangle((long)r.tl().x,
                               (long)r.tl().y,
                               (long)r.br().x - 1,
                               (long)r.br().y - 1);
    }

    //Load svm file without FHog.
    bool loadSVM(string file, FHog &hog) {
        ifstream fin(file, ios::binary);
        if (!fin)
        {
            cout << "Can't find a trained object detector file " << file << endl;
            return EXIT_FAILURE;
        }
        deserialize(hog, fin);
        bInit = true;
        return EXIT_SUCCESS;
    }

    bool loadMultiSVM(string path) {
        ofFile file(path);
        if (file.exists()) {
            clear();
            FHog pupil_detector = FHog();
            if (loadSVM(path, pupil_detector) == EXIT_SUCCESS) {
                detectors.push_back(pupil_detector);
                ofLog() << "Regist svm file >> " << path;
                bInit = true;
                return true;
            }
            else {
                return false;
            }
        }
        return false;
    }
    std::vector<HogTool::FHogData> multiUpdate(const cv::Mat& src) {
        if ((detectors.size() > 0) && (src.empty() == false) && bInit) {
            //Init
            detectResults.clear();
            //Update
            dlib::cv_image<dlib::bgr_pixel> cimg(src);
            evaluate_detectors(detectors, cimg, detectResults);

            //Todo
            //chack confidence
            //life time

            if (detectResults.size() > 0) {
                //Init
                FHogDatas.clear();
                for (auto rd : detectResults) {
                    if (rd.detection_confidence > 0) {
                        FHogData hog;
                        hog.rect = dlibRectangleToOpenCV(rd.rect);
                        hog.id = rd.weight_index;
                        hog.confidence = rd.detection_confidence;
                        FHogDatas.push_back(hog);
                    }
                }
            }
        }
        return FHogDatas;
    }
    void clear() {
        detectors.clear();
        bInit = false;
    }

    cv::Mat getFhogShapeMat(string file) {
        FHog tmp_detector;
        if (loadSVM(file, tmp_detector) == EXIT_SUCCESS) {
            dlib::matrix<unsigned char> svmImg = draw_fhog(tmp_detector);
            cv::Mat tmpMat = dlib::toMat(svmImg).clone();
            return tmpMat;
        }
        else {
            return cv::Mat::ones(100, 100, CV_8UC1) * 0;
        }
    }
private:
    // Detector
    std::vector<FHog> detectors;
    std::vector<dlib::rect_detection> detectResults;
    std::vector<HogTool::FHogData> FHogDatas;
    bool bInit = false;
};

