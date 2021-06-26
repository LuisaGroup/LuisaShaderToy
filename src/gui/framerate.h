//
// Created by Mike Smith on 2021/6/13.
//

#pragma once

#include <chrono>

namespace luisa::gui {

class Framerate {

    using Clock = std::chrono::high_resolution_clock;
    using Tick = std::chrono::high_resolution_clock::time_point;

private:
    Tick _last;
    double _alpha;
    double _dt;
    uint64_t _count;

public:
    explicit Framerate(double smoothness = 0.5) noexcept;
    void clear() noexcept;
    double tick() noexcept;
    [[nodiscard]] auto count() const noexcept { return _count; }
    [[nodiscard]] double fps() const noexcept;
};

}
