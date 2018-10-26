#include "ofApp.h"

// UIの位置を変更する画素値の条件（0:顕著性が高い, 255:顕著性が低い）
#define SALIENCY_IMG 391680   // 9216回 * 42.5(255/6)
#define SALIENCY_MAP 1566720  // 36864回 * 42.5(255/6)

#define SALIENCY_RANGE 2.5

#define WIDTHCOUNT 5
#define HEIGHTCOUNT 3

//--------------------------------------------------------------
void ofApp::setup(){

    // 背景色: White
    ofBackground(255, 255, 255);

    // 10*10の顕著マップの最小値の場所
    minPlace.widthMin = 0;
    minPlace.heightMin = 0;

    // 1回目と判定
    firstFrameCheck = true;
    // UIを出した箇所が次のフレームで一定数値以下であればUIを動かさない
    algorithmCheck = true;
    // 表示しているUIが画像か動画（地図）か
    imgDraw = false;
    mapDraw = false;

    // 動画の読み込み
    ofSetVerticalSync(true);
    // Hogのサンプルデータ読み込み
    hog.loadMultiSVM(ofToDataPath("face_detector.svm"));

    //---------------------   Camera   -----------------------------
    // カメラの設定
    //      camWidth = 1280;
    //      camHeight = 720;
    //
    //      vector<ofVideoDevice> devices = vidGrabber.listDevices();
    //
    //      for(size_t i = 0; i < devices.size(); i++){
    //        if(devices[i].bAvailable){
    //          ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
    //        }else{
    //          ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
    //        }
    //      }
    //
    //      vidGrabber.setDeviceID(0);
    //      vidGrabber.setDesiredFrameRate(60);
    //      vidGrabber.initGrabber(camWidth, camHeight);

}

//--------------------------------------------------------------
void ofApp::update(){
    // 動画の場合
    player.update();

    if (mapDraw) { player_map.update(); }

    if(player.isFrameNew()){
        // Mat変換
        frame = ofxCv::toCv(player).clone();
        // 画質を半分に下げる
        cv::pyrDown(frame.clone(), downFrame);

        hogData = hog.multiUpdate(downFrame);
        // 顔認識
        hogGetRect();
        // 顕著性マップ作成
        saliencyAlgorithm(downFrame);

        //---------------------   Camera   -----------------------------
        //          vidGrabber.update();
        //
        //          if( vidGrabber.isFrameNew() ){
        //            ofPixels & pixels = vidGrabber.getPixels();

        //            saliencyAlgorithm( ofxCv::toCv( pixels ).clone() );
        //--------------------------------------------------------------

        // 最小と最大の要素値とそれらの位置を求める
        //        minMaxLoc(saliencyMap, &minMax.min_val, &minMax.max_val, &minMax.min_loc, &minMax.max_loc, cv::Mat());
        //        ofLog()<<"minMaxLoc.min_val: "<<minMax.min_val;
        //        ofLog()<<"minMaxLoc.min_loc: "<<minMax.min_loc;
        // マスク処理
        saliencyMask();
        // updateが2回目以降もしくはボタンを押されてupdateが2回目以降に呼ばれた場合, if文の中に入る
//        saliencyCheck(firstFrameCheck);

        // 5*3のうちの画素最小値の場所を取得
        algorithmMinPixels(algorithmCheck);

        // 画素値の反転（現状 : 0:黒:顕著性が低い, 255:白:顕著性が高い）
        for(int y = 0; y < result.cols; ++y){
            for(int x = 0; x < result.rows; ++x){
                result.at<uchar>( x, y ) = 255 - (int)result.at<uchar>(x, y);
                //        ofLog()<<"(int)saliencyMap.at<uchar>("<<x<<","<<y<< ") : "<<(int)saliencyMap.at<uchar>( x, y );
            }
        }

        // 疑似カラー（カラーマップ）変換 :（0:赤:顕著性が高い, 255:青:顕著性が低い）
        applyColorMap( result.clone(), saliencyMap_color, cv::COLORMAP_JET );

        printf("----------------------------------------------------\n");
    }

}

