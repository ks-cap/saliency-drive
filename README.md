# saliency-drive
顕著性マップを生成

# Dependency
* C++
* ofxOpenCv
* ofxCv
* ofxGui

# Usage
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

```swift
Ptr<StaticSaliencySpectralResidual> saliencyAlgorithm_SPECTRAL_RESIDUAL = StaticSaliencySpectralResidual::create();
```

```
saliencyAlgorithm_SPECTRAL_RESIDUAL->computeSaliency( mat_gray.clone(), saliencyMap );
```
