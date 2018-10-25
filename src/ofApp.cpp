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

    // ofxSyphon
    //    mainOutputSyphonServer.setName("Screen Output");
    //
    //    mClient.setup();
    //    mClient.set("","Simple Server");
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
        cv::pyrDown(frame.clone(), frame);

        hogData = hog.multiUpdate(frame);
        
        for (auto data : hogData) {
            cv::rectangle(frame, data.rect, cv::Scalar(255, 0, 0), 2, CV_AA);

            ofLog()<<"rect"<<"["<<data.id<<"].x: "<< data.rect.x;
            ofLog()<<"rect"<<"["<<data.id<<"].y: "<< data.rect.y;
            ofLog()<<"rect"<<"["<<data.id<<"].width: "<< data.rect.width;
            ofLog()<<"rect"<<"["<<data.id<<"].height: "<< data.rect.height;

            // 顔の矩形
            ofRectangle rectangle = ofxCv::toOf(data.rect);

            HogTool::Face f;
            f.center += rectangle.getCenter();
            f.width = rectangle.getWidth();
            f.height = rectangle.getHeight();
            face.push_back(f);

            //            ofLog()<<"face.center: "<< face[data.id].center;
            //            ofLog()<<"face.width: "<< face[data.id].width;
            //            ofLog()<<"face.height: "<< face[data.id].height;

            HogTool::SaliencyRange s;
            s.center = face[data.id].center;
            s.width = face[data.id].width * SALIENCY_RANGE;
            s.height = face[data.id].height * SALIENCY_RANGE;
            saliencyRange.push_back(s);
            //
            //            ofLog()<<"saliencyRange.center: "<< saliencyRange[data.id].center;
            //            ofLog()<<"saliencyRange.width: "<< saliencyRange[data.id].width;
            //            ofLog()<<"saliencyRange.height: "<< saliencyRange[data.id].height;
            // printf("---------------------------------\n");
            cv::Rect _s;
            _s.x = saliencyRange[data.id].center.x - (saliencyRange[data.id].width / 2);
            _s.y = saliencyRange[data.id].center.y - (saliencyRange[data.id].height / 2);
            _s.height = saliencyRange[data.id].height;
            _s.width = saliencyRange[data.id].width;
            saliencyRect.push_back(_s);

            cv::rectangle(frame, _s, cv::Scalar(0, 0, 255), 2, CV_AA);

            ofLog()<<"saliencyRect"<<"["<<data.id<<"].x: "<< saliencyRect[data.id].x;
            ofLog()<<"saliencyRect"<<"["<<data.id<<"].y: "<< saliencyRect[data.id].y;
            ofLog()<<"saliencyRect"<<"["<<data.id<<"].width: "<< saliencyRect[data.id].width;
            ofLog()<<"saliencyRect"<<"["<<data.id<<"].height: "<< saliencyRect[data.id].height;
            //            printf("---------------------------------\n");
        }

        saliencyAlgorithm(frame);

        //---------------------   Camera   -----------------------------
        //          vidGrabber.update();
        //
        //          if( vidGrabber.isFrameNew() ){
        //            ofPixels & pixels = vidGrabber.getPixels();

        //            saliencyAlgorithm( ofxCv::toCv( pixels ).clone() );
        //--------------------------------------------------------------

        // 最小と最大の要素値とそれらの位置を求める
        minMaxLoc(saliencyMap_conv, &minMax.min_val, &minMax.max_val, &minMax.min_loc, &minMax.max_loc, cv::Mat());
        ofLog()<<"minMaxLoc.min_val: "<<minMax.min_val;
        ofLog()<<"minMaxLoc.min_loc: "<<minMax.min_loc;

        // updateが2回目以降もしくはボタンを押されてupdateが2回目以降に呼ばれた場合, if文の中に入る
        saliencyCheck(firstFrameCheck);

        // 10*10のうちの画素最小値の場所を取得
        algorithmMinPixels(algorithmCheck);

        // 画素値の反転（現状 : 0:黒:顕著性が低い, 255:白:顕著性が高い）
        for( int y = 0; y < saliencyMap_conv.cols; ++y ){
            for( int x = 0; x < saliencyMap_conv.rows; ++x ){
                saliencyMap_conv.at<uchar>( x, y ) = 255 - (int)saliencyMap_conv.at<uchar>( x, y );
                //        ofLog()<<"(int)saliencyMap_conv.at<uchar>("<<x<<","<<y<< ") : "<<(int)saliencyMap_conv.at<uchar>( x, y );
            }
        }

        // saliency適応範囲以外をマスク
        result = cv::Mat();
        mask = cv::Mat::zeros(saliencyMap_conv.rows, saliencyMap_conv.cols, CV_8UC1);
        
        ofLog()<<"saliencyMap_conv.channels: "<< saliencyMap_conv.channels();
        ofLog()<<"mask.channels: "<< mask.channels();

        for (int i = 0; i < (int)saliencyRect.size(); i++ ) {
            cv::rectangle(mask, saliencyRect[i], cv::Scalar(255, 255, 255), -1, CV_8UC3);
        }

        saliencyMap_conv.copyTo(result, mask.clone());
        // 疑似カラー（カラーマップ）変換 :（0:赤:顕著性が高い, 255:青:顕著性が低い）
        applyColorMap( result.clone(), saliencyMap_color, cv::COLORMAP_JET );

        printf("----------------------------------------------------");
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
        case release:
            player.draw(0, 0, ofGetWidth(),ofGetHeight());
            break;

        case preRelease:
            // 顕著性マップ(SPECTRAL_RESIDUAL) + saliency適応範囲を出力: Debug用
            ofxCv::drawMat(saliencyMap_color, 0, 0, ofGetWidth(),ofGetHeight());
            break;

        case debug:
            player.draw(0, 0, ofGetWidth()/2, ofGetHeight()/2);
            // 顔検知出力(Hog)
            ofxCv::drawMat(frame, 640, 0, ofGetWidth()/2, ofGetHeight()/2);
            // 顕著性マップ(SPECTRAL_RESIDUAL)を出力
            ofxCv::drawMat(saliencyMap_conv, 0, 360, ofGetWidth()/2, ofGetHeight()/2);
            // 顕著性マップ(SPECTRAL_RESIDUAL) + saliency適応範囲を出力
            ofxCv::drawMat(saliencyMap_color, 640, 360, ofGetWidth()/2, ofGetHeight()/2);

            // Label
            ofDrawBitmapStringHighlight("original", 20,20);
            ofDrawBitmapStringHighlight("faceTrack", 660,20);
            ofDrawBitmapStringHighlight("saliencyMap", 20,380);
            ofDrawBitmapStringHighlight("saliencyRange", 660,380);
            // FPS表示
            ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 1200, 20);
            break;
    }

    // UI画像
    if ( imgDraw ){ outputOfImg.draw( minPlace.widthMin, minPlace.heightMin ); }
    if ( mapDraw ){ player_map.draw( minPlace.widthMin, minPlace.heightMin, ofGetWidth()/5, ofGetHeight()/5); }

    // データの初期化
    if(!face.empty()) { face.clear(); }
    if(!saliencyRange.empty()) { saliencyRange.clear(); }
    if(!saliencyRect.empty()) { saliencyRect.clear(); }

}

