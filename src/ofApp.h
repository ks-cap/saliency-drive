#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"

#include "saliencySpecializedClasses.hpp"

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "iostream"

#include "ofxDLib.h"
#include "hogTools.hpp"
#include "saliencyTools.hpp"
#include "consts.hpp"

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
private:
    
    cv::Mat saliencyAlgorithm(cv::Mat mat);
    bool saliencyCheck(bool checkUI);
    void algorithmMinPixels(bool checkPixels);
    void hogGetRect();
    void saliencyMask();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    // 画像
    ofImage inputOfImg;
    // UI
    cv::Mat image;
    // 動画
    ofVideoPlayer player;
    // カメラ
    ofVideoGrabber vidGrabber;
    ofPixels videoInverted;
    int camWidth, camHeight;
    
    // 最小と最大の要素値とそれらの位置
    //    SaliencyTool::MinMax minMax;
    std::vector<int> pixelsList;

    // 出力データ（SPECTRAL_RESIDUAL, UI(画像)）
    ofImage outputOfImg_SPECTRAL_RESIDUAL, outputOfImg;
    
    cv::Mat frame;
    // 顕著性マップ
    cv::Mat saliencyMap_SPECTRAL_RESIDUAL, saliencyMap, saliencyMap_color;
    // マスク処理
    cv::Mat mask;
    // 顕著性マップ + マスク処理
    cv::Mat result;
    
    // 10*10の顕著マップの最小値の場所
    SaliencyTool::MinPlace minPlace;
    // 2回目以降の条件を分けるため
    bool firstFrameCheck;
    // UIを出した箇所が次のフレームで一定数値以下であればUIを動かさない
    bool algorithmCheck;
    
    // どちらを描画するか：画像（Picture）か動画（Map）か
    bool imgDraw;

    // 環境設定: release or prerelease or debug
    enum Consts::Use use;
    // 顔の矩形出力Mat
    cv::Mat hogFrame;
    
    HogTool hog;
    
    // 顔の範囲
    std::vector<HogTool::Face> face;
    // Saliency適応範囲
    std::vector<HogTool::SaliencyRange> saliencyRange;
    // Saliencyの矩形
    std::vector<cv::Rect> saliencyRect;
    
    std::vector<HogTool::FHogData> hogData;
    
//    struct FileName {
//        static const std::string circle = "circle.png";
//        static const std::string roadSign_speed = "roadSign_speed.png";
//        static const std::string roadSign_stop = "roadSign_stop.png";
//        static const std::string icon_mail = "icon_mail.png";
//        static const std::string string_picture = "string.png";
//        static const std::string map = "movie_map.mov";
//        static const std::string driver_daytime = "driver_daytime.mp4";
//        static const std::string driver_night = "driver_night.mp4";
//        static const std::string driver_daytime_long = "昼のドライブ映像.mp4";
//        static const std::string driver_night_long = "夜のドライブ映像.mp4";
//        static const std::string sampleMovie = "sampleMovie.mov";
//    };

//    FileName fileName;

};