//--------------------------------------------------------------
void ofApp::draw(){

    //---------------------   Camera   -----------------------------
    //    ofSetHexColor(0xffffff);
    //    vidGrabber.draw(0, 0, 640, 360 );
    //--------------------------------------------------------------

    // 出力（動画）
    switch (use) {
        case Consts::release:
            player.draw(0, 0, ofGetWidth(),ofGetHeight());
            break;

        case Consts::preRelease:
            // 顕著性マップ(SPECTRAL_RESIDUAL) + saliency適応範囲を出力: Debug用
            ofxCv::drawMat(saliencyMap_color, 0, 0, ofGetWidth(),ofGetHeight());
            break;

        case Consts::debug:
            player.draw(0, 0, ofGetWidth()/3, ofGetHeight()/2);
            // 顔検知出力(Hog)
            ofxCv::drawMat(downFrame, ofGetWidth()/3, 0, ofGetWidth()/3, ofGetHeight()/2);
            // 顕著性マップ(SPECTRAL_RESIDUAL)を出力
            ofxCv::drawMat(saliencyMap, ofGetWidth()-ofGetWidth()/3, 0, ofGetWidth()/3, ofGetHeight()/2);
            // 顔の矩形以外マスク処理
            ofxCv::drawMat(mask, 0, ofGetHeight()/2, ofGetWidth()/3, ofGetHeight()/2);
            // 顕著性マップ(SPECTRAL_RESIDUAL) + saliency適応範囲を出力
            ofxCv::drawMat(result, ofGetWidth()/3, ofGetHeight()/2, ofGetWidth()/3, ofGetHeight()/2);
            // 顕著性マップ(SPECTRAL_RESIDUAL) + saliency適応範囲を出力(color)
            ofxCv::drawMat(saliencyMap_color, ofGetWidth()-ofGetWidth()/3, ofGetHeight()/2, ofGetWidth()/3, ofGetHeight()/2);

            // Label
            ofDrawBitmapStringHighlight("original", 20, 20);
            ofDrawBitmapStringHighlight("hog", ofGetWidth()/3+20, 20);
            ofDrawBitmapStringHighlight("saliencyMap", ofGetWidth()-ofGetWidth()/3+20, 20);
            ofDrawBitmapStringHighlight("mask", 20, ofGetHeight()/2+20);
            ofDrawBitmapStringHighlight("result", ofGetWidth()/3+20, ofGetHeight()/2+20);
            ofDrawBitmapStringHighlight("saliencyRange", ofGetWidth()-ofGetWidth()/3+20, ofGetHeight()/2+20);
            // FPS表示
            ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 1200, 20);
            break;
    }

    // UI画像
    if (imgDraw){ outputOfImg.draw(minPlace.widthMin, minPlace.heightMin); }
    if (mapDraw){ player_map.draw(minPlace.widthMin, minPlace.heightMin, ofGetWidth()/5, ofGetHeight()/5); }

    // データの初期化
    if(!face.empty()) { face.clear(); }
    if(!saliencyRange.empty()) { saliencyRange.clear(); }
    if(!saliencyRect.empty()) { saliencyRect.clear(); }

}

//--------------------------------------------------------------
cv::Mat ofApp::saliencyAlgorithm(cv::Mat mat){
    cv::Mat mat_gray, mat_gaus, saliencyMap_norm;
    // 白黒加工
    cv::cvtColor(mat.clone(), mat_gray, cv::COLOR_BGR2GRAY);
    // ぼかし
    cv::GaussianBlur(mat_gray.clone(), mat_gaus, cv::Size(5, 5), 1, 1);
    // SPECTRAL_RESIDUAL(顕著性マップを求めるアルゴリズム)
    cv::Ptr<cv::saliency::Saliency> saliencyAlgorithm;
    saliencyAlgorithm = cv::saliency::StaticSaliencySpectralResidual::create();
    saliencyAlgorithm->computeSaliency(mat_gaus.clone(), saliencyMap_SPECTRAL_RESIDUAL);

    // アルファチャンネルの正規化を行う
    cv::normalize( saliencyMap_SPECTRAL_RESIDUAL.clone(), saliencyMap_norm, 0.0, 255.0, cv::NORM_MINMAX );
    // Matの型（ビット深度）を変換する
    saliencyMap_norm.convertTo( saliencyMap, CV_8UC3 );

    return saliencyMap;
}

