//
//  VROShaderProgram.h
//  ViroRenderer
//
//  Created by Raj Advani on 4/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROSHADERPROGRAM_H_
#define VROSHADERPROGRAM_H_

#include <stdlib.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include "VROLog.h"
#include "VROUniform.h"
#include "VROOpenGL.h"

enum class VROGeometrySourceSemantic;

/*
 Bit-mask indicating support for each shader attribute.
 */
enum class VROShaderMask {
    Tex = 1,
    Color = 2,
    Norm = 4,
    Tangent = 8,
    BoneIndex = 16,
    BoneWeight = 32
};

class VROShaderModifier;
class VRODriverOpenGL;

/*
 Represents a GLSL shader program. Shader programs are constructed with vertex
 code, fragment code, samplers, attributes, uniforms, and shader modifiers.
 
 Materials can share access to a single shader. In these cases, the materials can
 set their own values for the uniforms when it is their turn to render. Note though
 that the uniform objects themselves are not owned by these materials; they are
 owned by the shader.
 */
class VROShaderProgram {
public:

    /*
     Create a new shader program with the given source. This constructor assumes that the
     shader code is bundled with the application.
     */
    VROShaderProgram(std::string vertexShader, std::string fragmentShader,
                     const std::vector<std::string> &samplers,
                     const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                     const std::vector<VROGeometrySourceSemantic> attributes,
                     std::shared_ptr<VRODriverOpenGL> driver);
    
    uint32_t getShaderId() const {
        return _shaderId;
    }

    virtual ~VROShaderProgram();

    /*
     Get the VROUniform setter given a name or index. The name accessors are much slower
     as they iterate through each uniform.
     */
    int getUniformIndex(const std::string &name);
    VROUniform *getUniform(const std::string &name);
    VROUniform *getUniform(int index);

    /*
     Add a new uniform to this shader. Returns a pointer to the uniform. Note this shader
     will own the uniform (callers must not delete the pointer).
     */
    VROUniform *addUniform(VROShaderProperty type, int arraySize, const std::string &name);

    /*
     Hydration, for shaders, involves compiling and linking the shader program so it can be
     run by the GPU.
     */
    bool hydrate();
    void evict();
    bool isHydrated() const;

    /*
     Bind this shader program, or unbind any program. Returns false if the program was
     already bound.
     */
    bool bind();
    static void unbind();

    /*
     Get the vertex and fragment source code for this shader.
     */
    const std::string &getVertexSource() const;
    const std::string &getFragmentSource() const;

    int getNumUniforms() const {
        return (int) _uniforms.size();
    }
    const std::vector<VROUniform *> &getUniforms() const {
        return _uniforms;
    }

    const std::string &getName() const {
        return _shaderName;
    }
    
    GLuint getProgram() const {
        return _program;
    }
    
    bool hasLightingBlock() const {
        return _lightingBlockIndex != GL_INVALID_INDEX;
    }
    GLuint getLightingBlockIndex() const {
        return _lightingBlockIndex;
    }
    
    bool hasBonesBlock() const {
        return _bonesBlockIndex != GL_INVALID_INDEX;
    }
    GLuint getBonesBlockIndex() const {
        return _bonesBlockIndex;
    }

    bool hasParticlesVertexBlock() const {
        return _particlesVertexBlockIndex != GL_INVALID_INDEX;
    }

    GLuint getParticlesVertexBlockIndex() const {
        return _particlesVertexBlockIndex;
    }

    bool hasParticlesFragmentBlock() const {
        return _particlesFragmentBlockIndex != GL_INVALID_INDEX;
    }

    GLuint getParticlesFragmentBlockIndex() const {
        return _particlesFragmentBlockIndex;
    }

    const std::vector<std::shared_ptr<VROShaderModifier>> &getModifiers() const {
        return _modifiers;
    }
    bool hasModifier(std::shared_ptr<VROShaderModifier> modifier) {
        return std::find(_modifiers.begin(), _modifiers.end(), modifier) != _modifiers.end();
    }
    
protected:
    
    /*
     Bind attributes, standard uniforms, and uniform blocks. Attributes are bound prior
     to linking, and uniform blocks are bound after compilation. These can be overriden
     by subclasses. The default implementation (here) adds the attributes, uniform blocks,
     and uniforms for the standard 3D shader used by lighting models Constant, Lambert,
     Blinn, and Phong.
     */
    virtual void bindAttributes();
    virtual void bindUniformBlocks();
    virtual void addStandardUniforms();

private:
    
    uint32_t _shaderId;

    /*
     VROUniform for each uniform in the shader. The VROUniform holds both the location of the uniform
     and the proper glUniform function to invoke to set its value.
     */
    std::vector<VROUniform *> _uniforms;
    
    /*
     The uniform block indices used by this shader to refer to the lighting block and the 
     bones block, if supported.
     */
    GLuint _lightingBlockIndex;
    GLuint _bonesBlockIndex;

    /*
     The uniform block for particles to be used by this shader - one for the fragment and
     one for the vertex shader.
     */
    GLuint _particlesVertexBlockIndex;
    GLuint _particlesFragmentBlockIndex;

    /*
     The attributes supported by this shader, as defined by the VROShader enum above.
     */
    int _attributes;

    /*
     True if the uniforms used by this shader have changed and require a rebind.
     This is always true after a shader is compiled.
     */
    bool _uniformsNeedRebind;

    /*
     The name of the shader.
     */
    std::string _shaderName;

    /*
     The source code of the shader.
     */
    std::string _vertexSource;
    std::string _fragmentSource;

    /*
     Integer identifying the program.
     */
    GLuint _program;

    /*
     True if the shader failed to compile or link.
     */
    bool _failedToLink;

    /*
     List of the names of all samplers used by this shader.
     */
    std::vector<std::string> _samplers;
    
    /*
     The modifiers used on this shader.
     */
    std::vector<std::shared_ptr<VROShaderModifier>> _modifiers;

    /*
     Weak reference to the driver that created this program. The driver's lifecycle
     is tied to the parent EGL context, so we only delete GL objects if the driver
     is alive, to ensure we're deleting them under the correct context (e.g. to avoid
     accidentally deleting objects in a new context that were created in an older
     one).
     */
    std::weak_ptr<VRODriverOpenGL> _driver;

    /*
     Compile and link the shader. Returns true on success.
     */
    bool compileAndLink();

    /*
     Compile, link, and validate the shader at the given path. Type indicates fragment or vertex.
     */
    bool compileShader(GLuint *shader, GLenum type, const char *source);
    bool linkProgram(GLuint prog);
    bool validateProgram(GLuint prog);
    
    /*
     Loads the the uniforms used by the modifiers.
     */
    void addModifierUniforms();

    /*
     Set the location of each uniform in the uniformMap and each sampler in the samplers
     list. Requires an EGL context and requires that this shader is bound.
     */
    void findUniformLocations();
    
    /*
     Inflate the #include directives in the source. Loads the files referred to by the
     includes into the shader.
     */
    void inflateIncludes(std::string &source) const;
    
    /*
     Inflate the shader modifiers into the shader source.
     */
    void inflateVertexShaderModifiers(const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                                      std::string &source) const;
    void inflateFragmentShaderModifiers(const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                                      std::string &source) const;
    void inflateReplacements(const std::map<std::string, std::string> &replacements, std::string &source) const;
    void insertModifier(std::string modifierSource, std::string directive, std::string &source) const;

};

#endif /* VROSHADERPROGRAM_H_ */
