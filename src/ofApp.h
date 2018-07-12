#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "saliencySpecializedClasses.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include "opencv2/core/core.hpp"

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
  
  
  // 動画
  ofVideoPlayer player;
  
  // 出力データ（SPECTRAL_RESIDUAL）
  ofImage outputOfImg
  
  // 顕著性マップ
  Mat saliencyMap_SPECTRAL_RESIDUAL, saliencyMap_SPECTRAL_RESIDUAL_norm, saliencyMap_SPECTRAL_RESIDUAL_conv;
  
  // SPECTRAL_RESIDUAL(顕著性マップを求めるアルゴリズム : 画像)
  Ptr<StaticSaliencySpectralResidual> saliencyAlgorithm_SPECTRAL_RESIDUAL = StaticSaliencySpectralResidual::create();
  
};
