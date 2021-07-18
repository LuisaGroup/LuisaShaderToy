//
// Created by Mike Smith on 2021/6/27.
//

#include <glad/glad.h>
#include <gui/gl_texture.h>

namespace luisa::gui {

int GLTexture::_gl_format(PixelFormat format) noexcept {
    switch (format) {
        case PixelFormat::R8UNorm:
        case PixelFormat::R32F:
            return GL_RED;
        case PixelFormat::RG8UNorm:
        case PixelFormat::RG32F:
            return GL_RG;
        case PixelFormat::RGBA8UNorm:
        case PixelFormat::RGBA32F:
            return GL_RGBA;
        default:
            LUISA_ERROR_WITH_LOCATION("Invalid pixel format.");
    }
}

int GLTexture::_gl_type(PixelFormat format) noexcept {
    switch (format) {
        case PixelFormat::R8UNorm:
        case PixelFormat::RG8UNorm:
        case PixelFormat::RGBA8UNorm:
            return GL_UNSIGNED_BYTE;
        case PixelFormat::R32F:
        case PixelFormat::RG32F:
        case PixelFormat::RGBA32F:
            return GL_FLOAT;
        default:
            LUISA_ERROR_WITH_LOCATION("Invalid pixel format.");
    }
}

void GLTexture::_destroy() noexcept {
    if (_handle != 0u) {
        glDeleteTextures(1, &_handle);
        _handle = 0u;
    }
}

size_t GLTexture::_size_bytes() const noexcept { return _size.x * _size.y * _pixel_size; }

void GLTexture::resize(uint2 size) noexcept {
    _size = size;
    glTexImage2D(GL_TEXTURE_2D, 0, _format, size.x, size.y, 0, _format, _type, nullptr);
}

GLTexture::GLTexture(PixelFormat format, uint2 size) noexcept
    : _format{_gl_format(format)},
      _type{_gl_type(format)},
      _pixel_size{static_cast<uint>(compute::pixel_format_size(format))} {
    glGenTextures(1, &_handle);
    glBindTexture(GL_TEXTURE_2D, _handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    resize(size);
}

GLTexture::~GLTexture() noexcept { _destroy(); }

bool GLTexture::_upload() noexcept {
    auto valid = _front_buffer.size() == _size_bytes();
    if (valid) {
        glBindTexture(GL_TEXTURE_2D, _handle);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _size.x, _size.y, _format, _type, _front_buffer.data());
    }
    std::swap(_front_buffer, _back_buffer);
    return valid;
}

uint64_t GLTexture::handle() const noexcept { return _handle; }

}// namespace luisa::gui
