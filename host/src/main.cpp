// NeuroTrace host: reads samples, filters, prints band powers and blink events.
//   ./neurotrace_host --sim            synthetic signal (real-time)
//   ./neurotrace_host --port /dev/ttyUSB0
#include <cstring>
#include <deque>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "neurotrace/band_power.h"
#include "neurotrace/blink_detector.h"
#include "neurotrace/channel_filter.h"
#include "neurotrace/sample_source.h"

int main(int argc, char** argv) {
  constexpr double kFs = 250.0;
  std::unique_ptr<nt::SampleSource> source;

  for (int i = 1; i < argc; ++i) {
    if (!std::strcmp(argv[i], "--sim")) source = nt::makeSimulatedSource(kFs, true);
    else if (!std::strcmp(argv[i], "--port") && i + 1 < argc) source = nt::makeSerialSource(argv[++i]);
  }
  if (!source) {
    std::cerr << "Usage: " << argv[0] << " --sim | --port <device>\n";
    return 1;
  }

  std::vector<nt::ChannelFilter> filters(nt::kNumChannels, nt::ChannelFilter(kFs));
  nt::BandPowerAnalyzer analyzer(kFs, 256);
  nt::BlinkDetector blink(kFs);

  constexpr size_t kWindow = 1024;  // ~4 s analysis window
  std::deque<float> window;
  size_t sampleCount = 0;

  nt::Sample s;
  while (source->read(s)) {
    float ch0 = 0;
    for (size_t c = 0; c < nt::kNumChannels; ++c) {
      const float f = filters[c].process(s.uv[c]);
      if (c == 0) ch0 = f;
    }
    window.push_back(ch0);
    if (window.size() > kWindow) window.pop_front();
    ++sampleCount;

    const bool blinked = blink.update(ch0);
    if (blinked && sampleCount > kFs)  // ignore filter start-up transient
      std::cout << "[blink] t=" << sampleCount / kFs << " s" << std::endl;

    if (sampleCount % static_cast<size_t>(kFs) == 0 && window.size() == kWindow) {
      const auto bp = analyzer.compute({window.begin(), window.end()});
      const double total = bp.delta + bp.theta + bp.alpha + bp.beta;
      std::cout << std::fixed << std::setprecision(1) << "t=" << sampleCount / kFs << " s  delta=" << bp.delta
                << " theta=" << bp.theta << " alpha=" << bp.alpha << " beta=" << bp.beta
                << " uV^2  alpha/total=" << std::setprecision(2) << (total > 0 ? bp.alpha / total : 0) << std::endl;
      // TODO: forward bp to a visualiser (Unreal/Unity over UDP/OSC, or Dear ImGui + ImPlot).
    }
  }
  return 0;
}
