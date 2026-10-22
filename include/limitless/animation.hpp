#pragma once

#include <algorithm>
#include <cmath>

namespace limitless {
enum class Easing { Linear, CubicOut, ExpoOut, Smoothstep };

inline float ease(Easing easing, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    switch (easing) {
    case Easing::Linear:
        return t;
    case Easing::CubicOut:
        return 1 - std::pow(1 - t, 3);
    case Easing::ExpoOut:
        return t >= 1 ? 1 : 1 - std::pow(2, -10 * t);
    case Easing::Smoothstep:
        return t * t * (3 - 2 * t);
    }
    return t;
}

class Animation {
  public:
    float update(float target, float delta, float duration, bool snap = false,
                 Easing easing = Easing::CubicOut) {
        if (!initialized_ || snap) {
            value_ = start_ = target_ = target;
            elapsed_ = duration;
            initialized_ = true;
        }
        if (target != target_) {
            start_ = value_;
            target_ = target;
            elapsed_ = 0;
        }
        elapsed_ += (std::max)(delta, 0.0f);
        const float t = duration > 0 ? std::clamp(elapsed_ / duration, 0.0f, 1.0f) : 1.0f;
        value_ = start_ + (target_ - start_) * ease(easing, t);
        return value_;
    }
    void reset(float value) {
        value_ = start_ = target_ = value;
        elapsed_ = 0;
        initialized_ = true;
    }

  private:
    float value_ = 0;
    float start_ = 0;
    float target_ = 0;
    float elapsed_ = 0;
    bool initialized_ = false;
};

class Spring {
  public:
    float update(float target, float delta, float stiffness = 520, float damping = 42) {
        if (!initialized_) {
            value_ = target_ = target;
            velocity_ = 0;
            initialized_ = true;
        }
        target_ = target;
        float remaining = std::clamp(delta, 0.0f, 0.05f);
        constexpr float step = 1.0f / 240.0f;
        while (remaining > 0) {
            const float h = (std::min)(step, remaining);
            const float acceleration = -stiffness * (value_ - target_) - damping * velocity_;
            velocity_ += acceleration * h;
            value_ += velocity_ * h;
            remaining -= h;
        }
        if (std::abs(value_ - target_) < 0.0005f && std::abs(velocity_) < 0.0005f) {
            value_ = target_;
            velocity_ = 0;
        }
        return value_;
    }
    void reset(float value) {
        value_ = target_ = value;
        velocity_ = 0;
        initialized_ = true;
    }

  private:
    float value_ = 0;
    float target_ = 0;
    float velocity_ = 0;
    bool initialized_ = false;
};
}
