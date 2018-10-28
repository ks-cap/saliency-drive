#include "ofApp.h"

// UIの位置を変更する画素値の条件（白い箇所:顕著性が高い:255, 黒い箇所:顕著性が低い:0）
#define SALIENCY_IMG 826200   // 12960回 * 63.75(255/4)

// 顔矩形の係数
#define FACE_RANGE 1.2
#define SALIENCY_RANGE 2

// フレームの分割
#define WIDTHCOUNT 8
#define HEIGHTCOUNT 5

//--------------------------------------------------------------
void ofApp::setup(){

    // 背景色: White
    ofBackground(255, 255, 255);
    // 1回目と判定
    firstFrameCheck = true;
    // UIを出した箇所が次のフレームで一定数値以下であればUIを動かさない
    algorithmCheck = true;
    // 表示しているUIが画像か動画（地図）か
    imgDraw = false;

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

    //---------------------   Camera   -----------------------------
    //          vidGrabber.update();
    //
    //          if( vidGrabber.isFrameNew() ){
    //            ofPixels & pixels = vidGrabber.getPixels();

    //            saliencyAlgorithm( ofxCv::toCv( pixels ).clone() );
    //--------------------------------------------------------------


    // 動画の場合
    player.update();

    if(player.isFrameNew()){
        // Mat変換
        frame = ofxCv::toCv(player).clone();
        // 画質を半分に下げる
        cv::pyrDown(frame.clone(), hogFrame);

        hogData = hog.multiUpdate(hogFrame);
        // 顔認識
        hogGetRect();
        // 顕著性マップ作成
        saliencyAlgorithm(hogFrame);

        // 最小と最大の要素値とそれらの位置を求める
        //        minMaxLoc(saliencyMap, &minMax.min_val, &minMax.max_val, &minMax.min_loc, &minMax.max_loc, cv::Mat());
        //        ofLog()<<"minMaxLoc.min_val: "<<minMax.min_val;
        //        ofLog()<<"minMaxLoc.min_loc: "<<minMax.min_loc;

        // マスク処理
        saliencyMask();
        // updateが2回目以降もしくはボタンを押されてupdateが2回目以降に呼ばれた場合, if文の中に入る
        saliencyCheck(firstFrameCheck);

        // 5*3のうちの画素最小値の場所を取得
        algorithmMinPixels(algorithmCheck);

        // 画素値の反転（現状 : 0:黒:顕著性が低い, 255:白:顕著性が高い）
        //        for(int y = 0; y < result.cols; ++y){
        //            for(int x = 0; x < result.rows; ++x){
        //                result.at<uchar>( x, y ) = 255 - (int)result.at<uchar>(x, y);
        //                //        ofLog()<<"(int)saliencyMap.at<uchar>("<<x<<","<<y<< ") : "<<(int)saliencyMap.at<uchar>( x, y );
        //            }
        //        }

        // 疑似カラー（カラーマップ）変換 :（0:赤:顕著性が高い, 255:青:顕著性が低い）
        applyColorMap(result.clone(), saliencyMap_color, cv::COLORMAP_JET);

        // 前・後景が合成されたMat画像を作成 (後景:元の動画に対して, 前景:UIを貼り付け)
        if (imgDraw == true) {
//--------------------------------------------------------------
//            CV_Assert((minPlace.widthMin >= 0) && (minPlace.widthMin + ofGetWidth()/WIDTHCOUNT <= saliencyMap.cols));
//            CV_Assert((minPlace.heightMin >= 0) && (minPlace.heightMin + ofGetHeight()/HEIGHTCOUNT <= saliencyMap.rows));
//
//            frame_copy = frame.clone();
//            cv::Mat frame_img = frame_copy(cv::Rect(minPlace.widthMin, minPlace.heightMin, ofGetWidth()/WIDTHCOUNT, ofGetHeight()/HEIGHTCOUNT));
//            image.copyTo(frame_img);
//            ofxCv::toOf(frame_copy.clone(), outputOfImg);
//--------------------------------------------------------------
            // UI画像を上に再描画
            ofxCv::toOf(image.clone(), outputOfImg);
            outputOfImg.update();
//--------------------------------------------------------------
        }
        ofLog()<<"----------------------------------------------------\n";
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
            // Debug用
//            outputOfImg.draw(0, 0, ofGetWidth(),ofGetHeight());
            ofxCv::drawMat(frame_copy, 0, 0, ofGetWidth(),ofGetHeight());
            ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 1200, 20);
            break;

        case Consts::debug:
            player.draw(0, 0, ofGetWidth()/3, ofGetHeight()/2);
            // 顔検知出力(Hog)
            ofxCv::drawMat(hogFrame, ofGetWidth()/3, 0, ofGetWidth()/3, ofGetHeight()/2);
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

//--------------------------------------------------------------
    // UI画像を上に再描画
    if (imgDraw){
        outputOfImg.draw(minPlace.widthMin, minPlace.heightMin);
    }
//--------------------------------------------------------------
    // データの初期化
    if(!face.empty()) { face.clear(); }
    if(!saliencyRange.empty()) { saliencyRange.clear(); }
    if(!saliencyRect.empty()) { saliencyRect.clear(); }
    if(!pixelsList.empty()) { pixelsList.clear(); }
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
    cv::normalize(saliencyMap_SPECTRAL_RESIDUAL.clone(), saliencyMap_norm, 0.0, 255.0, cv::NORM_MINMAX);
    // Matの型（ビット深度）を変換する
    saliencyMap_norm.convertTo(saliencyMap, CV_8UC3);

    return saliencyMap;
}

