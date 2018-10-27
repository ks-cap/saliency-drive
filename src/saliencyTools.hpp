//
//  saliencyTools.hpp
//  saliency-drive
//
//  Created by 佐藤賢 on 2018/10/25.
//

#ifndef saliencyTools_hpp
#define saliencyTools_hpp

#include "ofMain.h"
#include "ofxOpenCv.h"

class SaliencyTool {
public:
    // 最小と最大の要素値とそれらの位置
//    struct MinMax {
//        cv::Point min_loc, max_loc;
//        double min_val, max_val;
//    };
    // 最小の位置
    struct MinPlace {
        int widthMin;
        int heightMin;
    };

};

#endif /* saliencyTools_hpp */
