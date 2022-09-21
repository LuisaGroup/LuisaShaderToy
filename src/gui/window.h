//
// Created by Mike Smith on 2021/6/13.
//

#pragma once

#include <string_view>
#include <type_traits>

#include <imgui/imgui.h>

#include <core/basic_types.h>
#include <core/concepts.h>
#include <gui/hid.h>

namespace luisa::gui {

class Window : concepts::Noncopyable {

public:
    using Handle = struct GLFWwindow *;

private:
    double _time_start;
    Handle _handle{nullptr};
    ImGuiContext *_context{nullptr};

private:
    void _begin_frame() noexcept;
    void _end_frame() noexcept;
    void _destroy() noexcept;

public:
    Window(std::string_view title, uint32_t width, uint32_t height) noexcept;
    Window(std::string_view title, uint2 size) noexcept : Window{title, size.x, size.y} {}
    ~Window() noexcept;

    Window(Window &&another) noexcept;
    Window &operator=(Window &&rhs) noexcept;

    [[nodiscard]] bool should_close() const noexcept;
    void notify_close() noexcept;

    template<typename Frame, std::enable_if_t<std::is_invocable_v<Frame>, int> = 0>
    void with_frame(Frame &&frame) {
        if (!should_close()) {
            _begin_frame();
            frame();
            _end_frame();
        } else {
            _destroy();
        }
    }

    template<typename Frame, std::enable_if_t<std::is_invocable_v<Frame>, int> = 0>
    void run(Frame &&frame) {
        while (!should_close()) {
            _begin_frame();
            frame();
            _end_frame();
        }
        _destroy();
    }

    [[nodiscard]] bool key_down(Key key) const noexcept;
    [[nodiscard]] bool key_up(Key key) const noexcept;
    [[nodiscard]] bool mouse_down(Mouse button) const noexcept;
    [[nodiscard]] bool mouse_up(Mouse button) const noexcept;
    [[nodiscard]] float2 cursor() const noexcept;
    [[nodiscard]] uint2 size() const noexcept;
    [[nodiscard]] uint2 framebuffer_size() const noexcept;
    [[nodiscard]] float time() const noexcept;
};

}// namespace luisa::gui