//--------------------------------------------------------------
bool ofApp::saliencyCheck(bool checkUI){
    if (!checkUI){
        // 今は firstFrameCheck で条件を発火させ, ボタンを押した直後は入らないようにしている

        // 前回の顕著性マップで顕著性が低かったピクセルのうちの一つ
        cv::Rect roi(minPlace.widthMin, minPlace.heightMin, saliencyMap.cols / WIDTHCOUNT, saliencyMap.rows / HEIGHTCOUNT);
        cv::Mat saliency_roi = saliencyMap(roi);
        int pixels = 0;
        // WIDTHCOUNT*HEIGHTCOUNTのうちの一つの画素値(count: 12960回 * 42.5(255/6))
        int i = 0;
        for(int y = 0; y < saliency_roi.cols; ++y){
            for(int x = 0; x < saliency_roi.rows; ++x){
                pixels += (int)saliency_roi.at<uchar>(x, y);
                i++;
            }
        }
//        ofLogNotice()<<"i :"<<i;
        // UIを出した箇所が次のフレームで一定数値以下であればUIを動かさないフラグを設定
        algorithmCheck = pixels < SALIENCY_IMG ? false : true ;
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
                if ((heightCount == 0 && widthCount == 0) || pixel > minPixels) {
                    if (pixel != 0) {
                        minPixels = pixel;
                        minPlace.widthMin = width;
                        minPlace.heightMin = height;
                        ofLogNotice()<<"pixelsMin["<<width<<", "<<height<<"]: "<<minPixels;
                    }
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
        cv::Rect rect = data.rect;
        ofLog()<<"rect"<<"["<<data.id<<"].x: "<< rect.x;
        ofLog()<<"rect"<<"["<<data.id<<"].y: "<< rect.y;
        ofLog()<<"rect"<<"["<<data.id<<"].width: "<< rect.width;
        ofLog()<<"rect"<<"["<<data.id<<"].height: "<< rect.height;

        ofRectangle rectangle = ofxCv::toOf(rect);

        HogTool::Face f;
        f.center = rectangle.getCenter();
        f.x = rectangle.getCenter().x - (rectangle.width * FACE_RANGE / 2);
        f.y = rectangle.getCenter().y - (rectangle.height * FACE_RANGE / 2);;
        f.width = rectangle.getWidth() * FACE_RANGE;
        f.height = rectangle.getHeight() * FACE_RANGE;
        face.push_back(f);

        // 顔の矩形
        cv::rectangle(hogFrame, cv::Rect(f.x, f.y, f.width, f.height), cv::Scalar(255, 0, 0), 2, CV_AA);

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

        // 顔周辺の矩形
        cv::rectangle(hogFrame, _s, cv::Scalar(0, 0, 255), 2, CV_AA);

        ofLog()<<"saliencyRect"<<"["<<data.id<<"].x: "<< saliencyRect[data.id].x;
        ofLog()<<"saliencyRect"<<"["<<data.id<<"].y: "<< saliencyRect[data.id].y;
        ofLog()<<"saliencyRect"<<"["<<data.id<<"].width: "<< saliencyRect[data.id].width;
        ofLog()<<"saliencyRect"<<"["<<data.id<<"].height: "<< saliencyRect[data.id].height;
    }
}

//--------------------------------------------------------------
void ofApp::saliencyMask(){
    // saliency適応範囲以外をマスク
    mask = cv::Mat();
    result = cv::Mat();

    cv::Mat mask_zero, mask_ones;
    // 背景黒
    mask_zero = cv::Mat::zeros(saliencyMap.rows, saliencyMap.cols, CV_8UC1);
    // 顔付近の矩形（白）
    for (int i = 0; i < (int)saliencyRect.size(); i++) {
        cv::rectangle(mask_zero, saliencyRect[i], cv::Scalar::all(255), -1, CV_8UC3);
    }

    // 背景白
    mask_ones = cv::Mat::ones(saliencyMap.rows, saliencyMap.cols, CV_8UC1)*255;
    // 顔の矩形（黒）
    for (int m = 0; m < (int)face.size(); m++) {
        ofLog()<<"face"<<"["<<m<<"].x: "<< face[m].x;
        ofLog()<<"face"<<"["<<m<<"].y: "<< face[m].y;
        ofLog()<<"face"<<"["<<m<<"].width: "<< face[m].width;
        ofLog()<<"face"<<"["<<m<<"].height: "<< face[m].height;
        cv::rectangle(mask_ones, cv::Rect((int)face[m].x, (int)face[m].y, (int)face[m].width, (int)face[m].height), cv::Scalar::all(0), -1, CV_8UC3);
    }
    mask_zero.copyTo(mask, mask_ones.clone());

    saliencyMap.copyTo(result, mask.clone());
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    printf("keyPressed: %d\n", key);
    // 条件を発火させ, ボタンを押した直後は saliencyCheck 関数に入らないようにしている
    firstFrameCheck = true;
    
    switch (key) {
            //-------------   UI   ------------------
            // "1"を押した時 単純形状表示
        case 49:
            inputOfImg.load("roadSign.png");
            inputOfImg.update();
            image = ofxCv::toCv(inputOfImg);
            // ウインドウのサイズに合わせ10×10にリサイズ: UI画像を上に再描画する場合のみ
            resize(image.clone(), image, cv::Size(), float(ofGetWidth()/WIDTHCOUNT)/image.cols, float(ofGetHeight()/HEIGHTCOUNT)/image.rows);
            imgDraw = true;
            break;
            //-------------   動画データ   ------------------
            // "A"を押した時: 昼のドライブ映像
        case 97:
            player.load("sampleMovie.mov");
            imgDraw = false;
            player.play();
            break;

            //-------------   環境   ------------------
            // "Z"を押した時: release
        case 122:
            use = Consts::release;
            break;
            // "X"を押した時: preRelease
        case 120:
            if (imgDraw){
                use = Consts::preRelease;
            }
            break;
            // "C"を押した時: debug
        case 99:
            use = Consts::debug;
            break;

            //-------------------------------
            // 上記以外のボタンを押した時
        default:
            imgDraw = false;
            player.stop();
            break;
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
