//
//  VROMaterialSubstrateMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/30/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMaterialSubstrateMetal.h"
#include "VROSharedStructures.h"
#include "VROMetalUtils.h"
#include "VRORenderContextMetal.h"
#include "VROMatrix4f.h"
#include "VROLight.h"

VROMaterialSubstrateMetal::VROMaterialSubstrateMetal(VROMaterial &material,
                                                     const VRORenderContextMetal &context) {
    
    _lightingModel = material.getLightingModel();

    id <MTLDevice> device = context.getDevice();
    id <MTLLibrary> library = context.getLibrary();
    
    _lightingUniformsBuffer = [device newBufferWithLength:sizeof(VROSceneLightingUniforms) options:0];
    _lightingUniformsBuffer.label = @"VROSceneLightingUniformBuffer";
    
    _materialUniformsBuffer = [device newBufferWithLength:sizeof(VROMaterialUniforms) options:0];
    _materialUniformsBuffer.label = @"VROMaterialUniformBuffer";
    
    VROMaterialUniforms *uniforms = (VROMaterialUniforms *)[_materialUniformsBuffer contents];
    uniforms->diffuse_surface_color = toVectorFloat4(material.getDiffuse().getContentsColor());
    uniforms->shininess = material.getShininess();
    
    switch (material.getLightingModel()) {
        case VROLightingModel::Constant:
            loadConstantLighting(material, library, device, context);
            break;
            
        case VROLightingModel::Blinn:
            loadBlinnLighting(material, library, device, context);
            break;
            
        case VROLightingModel::Lambert:
            loadLambertLighting(material, library, device, context);
            break;
            
        case VROLightingModel::Phong:
            loadPhongLighting(material, library, device, context);
            break;
            
        default:
            break;
    }
}

VROMaterialSubstrateMetal::~VROMaterialSubstrateMetal() {
    
}

void VROMaterialSubstrateMetal::loadConstantLighting(VROMaterial &material,
                                                     id <MTLLibrary> library, id <MTLDevice> device,
                                                     const VRORenderContextMetal &context) {
    

    _vertexProgram   = [library newFunctionWithName:@"constant_lighting_vertex"];
    VROMaterialVisual &diffuse = material.getDiffuse();

    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        _fragmentProgram = [library newFunctionWithName:@"constant_lighting_fragment_c"];
    }
    else {
        _textures.push_back(((VROTextureSubstrateMetal *)diffuse.getContentsTexture()->getSubstrate(context))->getTexture());
        _fragmentProgram = [library newFunctionWithName:@"constant_lighting_fragment_t"];
    }
}

void VROMaterialSubstrateMetal::loadLambertLighting(VROMaterial &material,
                                                    id <MTLLibrary> library, id <MTLDevice> device,
                                                    const VRORenderContextMetal &context) {
    
    _vertexProgram   = [library newFunctionWithName:@"lambert_lighting_vertex"];
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        _fragmentProgram = [library newFunctionWithName:@"lambert_lighting_fragment_c"];
    }
    else {
        _textures.push_back(((VROTextureSubstrateMetal *)diffuse.getContentsTexture()->getSubstrate(context))->getTexture());
        _fragmentProgram = [library newFunctionWithName:@"lambert_lighting_fragment_t"];
    }
}

void VROMaterialSubstrateMetal::loadPhongLighting(VROMaterial &material,
                                                  id <MTLLibrary> library, id <MTLDevice> device,
                                                  const VRORenderContextMetal &context) {
    
    /*
     If there's no specular map, then we fall back to Lambert lighting.
     */
    VROMaterialVisual &specular = material.getSpecular();
    if (specular.getContentsType() != VROContentsType::Texture2D) {
        loadLambertLighting(material, library, device, context);
        return;
    }
    
    _vertexProgram   = [library newFunctionWithName:@"phong_lighting_vertex"];
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        _fragmentProgram = [library newFunctionWithName:@"phong_lighting_fragment_c"];
    }
    else {
        _textures.push_back(((VROTextureSubstrateMetal *)diffuse.getContentsTexture()->getSubstrate(context))->getTexture());
        _fragmentProgram = [library newFunctionWithName:@"phong_lighting_fragment_t"];
    }
    _textures.push_back(((VROTextureSubstrateMetal *)specular.getContentsTexture()->getSubstrate(context))->getTexture());
}

void VROMaterialSubstrateMetal::loadBlinnLighting(VROMaterial &material,
                                                  id <MTLLibrary> library, id <MTLDevice> device,
                                                  const VRORenderContextMetal &context) {
    
    /*
     If there's no specular map, then we fall back to Lambert lighting.
     */
    VROMaterialVisual &specular = material.getSpecular();
    if (specular.getContentsType() != VROContentsType::Texture2D) {
        loadLambertLighting(material, library, device, context);
        return;
    }
    
    _vertexProgram   = [library newFunctionWithName:@"blinn_lighting_vertex"];
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        _fragmentProgram = [library newFunctionWithName:@"blinn_lighting_fragment_c"];
    }
    else {
        _textures.push_back(((VROTextureSubstrateMetal *)diffuse.getContentsTexture()->getSubstrate(context))->getTexture());
        _fragmentProgram = [library newFunctionWithName:@"blinn_lighting_fragment_t"];
    }
    _textures.push_back(((VROTextureSubstrateMetal *)specular.getContentsTexture()->getSubstrate(context))->getTexture());
}

void VROMaterialSubstrateMetal::setLightingUniforms(const std::vector<std::shared_ptr<VROLight>> &lights) {
    VROSceneLightingUniforms *uniforms = (VROSceneLightingUniforms *)[_lightingUniformsBuffer contents];
    uniforms->num_lights = (int) lights.size();
    
    VROVector3f ambientLight;

    for (int i = 0; i < lights.size(); i++) {
        const std::shared_ptr<VROLight> &light = lights[i];
        
        VROLightUniforms &light_uniforms = uniforms->lights[i];
        light_uniforms.color = toVectorFloat3(light->getColor());
        light_uniforms.attenuation_start_distance = light->getAttenuationStartDistance();
        light_uniforms.attenuation_end_distance = light->getAttenuationEndDistance();
        light_uniforms.attenuation_falloff_exp = light->getAttenuationFalloffExponent();
        light_uniforms.spot_inner_angle = light->getSpotInnerAngle();
        light_uniforms.spot_outer_angle = light->getSpotOuterAngle();
        
        if (light->getType() == VROLightType::Ambient) {
            ambientLight += light->getColor();
        }
        
        if (light->getType() == VROLightType::Directional) {
            light_uniforms.position = toVectorFloat4(light->getDirection(), 0.0);
        }
        else {
            light_uniforms.position = toVectorFloat4(light->getTransformedPosition(), 1.0);
        }
    }
    
    uniforms->ambient_light_color = toVectorFloat3(ambientLight);
}