//
// Created by Mike Smith on 2021/6/13.
//

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <core/logging.h>
#include <gui/imgui_impl_glfw.h>
#include <gui/window.h>

namespace luisa::gui {

Window::Window(std::string_view title, uint32_t width, uint32_t height) noexcept
    : _time_start{glfwGetTime()} {

    static std::once_flag once_flag;
    std::call_once(once_flag, [] {
        glfwSetErrorCallback([](int error, const char *description) noexcept {
            LUISA_ERROR_WITH_LOCATION("GLFW Error {}: {}.", error, description);
        });
        if (!glfwInit()) { LUISA_ERROR_WITH_LOCATION("Failed to initialize GLFW."); }
    });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    _handle = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height),
                               std::string{title}.c_str(), nullptr, nullptr);
    if (_handle == nullptr) {
        LUISA_ERROR_WITH_LOCATION("Failed to create window '{}' with size {}x{}.", title, width, height);
    }

    glfwMakeContextCurrent(_handle);
    glfwSwapInterval(0);

    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        LUISA_ERROR_WITH_LOCATION("Failed to initialize OpenGL loader.");
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(nullptr);
    _context = ImGui::CreateContext();
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(_handle);
    ImGui_ImplOpenGL3_Init("#version 150");
    ImGui_ImplOpenGL3_CreateDeviceObjects();
    ImGui::SetCurrentContext(nullptr);
}

void Window::_begin_frame() noexcept {
    glfwMakeContextCurrent(_handle);
    ImGui::SetCurrentContext(_context);
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame(_handle);
    ImGui::NewFrame();
    int display_w, display_h;
    glfwGetFramebufferSize(_handle, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::_end_frame() noexcept {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(_handle);
    ImGui::SetCurrentContext(nullptr);
    glfwMakeContextCurrent(nullptr);
}

Window::~Window() noexcept { _destroy(); }

bool Window::should_close() const noexcept {
    return _handle == nullptr || glfwWindowShouldClose(_handle);
}

void Window::notify_close() noexcept {
    if (_handle != nullptr) { glfwSetWindowShouldClose(_handle, true); }
}

bool Window::key_down(Key key) const noexcept {
    return glfwGetKey(_handle, static_cast<int>(key)) == GLFW_PRESS;
}

bool Window::mouse_down(Mouse button) const noexcept {
    return glfwGetMouseButton(_handle, static_cast<int>(button)) == GLFW_PRESS;
}

bool Window::key_up(Key key) const noexcept {
    return glfwGetKey(_handle, static_cast<int>(key)) == GLFW_RELEASE;
}

bool Window::mouse_up(Mouse button) const noexcept {
    return glfwGetMouseButton(_handle, static_cast<int>(button)) == GLFW_RELEASE;
}

float2 Window::cursor() const noexcept {
    auto x = 0.0;
    auto y = 0.0;
    glfwGetCursorPos(_handle, &x, &y);
    auto window_size = size();
    return {std::clamp(static_cast<float>(x), 0.0f, static_cast<float>(window_size.x)),
            std::clamp(static_cast<float>(y), 0.0f, static_cast<float>(window_size.y))};
}

void Window::_destroy() noexcept {
    if (_context != nullptr) {
        ImGui::SetCurrentContext(_context);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown(_handle);
        ImGui::DestroyContext(_context);
        ImGui::SetCurrentContext(nullptr);
    }
    if (_handle != nullptr) {
        glfwDestroyWindow(_handle);
    }
    _context = nullptr;
    _handle = nullptr;
}

Window &Window::operator=(Window &&rhs) noexcept {
    if (&rhs != this) {
        _destroy();
        _time_start = rhs._time_start;
        _handle = rhs._handle;
        _context = rhs._context;
        rhs._handle = nullptr;
        rhs._context = nullptr;
    }
    return *this;
}

Window::Window(Window &&another) noexcept
    : _time_start{another._time_start},
      _handle{another._handle},
      _context{another._context} {
    another._handle = nullptr;
    another._context = nullptr;
}

uint2 Window::size() const noexcept {
    auto w = 0;
    auto h = 0;
    glfwGetWindowSize(_handle, &w, &h);
    return make_uint2(w, h);
}

uint2 Window::framebuffer_size() const noexcept {
    auto w = 0;
    auto h = 0;
    glfwGetFramebufferSize(_handle, &w, &h);
    return make_uint2(w, h);
}

float Window::time() const noexcept {
    return static_cast<float>(glfwGetTime() - _time_start);
}

}// namespace luisa::gui
