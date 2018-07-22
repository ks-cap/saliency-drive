#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "saliencySpecializedClasses.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "iostream"

using namespace std;
using namespace cv;
using namespace saliency;

class ofApp : public ofBaseApp{
  
public:
  void setup();
  void update();
  void draw();
  
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
  Mat image;
  // 動画
  ofVideoPlayer player;
  
  // minMaxLocで使用
  cv::Point min_loc, max_loc;
  double min_val, max_val;
  
  // 出力データ（SPECTRAL_RESIDUAL, UI(画像)）
  ofImage outputOfImg_SPECTRAL_RESIDUAL, outputOfImg;
  // 顕著性マップ
  Mat saliencyMap, saliencyMap_norm, saliencyMap_conv, saliencyMap_color;

  // SPECTRAL_RESIDUAL(顕著性マップを求めるアルゴリズム : 画像)
  Ptr<StaticSaliencySpectralResidual> saliencyAlgorithm_SPECTRAL_RESIDUAL = StaticSaliencySpectralResidual::create();

  ofVideoGrabber vidGrabber;
  ofPixels videoInverted;
//  ofTexture videoTexture;
  int camWidth;
  int camHeight;
};
