# saliency-drive
顕著性マップを生成

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
```
