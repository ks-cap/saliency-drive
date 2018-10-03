# saliency-drive
Create saliency map from video data and camera image.

# Requirements
* C++
* ofxOpenCv(+conrtib) / ofxCv / ofxGui

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
```

```
Ptr<StaticSaliencySpectralResidual> saliencyAlgorithm_SPECTRAL_RESIDUAL = StaticSaliencySpectralResidual::create();
```


#### ofApp.cpp
```
saliencyAlgorithm_SPECTRAL_RESIDUAL->computeSaliency( mat_gray.clone(), saliencyMap );
normalize( saliencyMap.clone(), saliencyMap_norm, 0.0, 255.0, NORM_MINMAX );
saliencyMap_norm.convertTo( saliencyMap_conv, CV_8UC3 );
```

# Demo
![demo](https://raw.github.com/wiki/Ken-S-0114/saliency-drive/images/saliency.mov.gif)
