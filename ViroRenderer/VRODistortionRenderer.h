//
//  VRODistortionRenderer.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/23/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VRODistortionRenderer_h
#define VRODistortionRenderer_h

#include <stdio.h>
#include <MetalKit/MetalKit.h>
#include "VRORenderTarget.h"
#include <memory>

enum class VROEyeType;
class VRODistortion;
class VRODistortionMesh;
class VROFieldOfView;
class VRODevice;
class VROViewport;
class VRORenderContextMetal;
class VROEye;
class VROConcurrentBuffer;

@class VROView;

/*
 Viewport specified in tangents, i.e. the tangent of the eye's
 field of view angle in each direction.
 */
class VROEyeViewport {
    
public:
    float x;
    float y;
    float width;
    float height;
    float eyeX;
    float eyeY;
    
    VROEyeViewport() :
        x(0), y(0), width(0), height(0), eyeX(0), eyeY(0)
    {}
    VROEyeViewport(const VROFieldOfView &eyeFOV, float xOffset);
    
};

class VRODistortionRenderer {
    
public:
    
    VRODistortionRenderer(std::shared_ptr<VRODevice> device);
    ~VRODistortionRenderer();
    
    /*
     Update the render-texture and the distortion mesh using hte latest FOV and viewport
     parameters.
     */
    void updateDistortion(id <MTLDevice> gpu, id <MTLLibrary> library, VROView *view);
    
    /*
     Binds the eye texture (the texture to which we will render both eyes) as the 
     render target. Following this call the left and right eye should be rendered.
     */
    std::shared_ptr<VRORenderTarget> bindEyeRenderTarget(id <MTLCommandBuffer> commandBuffer);
    
    /*
     Renders the left half of the eye texture to the left distortion mesh, and the right
     half of the eye texture to the right distortion mesh. Renders into the provided 
     encoder (typically the screen).
     */
    void renderEyesToScreen(id <MTLRenderCommandEncoder> screenEncoder, int frame);
    
    bool isChromaticAberrationEnabled() {
        return _chromaticAberrationCorrectionEnabled;
    }
    void setChromaticAberrationEnabled(bool enabled) {
        _chromaticAberrationCorrectionEnabled = enabled;
    }
    
    bool isVignetteEnabled() {
        return _vignetteEnabled;
    }
    void setVignetteEnabled(bool enabled) {
        _vignetteEnabled = enabled;
        _fovsChanged = true;
    }
    
    void updateViewports(VROEye *leftEye, VROEye *rightEye);
    
    void fovDidChange(VROEye *leftEye,
                      VROEye *rightEye,
                      float virtualEyeToScreenDistance);
    
private:
    
    id <MTLRenderPipelineState> _pipelineState;
    id <MTLRenderPipelineState> _aberrationPipelineState;
    id <MTLDepthStencilState> _depthState;
    
    /*
     Buffer containing the uniforms needed for the distortion pass.
     */
    VROConcurrentBuffer *_uniformsBuffer;
    
    /*
     The texture onto which we render both eyes. Eyes are rendered onto the
     MSAA texture and resolved onto _texture.
     */
    id <MTLTexture> _msaaTexture, _texture;
    
    float _resolutionScale;
    bool _chromaticAberrationCorrectionEnabled;
    bool _vignetteEnabled;
    
    /*
     The meshes used for barrel distortion for the left and right eye.
     */
    VRODistortionMesh *_leftEyeDistortionMesh;
    VRODistortionMesh *_rightEyeDistortionMesh;
    
    std::shared_ptr<VRODevice> _device;
    VROEyeViewport _leftEyeViewport;
    VROEyeViewport _rightEyeViewport;
    
    bool _fovsChanged;
    bool _drawingFrame;
    
    /*
     The number of pixels per tan(), used to convert an EyeViewport into an actual
     pixel-based viewport.
     */
    float _xPxPerTanAngle;
    float _yPxPerTanAngle;
    float _metersPerTanAngle;
    
    /*
     Update the pipeline used to render the distortion pass.
     */
    void updateDistortionPassPipeline(id <MTLDevice> gpu, id <MTLLibrary> library, MTKView *view);
    
    VRODistortionMesh *createDistortionMesh(const VROEyeViewport &eyeViewport,
                                            float textureWidthTanAngle,
                                            float textureHeightTanAngle,
                                            float xEyeOffsetTanAngleScreen,
                                            float yEyeOffsetTanAngleScreen,
                                            id <MTLDevice> gpu);
    
    void renderDistortionMesh(const VRODistortionMesh &mesh,
                              id <MTLTexture> texture,
                              id <MTLRenderCommandEncoder> renderEncoder,
                              VROEyeType eye,
                              int frame);
    
    float computeDistortionScale(const VRODistortion &distortion,
                                 float screenWidthM,
                                 float interpupillaryDistanceM);

};

#endif /* VRODistortionRenderer_h */
