//
// Created by Mike Smith on 2021/6/13.
//

#pragma once

#include <iostream>
#include <memory>
#include <span>
#include <type_traits>

#include <core/basic_types.h>
#include <core/logging.h>
#include <core/concepts.h>
#include <runtime/pixel.h>

namespace luisa::gui {

using compute::PixelFormat;

class GLTexture {

private:
    uint2 _size;
    int _format;
    int _type;
    uint _handle{0u};
    uint _pixel_size;
    std::vector<std::byte> _front_buffer;
    std::vector<std::byte> _back_buffer;

private:
    [[nodiscard]] static int _gl_format(PixelFormat format) noexcept;
    [[nodiscard]] static int _gl_type(PixelFormat format) noexcept;
    [[nodiscard]] size_t _size_bytes() const noexcept;
    void _destroy() noexcept;
    [[nodiscard]] bool _upload() noexcept;

public:
    GLTexture(PixelFormat format, uint2 size) noexcept;
    GLTexture(PixelFormat format, uint width, uint height) noexcept : GLTexture{format, {width, height}} {}
    GLTexture(GLTexture &&) noexcept = delete;
    GLTexture(const GLTexture &) noexcept = delete;
    GLTexture &operator=(GLTexture &&) noexcept = delete;
    GLTexture &operator=(const GLTexture &) noexcept = delete;
    ~GLTexture() noexcept;

    [[nodiscard]] uint64_t handle() const noexcept;
    void resize(uint2 size) noexcept;

    template<typename Update, std::enable_if_t<std::is_invocable_v<Update, void *>, int> = 0>
    bool present(Update &&update) {
        _back_buffer.resize(_size_bytes());
        update(_back_buffer.data());
        return _upload();
    }
};

}// namespace luisa::gui
