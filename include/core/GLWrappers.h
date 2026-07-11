#pragma once

#include <glad/gl.h>

namespace core {

// Move-only RAII sarmalayıcısı (wrapper) base.
// Prevent copies of OpenGL resources to avoid double-free errors.
class GLResource {
public:
    GLResource() = default;
    virtual ~GLResource() = default;

    GLResource(const GLResource&) = delete;
    GLResource& operator=(const GLResource&) = delete;
};

// ── GLBuffer (VBO / EBO) ─────────────────────────────────────────────────────
class GLBuffer : public GLResource {
public:
    GLBuffer();
    ~GLBuffer() override;

    GLBuffer(GLBuffer&& other) noexcept;
    GLBuffer& operator=(GLBuffer&& other) noexcept;

    void Create();
    void Destroy();
    void Bind(GLenum target) const;
    void Unbind(GLenum target) const;

    // Direct State Access (DSA): Set immutable storage (OpenGL 4.5+)
    void AllocateStorage(GLsizeiptr size, const void* data, GLbitfield flags);

    // Direct State Access (DSA): Update sub-data
    void UpdateSubData(GLintptr offset, GLsizeiptr size, const void* data);

    GLuint GetId() const { return m_Id; }

private:
    GLuint m_Id = 0;
};

// ── GLVertexArray (VAO) ──────────────────────────────────────────────────────
class GLVertexArray : public GLResource {
public:
    GLVertexArray();
    ~GLVertexArray() override;

    GLVertexArray(GLVertexArray&& other) noexcept;
    GLVertexArray& operator=(GLVertexArray&& other) noexcept;

    void Create();
    void Destroy();
    void Bind() const;
    void Unbind() const;

    GLuint GetId() const { return m_Id; }

private:
    GLuint m_Id = 0;
};

// ── GLTexture ────────────────────────────────────────────────────────────────
class GLTexture : public GLResource {
public:
    GLTexture();
    ~GLTexture() override;

    GLTexture(GLTexture&& other) noexcept;
    GLTexture& operator=(GLTexture&& other) noexcept;

    void Create(GLenum target = GL_TEXTURE_2D);
    void Destroy();
    void Bind(GLuint unit) const;

    // DSA: Set texture storage (OpenGL 4.5+)
    void AllocateStorage2D(GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height);

    // DSA: Upload texture data
    void UploadSubImage2D(GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);

    // DSA: Set parameters
    void SetParameteri(GLenum pname, GLint param);

    GLuint GetId() const { return m_Id; }

private:
    GLuint m_Id = 0;
    GLenum m_Target = GL_TEXTURE_2D;
};

// ── GLFramebuffer (FBO) ──────────────────────────────────────────────────────
class GLFramebuffer : public GLResource {
public:
    GLFramebuffer();
    ~GLFramebuffer() override;

    GLFramebuffer(GLFramebuffer&& other) noexcept;
    GLFramebuffer& operator=(GLFramebuffer&& other) noexcept;

    void Create();
    void Destroy();
    void Bind(GLenum target = GL_FRAMEBUFFER) const;
    void Unbind(GLenum target = GL_FRAMEBUFFER) const;

    // DSA: Attach texture to framebuffer (OpenGL 4.5+)
    void AttachTexture(GLenum attachment, const GLTexture& texture, GLint level = 0);
    void AttachTextureId(GLenum attachment, GLuint textureId, GLint level = 0);

    bool CheckStatus(GLenum target = GL_FRAMEBUFFER) const;

    GLuint GetId() const { return m_Id; }

private:
    GLuint m_Id = 0;
};

} // namespace core
