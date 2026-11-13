#pragma once
#include <cmath>
#include <cstddef>

namespace nt {

// Simple threshold detector for eye blinks on a frontal channel (Fp1/Fp2).
// Blinks produce large (often > 100 uV) slow deflections. Tune threshold per setup.
class BlinkDetector {
 public:
  BlinkDetector(double fs, float thresholdUv = 100.0f, double refractorySec = 0.3)
      : threshold_(thresholdUv), refractory_(static_cast<size_t>(refractorySec * fs)) {}

  // Feed one filtered sample (uV). Returns true on the sample where a blink is detected.
  bool update(float uv) {
    if (cooldown_ > 0) { --cooldown_; return false; }
    if (std::fabs(uv) > threshold_) { cooldown_ = refractory_; return true; }
    return false;
  }

 private:
  float threshold_;
  size_t refractory_;
  size_t cooldown_ = 0;
};

}  // namespace nt
