//
//  VROBodyRecognitionTest.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/14/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//
#include "VROBodyRecognitionTest.h"
#include "VROTestUtil.h"
#include "VROSphere.h"
#include "VROPoseFilterMovingAverage.h"
#include "VROPoseFilterLowPass.h"

#if VRO_PLATFORM_IOS
#include "VROBodyTrackerYolo.h"
#include "VROBodyTrackeriOS.h"
#include "VRODriverOpenGLiOS.h"
#include "VROVideoTexture.h"
#endif

static const bool kTestVideoTracking = false;

VROBodyRecognitionTest::VROBodyRecognitionTest() :
VRORendererTest(VRORendererTestType::BodyRecognition) {
    
}

VROBodyRecognitionTest::~VROBodyRecognitionTest() {
    
}

void VROBodyRecognitionTest::build(std::shared_ptr<VRORenderer> renderer,
                                   std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                   std::shared_ptr<VRODriver> driver) {
    _renderer = renderer;
    _sceneController = std::make_shared<VROARSceneController>();
    _sceneController->setDelegate(shared_from_this());
    
    _arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
    _arScene->initDeclarativeSession();
    
#if VRO_PLATFORM_IOS
    VROViewAR *view = (VROViewAR *) std::dynamic_pointer_cast<VRODriverOpenGLiOS>(driver)->getView();
    _bodyTracker = std::make_shared<VROBodyTrackeriOS>();
    _skeletonRenderer = std::make_shared<VROSkeletonRenderer>(view, _bodyTracker);
    
    _bodyTracker->setDelegate(shared_from_this());
    _bodyTracker->initBodyTracking(view.cameraPosition, driver);
    _bodyTracker->startBodyTracking();
    
    if (kTestVideoTracking) {
        std::shared_ptr<VROARSession> arSession = [view getARSession];
        std::shared_ptr<VROTexture> backgroundTexture = arSession->getCameraBackgroundTexture();
        std::shared_ptr<VROVideoTexture> vidTexture = std::dynamic_pointer_cast<VROVideoTexture>(backgroundTexture);
        vidTexture->loadVideo(VROTestUtil::getURLForResource("Yoga", "mp4"), nullptr, driver);
        vidTexture->setLoop(true);
        vidTexture->play();
    }
#endif

    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.6, 0.6, 0.6 });
    _arScene->getRootNode()->addLight(ambient);
}

void VROBodyRecognitionTest::onBodyJointsFound(const VROPoseFrame &joints) {
#if VRO_PLATFORM_IOS
    _skeletonRenderer->onBodyJointsFound(joints);
#endif
}
