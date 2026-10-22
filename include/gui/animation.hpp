#pragma once

#include <algorithm>
#include <cmath>

namespace gui {
class Animation {
  public:
    float update(float target, float delta, float duration, bool snap = false) {
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
        value_ = start_ + (target_ - start_) * (1 - std::pow(1 - t, 3));
        return value_;
    }
    void reset(float value) {
        value_ = start_ = target_ = value;
        elapsed_ = 0;
        initialized_ = true;
    }
    float target() const {
        return target_;
    }

  private:
    float value_ = 0;
    float start_ = 0;
    float target_ = 0;
    float elapsed_ = 0;
    bool initialized_ = false;
};
}
