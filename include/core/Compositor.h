#pragma once

#include <glad/gl.h>
#include "core/LayerStack.h"

namespace core {

class Compositor {
public:
    Compositor();
    ~Compositor();

    // Prevent copy
    Compositor(const Compositor&) = delete;
    Compositor& operator=(const Compositor&) = delete;

    // Initialize shader program and geometry
    bool Init();

    // Composite all layers in LayerStack onto a target FBO texture
    void Composite(const LayerStack& stack, GLuint targetFboId, int width, int height);

private:
    void CompileShaders();
    void SetupQuadGeometry();
    void Cleanup();

    GLuint m_ShaderProgram;
    GLuint m_QuadVAO;
    GLuint m_QuadVBO;

    // Shader Uniform Locations
    GLint m_LocBackTex;
    GLint m_LocFrontTex;
    GLint m_LocOpacity;
    GLint m_LocBlendMode;

    // Viewport rendering resources (RAII: non-static class members)
    GLuint m_EmptyTexture = 0;
    GLuint m_PingPongTextures[2] = {0, 0};
    GLuint m_PingPongFBOs[2] = {0, 0};
    int m_CachedWidth = 0;
    int m_CachedHeight = 0;
};

} // namespace core
