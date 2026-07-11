#include "core/Compositor.h"
#include <iostream>
#include <vector>
#include <algorithm>

namespace core {

// Vertex Shader: Renders a fullscreen quad using normalized device coordinates
static const char* s_VertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// Fragment Shader: Blends front texture onto back texture using selected blend mode and opacity
static const char* s_FragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D backTex;
uniform sampler2D frontTex;
uniform float opacity;
uniform int blendMode; // enum BlendMode: 0=Normal, 1=Multiply, 2=Screen, 3=Overlay, 4=SoftLight, 5=Difference

// Blend Mode formulas matching Photoshop behavior
vec3 BlendNormal(vec3 b, vec3 f) {
    return f;
}

vec3 BlendMultiply(vec3 b, vec3 f) {
    return b * f;
}

vec3 BlendScreen(vec3 b, vec3 f) {
    return b + f - b * f;
}

vec3 BlendOverlay(vec3 b, vec3 f) {
    return vec3(
        (b.r < 0.5) ? (2.0 * b.r * f.r) : (1.0 - 2.0 * (1.0 - b.r) * (1.0 - f.r)),
        (b.g < 0.5) ? (2.0 * b.g * f.g) : (1.0 - 2.0 * (1.0 - b.g) * (1.0 - f.g)),
        (b.b < 0.5) ? (2.0 * b.b * f.b) : (1.0 - 2.0 * (1.0 - b.b) * (1.0 - f.b))
    );
}

float SoftLightChannel(float b, float f) {
    if (f < 0.5) {
        return b - (1.0 - 2.0 * f) * b * (1.0 - b);
    } else {
        float d = (b > 0.25) ? sqrt(b) : ((16.0 * b - 12.0) * b + 4.0) * b;
        return b + (2.0 * f - 1.0) * (d - b);
    }
}

vec3 BlendSoftLight(vec3 b, vec3 f) {
    return vec3(
        SoftLightChannel(b.r, f.r),
        SoftLightChannel(b.g, f.g),
        SoftLightChannel(b.b, f.b)
    );
}

vec3 BlendDifference(vec3 b, vec3 f) {
    return abs(b - f);
}

void main() {
    vec4 back = texture(backTex, TexCoords);
    vec4 front = texture(frontTex, TexCoords);
    
    // Perform blending in RGB channels
    vec3 blendedRgb = vec3(0.0);
    
    if (blendMode == 0)      blendedRgb = BlendNormal(back.rgb, front.rgb);
    else if (blendMode == 1) blendedRgb = BlendMultiply(back.rgb, front.rgb);
    else if (blendMode == 2) blendedRgb = BlendScreen(back.rgb, front.rgb);
    else if (blendMode == 3) blendedRgb = BlendOverlay(back.rgb, front.rgb);
    else if (blendMode == 4) blendedRgb = BlendSoftLight(back.rgb, front.rgb);
    else if (blendMode == 5) blendedRgb = BlendDifference(back.rgb, front.rgb);
    else                     blendedRgb = BlendNormal(back.rgb, front.rgb);
    
    // Opacity mix (Photoshop blending math: front_alpha * opacity)
    float blendedAlpha = front.a * opacity;
    
    // Standard transparency alpha compositing (Over operator)
    float outAlpha = blendedAlpha + back.a * (1.0 - blendedAlpha);
    vec3 outRgb = vec3(0.0);
    
    if (outAlpha > 0.0) {
        outRgb = (blendedRgb * blendedAlpha + back.rgb * back.a * (1.0 - blendedAlpha)) / outAlpha;
    }
    
    FragColor = vec4(outRgb, outAlpha);
}
)";

Compositor::Compositor()
    : m_ShaderProgram(0), m_QuadVAO(0), m_QuadVBO(0),
      m_LocBackTex(-1), m_LocFrontTex(-1), m_LocOpacity(-1), m_LocBlendMode(-1) {}

Compositor::~Compositor() {
    Cleanup();
}

void Compositor::Cleanup() {
    if (m_ShaderProgram != 0) {
        glDeleteProgram(m_ShaderProgram);
        m_ShaderProgram = 0;
    }
    if (m_QuadVAO != 0) {
        glDeleteVertexArrays(1, &m_QuadVAO);
        m_QuadVAO = 0;
    }
    if (m_QuadVBO != 0) {
        glDeleteBuffers(1, &m_QuadVBO);
        m_QuadVBO = 0;
    }
}

bool Compositor::Init() {
    CompileShaders();
    SetupQuadGeometry();
    return m_ShaderProgram != 0;
}

void Compositor::CompileShaders() {
    // Compile Vertex Shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &s_VertexShaderSource, nullptr);
    glCompileShader(vs);
    
    GLint success;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vs, 512, nullptr, infoLog);
        std::cerr << "Compositor VS Compilation Failed:\n" << infoLog << std::endl;
    }

    // Compile Fragment Shader
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &s_FragmentShaderSource, nullptr);
    glCompileShader(fs);
    
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fs, 512, nullptr, infoLog);
        std::cerr << "Compositor FS Compilation Failed:\n" << infoLog << std::endl;
    }

    // Link Program
    m_ShaderProgram = glCreateProgram();
    glAttachShader(m_ShaderProgram, vs);
    glAttachShader(m_ShaderProgram, fs);
    glLinkProgram(m_ShaderProgram);

    glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_ShaderProgram, 512, nullptr, infoLog);
        std::cerr << "Compositor Shader Program Linking Failed:\n" << infoLog << std::endl;
        glDeleteProgram(m_ShaderProgram);
        m_ShaderProgram = 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (m_ShaderProgram != 0) {
        // Retrieve uniform locations
        m_LocBackTex = glGetUniformLocation(m_ShaderProgram, "backTex");
        m_LocFrontTex = glGetUniformLocation(m_ShaderProgram, "frontTex");
        m_LocOpacity = glGetUniformLocation(m_ShaderProgram, "opacity");
        m_LocBlendMode = glGetUniformLocation(m_ShaderProgram, "blendMode");
    }
}

