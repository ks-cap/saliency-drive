#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  
  // 画像の読み込み
  inputOfImg.load("cat.jpeg");
  inputOfImg.update();
  
  image = ofxCv::toCv( inputOfImg );
  resize( image, image, cv::Size(), 0.3, 0.3 );
  ofxCv::toOf( image, outputOfImg );
  outputOfImg.update();
  // 動画の読み込み
  ofBackground( 255,255,255 );
  ofSetVerticalSync( true );
  player.load( "test.mp4" );
  player.play();
  
}

//--------------------------------------------------------------
void ofApp::update(){
  player.update();
  
  if(player.isFrameNew()){
    // 1フレームを取得
    //    ofPixelsRef pix = player.getPixels();
    // Mat変換
    Mat mat = ofxCv::toCv( player ).clone();
    // 白黒加工
    cvtColor( mat, mat, COLOR_BGR2GRAY );
    
    // // 顕著性マップ(SPECTRAL_RESIDUAL)に変換
    saliencyAlgorithm_SPECTRAL_RESIDUAL->computeSaliency( mat.clone(), saliencyMap_SPECTRAL_RESIDUAL );

//    ofLog()<<"saliencyMap_SPECTRAL_RESIDUAL_at : "<<(int)saliencyMap_SPECTRAL_RESIDUAL.at<uchar>( 0, 0 );
    
    // アルファチャンネルの正規化を行う
    normalize( saliencyMap_SPECTRAL_RESIDUAL.clone(), saliencyMap_SPECTRAL_RESIDUAL_norm, 0.0, 255.0, NORM_MINMAX );
//    ofLog()<<"正規化 : "<<(int)saliencyMap_SPECTRAL_RESIDUAL_norm.at<uchar>( 0, 0 );
    
    // Matの型（ビット深度）を変換する
    saliencyMap_SPECTRAL_RESIDUAL_norm.convertTo( saliencyMap_SPECTRAL_RESIDUAL_conv, CV_8UC3 );
//    ofLog()<<"Matの型 : "<<(double)saliencyMap_SPECTRAL_RESIDUAL_conv.at<double>( 0, 0 );
    
    // 疑似カラー（カラーマップ）変換
    applyColorMap( saliencyMap_SPECTRAL_RESIDUAL_conv.clone(), saliencyMap_SPECTRAL_RESIDUAL_color, COLORMAP_JET );
    
    // 動画データ保存用：未実装
//    ofxCv::toOf( saliencyMap_SPECTRAL_RESIDUAL_color, outputOfImg_SPECTRAL_RESIDUAL );
    
    // データの各画素にアクセス
    ofLog()<<"x : "<<saliencyMap_SPECTRAL_RESIDUAL_conv.cols;
    ofLog()<<"y : "<<saliencyMap_SPECTRAL_RESIDUAL_conv.rows;
    
    for( int y = 0; y < saliencyMap_SPECTRAL_RESIDUAL_conv.rows; ++y ){
      for( int x = 0; x < saliencyMap_SPECTRAL_RESIDUAL_conv.cols; ++x ){
        // 画像のチャネル数分だけループ。白黒の場合は1回、カラーの場合は3回
        for( int c = 0; c < saliencyMap_SPECTRAL_RESIDUAL_conv.channels(); ++c ){
//          (int)saliencyMap_SPECTRAL_RESIDUAL_conv.at<uchar>( x, y );
          
        }
      }
    }

  }
  
}

//--------------------------------------------------------------
void ofApp::draw(){

  // 出力（動画）
  player.draw( 0, 0, 512, 384 );
  
  // 顕著性マップ(SPECTRAL_RESIDUAL)を出力
  ofxCv::drawMat( saliencyMap_SPECTRAL_RESIDUAL_conv, 0, 384, 512, 384 );
  
  // 顕著性マップ(SPECTRAL_RESIDUAL:カラーマップ)を出力
  ofxCv::drawMat( saliencyMap_SPECTRAL_RESIDUAL_color, 512, 384, 512, 384 );
  
  outputOfImg.draw( 200, 300 );
  
  // FPS表示
  ofDrawBitmapStringHighlight( ofToString(ofGetFrameRate()), 900, 20 );
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
