#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "neurotrace/band_power.h"
#include "neurotrace/channel_filter.h"
#include "neurotrace_protocol.h"

int main() {
  constexpr double fs = 250.0;

  // 1) Protocol round trip, including negative 24-bit values.
  {
    const int32_t in[nt::kNumChannels] = {-123456, 7654321};
    uint8_t frame[nt::kFrameSize], counter;
    int32_t out[nt::kNumChannels];
    nt::encodeFrame(9, in, frame);
    bool ok = nt::decodeFrame(frame, counter, out);
    assert(ok && counter == 9 && out[0] == in[0] && out[1] == in[1]);
    (void)ok;
  }

  // 2) A 10 Hz sine must show up in the alpha band.
  {
    nt::ChannelFilter filt(fs);
    std::vector<float> x;
    for (int n = 0; n < 1024 + 500; ++n) {
      const float v = filt.process(20.0f * std::sin(2 * M_PI * 10.0 * n / fs));
      if (n >= 500) x.push_back(v);  // skip filter transient
    }
    const auto bp = nt::BandPowerAnalyzer(fs, 256).compute(x);
    std::cout << "alpha=" << bp.alpha << " beta=" << bp.beta << " theta=" << bp.theta << "\n";
    if (!(bp.alpha > 10 * bp.beta && bp.alpha > 10 * bp.theta)) { std::cerr << "FAIL alpha dominance\n"; return 1; }
    if (std::fabs(bp.alpha - 200.0) > 40.0) { std::cerr << "FAIL alpha power (expected ~200)\n"; return 1; }
  }

  // 3) 50 Hz mains is strongly attenuated.
  {
    nt::ChannelFilter filt(fs);
    double in = 0, out = 0;
    for (int n = 0; n < 2000; ++n) {
      const float v = 50.0f * std::sin(2 * M_PI * 50.0 * n / fs);
      const float y = filt.process(v);
      if (n > 500) { in += v * v; out += y * y; }
    }
    if (!(out < 0.01 * in)) { std::cerr << "FAIL notch\n"; return 1; }
  }
  std::cout << "all tests passed\n";
  return 0;
}
