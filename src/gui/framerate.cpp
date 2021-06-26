//
// Created by Mike Smith on 2021/6/13.
//

#include <gui/framerate.h>

namespace luisa::gui {

Framerate::Framerate(double smoothness) noexcept
    : _last{Clock::now()},
      _alpha{smoothness},
      _dt{0.0f},
      _count{0u} {}

double Framerate::tick() noexcept {
    using namespace std::chrono_literals;
    auto last = _last;
    _last = Clock::now();
    _count++;
    auto dt = static_cast<double>((_last - last) / 1ns) * 1e-6;
    _dt = _count == 0u ? dt : (_alpha * _dt + (1.0f - _alpha) * dt);
    return dt;
}

double Framerate::fps() const noexcept {
    return _count == 0u ? 0.0 : 1000.0 / _dt;
}

void Framerate::clear() noexcept {
    _last = Clock::now();
    _dt = 0.0;
    _count = 0u;
}

}
