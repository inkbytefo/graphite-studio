#include "core/GLWrappers.h"
#include <utility>

namespace core {

// ── GLBuffer (VBO / EBO) ─────────────────────────────────────────────────────

GLBuffer::GLBuffer() = default;

GLBuffer::~GLBuffer() {
    Destroy();
}

GLBuffer::GLBuffer(GLBuffer&& other) noexcept : m_Id(other.m_Id) {
    other.m_Id = 0;
}

GLBuffer& GLBuffer::operator=(GLBuffer&& other) noexcept {
    if (this != &other) {
        Destroy();
        m_Id = other.m_Id;
        other.m_Id = 0;
    }
    return *this;
}

void GLBuffer::Create() {
    Destroy();
    glCreateBuffers(1, &m_Id);
}

void GLBuffer::Destroy() {
    if (m_Id != 0) {
        glDeleteBuffers(1, &m_Id);
        m_Id = 0;
    }
}

void GLBuffer::Bind(GLenum target) const {
    if (m_Id != 0) {
        glBindBuffer(target, m_Id);
    }
}

void GLBuffer::Unbind(GLenum target) const {
    glBindBuffer(target, 0);
}

void GLBuffer::AllocateStorage(GLsizeiptr size, const void* data, GLbitfield flags) {
    if (m_Id != 0) {
        glNamedBufferStorage(m_Id, size, data, flags);
    }
}

void GLBuffer::UpdateSubData(GLintptr offset, GLsizeiptr size, const void* data) {
    if (m_Id != 0) {
        glNamedBufferSubData(m_Id, offset, size, data);
    }
}

// ── GLVertexArray (VAO) ──────────────────────────────────────────────────────

GLVertexArray::GLVertexArray() = default;

GLVertexArray::~GLVertexArray() {
    Destroy();
}

GLVertexArray::GLVertexArray(GLVertexArray&& other) noexcept : m_Id(other.m_Id) {
    other.m_Id = 0;
}

GLVertexArray& GLVertexArray::operator=(GLVertexArray&& other) noexcept {
    if (this != &other) {
        Destroy();
        m_Id = other.m_Id;
        other.m_Id = 0;
    }
    return *this;
}

void GLVertexArray::Create() {
    Destroy();
    glCreateVertexArrays(1, &m_Id);
}

void GLVertexArray::Destroy() {
    if (m_Id != 0) {
        glDeleteVertexArrays(1, &m_Id);
        m_Id = 0;
    }
}

void GLVertexArray::Bind() const {
    if (m_Id != 0) {
        glBindVertexArray(m_Id);
    }
}

void GLVertexArray::Unbind() const {
    glBindVertexArray(0);
}

// ── GLTexture ────────────────────────────────────────────────────────────────

GLTexture::GLTexture() = default;

GLTexture::~GLTexture() {
    Destroy();
}

GLTexture::GLTexture(GLTexture&& other) noexcept 
    : m_Id(other.m_Id), m_Target(other.m_Target) {
    other.m_Id = 0;
}

GLTexture& GLTexture::operator=(GLTexture&& other) noexcept {
    if (this != &other) {
        Destroy();
        m_Id = other.m_Id;
        m_Target = other.m_Target;
        other.m_Id = 0;
    }
    return *this;
}

void GLTexture::Create(GLenum target) {
    Destroy();
    m_Target = target;
    glCreateTextures(m_Target, 1, &m_Id);
}

void GLTexture::Destroy() {
    if (m_Id != 0) {
        glDeleteTextures(1, &m_Id);
        m_Id = 0;
    }
}

void GLTexture::Bind(GLuint unit) const {
    if (m_Id != 0) {
        glBindTextureUnit(unit, m_Id);
    }
}

void GLTexture::AllocateStorage2D(GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height) {
    if (m_Id != 0) {
        glTextureStorage2D(m_Id, levels, internalFormat, width, height);
    }
}

void GLTexture::UploadSubImage2D(GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) {
    if (m_Id != 0) {
        glTextureSubImage2D(m_Id, level, xoffset, yoffset, width, height, format, type, pixels);
    }
}

void GLTexture::SetParameteri(GLenum pname, GLint param) {
    if (m_Id != 0) {
        glTextureParameteri(m_Id, pname, param);
    }
}

// ── GLFramebuffer (FBO) ──────────────────────────────────────────────────────

GLFramebuffer::GLFramebuffer() = default;

GLFramebuffer::~GLFramebuffer() {
    Destroy();
}

GLFramebuffer::GLFramebuffer(GLFramebuffer&& other) noexcept : m_Id(other.m_Id) {
    other.m_Id = 0;
}

GLFramebuffer& GLFramebuffer::operator=(GLFramebuffer&& other) noexcept {
    if (this != &other) {
        Destroy();
        m_Id = other.m_Id;
        other.m_Id = 0;
    }
    return *this;
}

void GLFramebuffer::Create() {
    Destroy();
    glCreateFramebuffers(1, &m_Id);
}

void GLFramebuffer::Destroy() {
    if (m_Id != 0) {
        glDeleteFramebuffers(1, &m_Id);
        m_Id = 0;
    }
}

void GLFramebuffer::Bind(GLenum target) const {
    if (m_Id != 0) {
        glBindFramebuffer(target, m_Id);
    }
}

void GLFramebuffer::Unbind(GLenum target) const {
    glBindFramebuffer(target, 0);
}

void GLFramebuffer::AttachTexture(GLenum attachment, const GLTexture& texture, GLint level) {
    if (m_Id != 0) {
        glNamedFramebufferTexture(m_Id, attachment, texture.GetId(), level);
    }
}

void GLFramebuffer::AttachTextureId(GLenum attachment, GLuint textureId, GLint level) {
    if (m_Id != 0) {
        glNamedFramebufferTexture(m_Id, attachment, textureId, level);
    }
}

bool GLFramebuffer::CheckStatus(GLenum target) const {
    if (m_Id != 0) {
        return glCheckNamedFramebufferStatus(m_Id, target) == GL_FRAMEBUFFER_COMPLETE;
    }
    return false;
}

} // namespace core