//--------------------------------------------------------------
cv::Mat ofApp::saliencyAlgorithm(cv::Mat mat){
    cv::Mat mat_gray, mat_gaus;
    // 白黒加工
    cv::cvtColor(mat.clone(), mat_gray, cv::COLOR_BGR2GRAY);
    // ぼかし
    cv::GaussianBlur(mat_gray.clone(), mat_gaus, cv::Size(5, 5), 0, 0);
    // SPECTRAL_RESIDUAL(顕著性マップを求めるアルゴリズム)
    cv::Ptr<cv::saliency::Saliency> saliencyAlgorithm;
    saliencyAlgorithm = cv::saliency::StaticSaliencySpectralResidual::create();
    saliencyAlgorithm->computeSaliency( mat_gaus.clone(), saliencyMap );

    // アルファチャンネルの正規化を行う
    cv::normalize( saliencyMap.clone(), saliencyMap_norm, 0.0, 255.0, cv::NORM_MINMAX );
    // Matの型（ビット深度）を変換する
    saliencyMap_norm.convertTo( saliencyMap_conv, CV_8UC3 );

    return saliencyMap_conv;
}

//--------------------------------------------------------------
bool ofApp::saliencyCheck(bool checkUI){
    if ( !checkUI ){
        // 省略記載：（注意）falseだからといってmapDraw = trueとは限らない
        // 今は firstFrameCheck で条件を発火させ, ボタンを押した直後は入らないようにしている
        int count = imgDraw ? 10 : 5;
        // 前回の顕著性マップで顕著性が低かったピクセルのうちの一つ
        cv::Rect roi(minPlace.widthMin, minPlace.heightMin, saliencyMap_conv.cols / count, saliencyMap_conv.rows / count);
        cv::Mat saliency_roi = saliencyMap_conv(roi);
        int pixels = 0;
        // 10*10のうちの一つの画素値(count: 9216回 * 42.5(255/6))
        // 5*5のうちの一つの画素値(count: 36864回　* 42.5(255/6))
        for( int y = 0; y < saliency_roi.cols; ++y ){
            for( int x = 0; x < saliency_roi.rows; ++x ){
                pixels += (int)saliency_roi.at<uchar>( x, y );
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
                //                ofLog()<<"saliencyMap_convs/5 : "<< saliencyMap_conv.cols / WIDTHCOUNT;
                //                ofLog()<<"saliencyMap_rows/3 : "<< saliencyMap_conv.rows / HEIGHTCOUNT;
                cv::Rect roi(width, height, saliencyMap_conv.cols / WIDTHCOUNT, saliencyMap_conv.rows / HEIGHTCOUNT);
                cv::Mat saliency_roi = saliencyMap_conv(roi);

                // 大きな範囲のうちの一箇所の画素値の合計
                int pixel = 0;
                for (int y = 0; y < saliency_roi.cols; ++y) {
                    for(int x = 0; x < saliency_roi.rows; ++x){
                        pixel += (int)saliency_roi.at<uchar>(x, y);
                    }
                }

                //                pixels.push_back(pixel);
                // 最小値の値とその場所を更新
                if ((heightCount == 0 && widthCount == 0) || pixel < minPixels) {
                    minPixels = pixel;
                    minPlace.widthMin = width;
                    minPlace.heightMin = height;
                }

                width += saliencyMap_conv.cols / WIDTHCOUNT;
            }
            height += saliencyMap_conv.rows / HEIGHTCOUNT;
        }
    }

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    printf("keyPressed: %d\n", key);
    // 条件を発火させ, ボタンを押した直後は saliencyCheck 関数に入らないようにしている
    firstFrameCheck = true;

    enum File file;
    
    switch (key) {
            //-------------   UI   ------------------
            // "1"を押した時 単純形状表示
        case 49:
            inputOfImg.load("circle.png");
            file = png;
            break;
            // "2"を押した時 道路の標識（速度制限）表示
        case 50:
            inputOfImg.load("roadSign_speed.png");
            file = png;
            break;
            // "3"を押した時 道路の標識（停止）表示
        case 51:
            inputOfImg.load("roadSign_stop.png");
            file = png;
            break;
            // "4"を押した時: メールのアイコン表示
        case 52:
            inputOfImg.load("icon_mail.png");
            file = png;
            break;
            // "5"を押した時: マップ表示
        case 53:
            inputOfImg.load("string.png");
            file = png;
            break;
            // "6"を押した時: マップ表示
        case 54:
            player_map.load("movie_map.mov");
            file = mov;
            break;
            // "7"を押した時 道路の標識（速度制限）表示: 半透明
        case 55:
            inputOfImg.load("roadSign_speed2.png");
            file = png;
            break;
            //-------------   動画データ   ------------------
            // "A"を押した時: 昼のドライブ映像
        case 97:
            player.load("driver_daytime.mp4");
            file = mp4;
            player.play();
            break;
            // "S"を押した時: 夜のドライブ映像
        case 115:
            player.load("driver_night.mp4");
            file = mp4;
            player.play();
            break;
            // "D"を押した時: サンプル映像
        case 100:
            player.load("sampleMovie.mov");
            file = mp4;
            player.play();
            break;

            //-------------   環境   ------------------
            // "Z"を押した時: release
        case 122:
            use = release;
            break;
            // "X"を押した時: preRelease
        case 120:
            use = preRelease;
            break;
            // "C"を押した時: debug
        case 99:
            use = debug;
            break;

            //-------------------------------
            // "-"を押した時: 終了
        case 59:
            file = none;
            player.stop();
            break;
            // 上記以外のボタンを押した時
        default:
            file = none;
            break;
    }

    switch (file) {
        case png:
            imgDraw = true;
            mapDraw = false;
            break;
        case mov:
            imgDraw = false;
            mapDraw = true;
            break;
        case mp4:
            imgDraw = false;
            mapDraw = false;
            break;
        case none:
            imgDraw = false;
            mapDraw = false;
            break;
    }

    if ( imgDraw ) {
        inputOfImg.update();
        image = ofxCv::toCv( inputOfImg );
        // ウインドウのサイズに合わせ10×10にリサイズ
        resize( image, image, cv::Size(), float(ofGetWidth()/10)/image.cols, float(ofGetHeight()/10)/image.rows );
        ofxCv::toOf( image, outputOfImg );
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
