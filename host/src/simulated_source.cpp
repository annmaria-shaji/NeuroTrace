#include <chrono>
#include <cmath>
#include <random>
#include <thread>

#include "neurotrace/sample_source.h"

namespace nt {
namespace {

class SimulatedSource : public SampleSource {
 public:
  SimulatedSource(double fs, bool realtime) : fs_(fs), realtime_(realtime), rng_(42), noise_(0.0f, 2.0f) {}

  bool read(Sample& out) override {
    if (realtime_) std::this_thread::sleep_for(std::chrono::duration<double>(1.0 / fs_));
    const double t = n_ / fs_;
    // Alpha is strong for 5 s ("eyes closed"), weak for 5 s ("eyes open").
    const double alphaAmp = (static_cast<long>(t / 5.0) % 2 == 0) ? 20.0 : 4.0;
    const double tb = std::fmod(t, 7.0);  // a blink every 7 s
    for (size_t c = 0; c < kNumChannels; ++c) {
      double v = alphaAmp * std::sin(2 * M_PI * 10.0 * t + c) + 3.0 * std::sin(2 * M_PI * 20.0 * t) +
                 15.0 * std::sin(2 * M_PI * 50.0 * t) + noise_(rng_);
      if (tb < 0.15) v += 250.0 * std::sin(M_PI * tb / 0.15);
      out.uv[c] = static_cast<float>(v);
    }
    ++n_;
    return true;
  }

 private:
  double fs_;
  bool realtime_;
  unsigned long n_ = 0;
  std::mt19937 rng_;
  std::normal_distribution<float> noise_;
};

}  // namespace

std::unique_ptr<SampleSource> makeSimulatedSource(double fs, bool realtime) {
  return std::make_unique<SimulatedSource>(fs, realtime);
}

}  // namespace nt
