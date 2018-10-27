# saliency-drive
Create saliency map from video data and camera image.

# Requirements
* C++, C
* ofxOpenCv(+conrtib) / ofxCv / ofxDlib

# Usage
#### ofApp.h
```
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"

#include "saliencySpecializedClasses.hpp"

#include "opencv2/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "iostream"

#include "ofxDLib.h"
#include "hogTools.hpp"
#include "saliencyTools.hpp"
#include "consts.hpp"
```


# Demo
![demo](https://raw.github.com/wiki/Ken-S-0114/saliency-drive/images/saliency.gif)