//--------------------------------------------------------------
bool ofApp::saliencyCheck(bool checkUI){
    if ( !checkUI ){
        // 省略記載：（注意）falseだからといってmapDraw = trueとは限らない
        // 今は firstFrameCheck で条件を発火させ, ボタンを押した直後は入らないようにしている
        //        int count = imgDraw ? 10 : 5;

        // 前回の顕著性マップで顕著性が低かったピクセルのうちの一つ
        cv::Rect roi(minPlace.widthMin, minPlace.heightMin, saliencyMap.cols / WIDTHCOUNT, saliencyMap.rows / HEIGHTCOUNT);
        cv::Mat saliency_roi = saliencyMap(roi);
        int pixels = 0;
        // 10*10のうちの一つの画素値(count: 9216回 * 42.5(255/6))
        // 5*5のうちの一つの画素値(count: 36864回　* 42.5(255/6))
        for(int y = 0; y < saliency_roi.cols; ++y){
            for(int x = 0; x < saliency_roi.rows; ++x){
                pixels += (int)saliency_roi.at<uchar>(x, y);
            }
        }
        // UIを出した箇所が次のフレームで一定数値以下であればUIを動かさないフラグを設定
        int saliency = imgDraw ? SALIENCY_IMG : SALIENCY_MAP;
        algorithmCheck = pixels < saliency ? false : true ;

    } else {
        // 初回のチェックをなくす
        firstFrameCheck = false;
        algorithmCheck = true;
    }
    return algorithmCheck;
}

//--------------------------------------------------------------
void ofApp::algorithmMinPixels(bool checkPixels){

    if (checkPixels) {
        // 処理領域を設定
        int height = 0;
        int minPixels = 0;

        for (int heightCount = 0; heightCount < HEIGHTCOUNT; ++heightCount) {
            int width = 0;
            for (int widthCount = 0; widthCount < WIDTHCOUNT; ++widthCount) {
                // cols： 画像の幅（※行列の列数に対応）|| rows： 画像の高さ（※行列の行数に対応）
                cv::Rect roi(width, height, result.cols / WIDTHCOUNT, result.rows / HEIGHTCOUNT);
                cv::Mat result_roi = result(roi);

                // 大きな範囲のうちの一箇所の画素値の合計
                int pixel = 0;
                for (int y = 0; y < result_roi.cols; ++y) {
                    for(int x = 0; x < result_roi.rows; ++x){
                        pixel += (int)result_roi.at<uchar>(x, y);
                    }
                }
                // 最小値の値とその場所を更新
                if ((heightCount == 0 && widthCount == 0) || pixel < minPixels) {
                    minPixels = pixel;
                    minPlace.widthMin = width;
                    minPlace.heightMin = height;
                    ofLogNotice()<<"pixelsMin["<<width<<", "<<height<<"]: "<<minPixels;
                }

                ofLogNotice()<<"pixels["<<width<<", "<<height<<"]: "<<pixel;
                pixelsList.push_back(pixel);

                width += result.cols / WIDTHCOUNT;
            }
            height += result.rows / HEIGHTCOUNT;
        }
    }

}

//--------------------------------------------------------------
void ofApp::hogGetRect(){
    for (auto data : hogData) {
        cv::rectangle(downFrame, data.rect, cv::Scalar(255, 0, 0), 2, CV_AA);
        cv::Rect rect = data.rect;
        ofLog()<<"rect"<<"["<<data.id<<"].x: "<< rect.x;
        ofLog()<<"rect"<<"["<<data.id<<"].y: "<< rect.y;
        ofLog()<<"rect"<<"["<<data.id<<"].width: "<< rect.width;
        ofLog()<<"rect"<<"["<<data.id<<"].height: "<< rect.height;

        // 顔の矩形
        ofRectangle rectangle = ofxCv::toOf(rect);

        HogTool::Face f;
        f.center += rectangle.getCenter();
        f.width = rectangle.getWidth();
        f.height = rectangle.getHeight();
        face.push_back(f);

        HogTool::SaliencyRange s;
        s.center = face[data.id].center;
        s.width = face[data.id].width * SALIENCY_RANGE;
        s.height = face[data.id].height * SALIENCY_RANGE;
        saliencyRange.push_back(s);

        cv::Rect _s;
        _s.x = saliencyRange[data.id].center.x - (saliencyRange[data.id].width / 2);
        _s.y = saliencyRange[data.id].center.y - (saliencyRange[data.id].height / 2);
        _s.height = saliencyRange[data.id].height;
        _s.width = saliencyRange[data.id].width;
        saliencyRect.push_back(_s);

        cv::rectangle(downFrame, _s, cv::Scalar(0, 0, 255), 2, CV_AA);

        ofLog()<<"saliencyRect"<<"["<<data.id<<"].x: "<< saliencyRect[data.id].x;
        ofLog()<<"saliencyRect"<<"["<<data.id<<"].y: "<< saliencyRect[data.id].y;
        ofLog()<<"saliencyRect"<<"["<<data.id<<"].width: "<< saliencyRect[data.id].width;
        ofLog()<<"saliencyRect"<<"["<<data.id<<"].height: "<< saliencyRect[data.id].height;
    }
}