void Compositor::SetupQuadGeometry() {
    // Quad vertices (Positions (X,Y) and TexCoords (U,V))
    float vertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_QuadVAO);
    glGenBuffers(1, &m_QuadVBO);

    glBindVertexArray(m_QuadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // TexCoords attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Compositor::Composite(const LayerStack& stack, GLuint targetFboId, int width, int height) {
    if (stack.GetCount() == 0 || m_ShaderProgram == 0) return;

    // Create a fallback 1x1 transparent texture to prevent sampler issues when binding 0
    static GLuint emptyTexture = 0;
    if (emptyTexture == 0) {
        glGenTextures(1, &emptyTexture);
        glBindTexture(GL_TEXTURE_2D, emptyTexture);
        unsigned char emptyPixels[4] = {0, 0, 0, 0};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, emptyPixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    // We need to render layers in order from bottom (index 0) to top (index N-1)
    // To do this on GPU, we will use a ping-pong buffer technique.
    // 1. Draw bottom layer to a temporary buffer or start directly if we have multiple FBOs.
    // Actually, we can create a temporary ping-pong texture in the compositor.
    // Let's create static ping-pong textures that resize dynamically matching the workspace size.
    static GLuint pingPongTextures[2] = {0, 0};
    static GLuint pingPongFBOs[2] = {0, 0};
    static int cachedWidth = 0, cachedHeight = 0;

    if (cachedWidth != width || cachedHeight != height) {
        std::cout << "[Compositor] Resizing ping-pong FBOs from " << cachedWidth << "x" << cachedHeight 
                  << " to " << width << "x" << height << std::endl;
        
        // Delete old textures/FBOs if size changed
        for (int i = 0; i < 2; ++i) {
            if (pingPongTextures[i] != 0) glDeleteTextures(1, &pingPongTextures[i]);
            if (pingPongFBOs[i] != 0) glDeleteFramebuffers(1, &pingPongFBOs[i]);
            
            glGenTextures(1, &pingPongTextures[i]);
            glBindTexture(GL_TEXTURE_2D, pingPongTextures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
            glGenFramebuffers(1, &pingPongFBOs[i]);
            glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBOs[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingPongTextures[i], 0);

            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "[Compositor] Ping-pong Framebuffer " << i << " is not complete! Status: " << status << std::endl;
            } else {
                std::cout << "[Compositor] Ping-pong Framebuffer " << i << " created successfully (Complete)." << std::endl;
            }
        }
        cachedWidth = width;
        cachedHeight = height;
    }

    // Save OpenGL states to restore them later
    GLint lastViewport[4];
    glGetIntegerv(GL_VIEWPORT, lastViewport);
    GLint lastFbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFbo);

    glViewport(0, 0, width, height);

    // Find the first visible layer to start composition
    int startLayerIdx = -1;
    for (int i = 0; i < stack.GetCount(); ++i) {
        if (stack.GetLayer(i)->visible) {
            startLayerIdx = i;
            break;
        }
    }

    // If no layers are visible, clear target and return
    if (startLayerIdx == -1) {
        glBindFramebuffer(GL_FRAMEBUFFER, targetFboId);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, lastFbo);
        glViewport(lastViewport[0], lastViewport[1], lastViewport[2], lastViewport[3]);
        return;
    }

    // Copy/Draw the first visible layer directly to our first ping-pong FBO
    int currentWriteBuffer = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBOs[currentWriteBuffer]);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Simple copy shader (which is just our compositor with normal blend and 1.0 opacity on a black background)
    glUseProgram(m_ShaderProgram);
    glBindVertexArray(m_QuadVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, emptyTexture); // Bind 1x1 transparent instead of 0
    glUniform1i(m_LocBackTex, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, stack.GetLayer(startLayerIdx)->textureId);
    glUniform1i(m_LocFrontTex, 1);

    glUniform1f(m_LocOpacity, stack.GetLayer(startLayerIdx)->opacity);
    glUniform1i(m_LocBlendMode, 0); // Normal blending

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Now loop through remaining visible layers and blend them on top
    for (int i = startLayerIdx + 1; i < stack.GetCount(); ++i) {
        Layer* layer = stack.GetLayer(i);
        if (!layer->visible) continue;

        int readBuffer = currentWriteBuffer;
        currentWriteBuffer = 1 - currentWriteBuffer;

        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBOs[currentWriteBuffer]);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Bind lower composite result as background texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pingPongTextures[readBuffer]);
        glUniform1i(m_LocBackTex, 0);

        // Bind current layer as foreground texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, layer->textureId);
        glUniform1i(m_LocFrontTex, 1);

        // Set opacity and blend mode
        glUniform1f(m_LocOpacity, layer->opacity);
        glUniform1i(m_LocBlendMode, static_cast<int>(layer->blendMode));

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // Finally, render the final composite result onto the target FBO (Canvas FBO)
    glBindFramebuffer(GL_FRAMEBUFFER, targetFboId);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, emptyTexture); // Bind 1x1 transparent instead of 0
    glUniform1i(m_LocBackTex, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingPongTextures[currentWriteBuffer]);
    glUniform1i(m_LocFrontTex, 1);

    glUniform1f(m_LocOpacity, 1.0f);
    glUniform1i(m_LocBlendMode, 0); // Normal blending

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Restore original GL states
    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, lastFbo);
    glViewport(lastViewport[0], lastViewport[1], lastViewport[2], lastViewport[3]);
}

} // namespace core