//--------------------------------------------------------------
void ofApp::saliencyMask(){
    // saliency適応範囲以外をマスク
    result = cv::Mat();
//    mask = cv::Mat::ones(saliencyMap.rows, saliencyMap.cols, CV_8UC1);
    mask = cv::Mat::zeros(saliencyMap.rows, saliencyMap.cols, CV_8UC1);

    ofLog()<<"saliencyMap.channels: "<< saliencyMap.channels();
    ofLog()<<"mask.channels: "<< mask.channels();

    for (int i = 0; i < (int)saliencyRect.size(); i++ ) {
        cv::rectangle(mask, saliencyRect[i], cv::Scalar(255, 255, 255), -1, CV_8UC3);
//        cv::rectangle(mask, saliencyRect[i], cv::Scalar(0, 0, 0), -1, CV_8UC3);
    }

    saliencyMap.copyTo(result, mask.clone());
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    printf("keyPressed: %d\n", key);
    // 条件を発火させ, ボタンを押した直後は saliencyCheck 関数に入らないようにしている
    firstFrameCheck = true;

    enum Consts::File file;
    
    switch (key) {
            //-------------   UI   ------------------
            // "1"を押した時 単純形状表示
        case 49:
            inputOfImg.load("circle.png");
            file = Consts::png;
            break;
            // "2"を押した時 道路の標識（速度制限）表示
        case 50:
            inputOfImg.load("roadSign_speed.png");
            file = Consts::png;
            break;
            // "3"を押した時 道路の標識（停止）表示
        case 51:
            inputOfImg.load("roadSign_stop.png");
            file = Consts::png;
            break;
            // "4"を押した時: メールのアイコン表示
        case 52:
            inputOfImg.load("icon_mail.png");
            file = Consts::png;
            break;
            // "5"を押した時: マップ表示
        case 53:
            inputOfImg.load("string.png");
            file = Consts::png;
            break;
            // "6"を押した時: マップ表示
        case 54:
            player_map.load("movie_map.mov");
            file = Consts::mov;
            break;
            // "7"を押した時 道路の標識（速度制限）表示: 半透明
        case 55:
            inputOfImg.load("roadSign_speed2.png");
            file = Consts::png;
            break;
            //-------------   動画データ   ------------------
            // "A"を押した時: 昼のドライブ映像
        case 97:
            player.load("driver_daytime.mp4");
            file = Consts::mp4;
            player.play();
            break;
            // "S"を押した時: 夜のドライブ映像
        case 115:
            player.load("driver_night.mp4");
            file = Consts::mp4;
            player.play();
            break;
            // "D"を押した時: サンプル映像
        case 100:
            player.load("sampleMovie.mov");
            file = Consts::mp4;
            player.play();
            break;

            //-------------   環境   ------------------
            // "Z"を押した時: release
        case 122:
            use = Consts::release;
            break;
            // "X"を押した時: preRelease
        case 120:
            use = Consts::preRelease;
            break;
            // "C"を押した時: debug
        case 99:
            use = Consts::debug;
            break;

            //-------------------------------
            // "-"を押した時: 終了
        case 59:
            file = Consts::none;
            player.stop();
            break;
            // 上記以外のボタンを押した時
        default:
            file = Consts::none;
            break;
    }

    switch (file) {
        case Consts::png:
            imgDraw = true;
            mapDraw = false;
            break;
        case Consts::mov:
            imgDraw = false;
            mapDraw = true;
            break;
        case Consts::mp4:
            imgDraw = false;
            mapDraw = false;
            break;
        case Consts::none:
            imgDraw = false;
            mapDraw = false;
            break;
    }

    if ( imgDraw ) {
        inputOfImg.update();
        image = ofxCv::toCv( inputOfImg );
        // ウインドウのサイズに合わせ10×10にリサイズ
        resize(image.clone(), image, cv::Size(), float(ofGetWidth()/WIDTHCOUNT)/image.cols, float(ofGetHeight()/HEIGHTCOUNT)/image.rows);
        ofxCv::toOf(image, outputOfImg);
        outputOfImg.update();
    }

    if( mapDraw ){
        player_map.play();
    }else {
        player_map.stop();
    }
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
