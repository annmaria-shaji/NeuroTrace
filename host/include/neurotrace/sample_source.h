#pragma once
#include <array>
#include <memory>
#include <string>

#include "neurotrace_protocol.h"

namespace nt {

struct Sample {
  std::array<float, kNumChannels> uv{};  // microvolts
};

class SampleSource {
 public:
  virtual ~SampleSource() = default;
  // Blocks until the next sample is available. Returns false on end of stream / error.
  virtual bool read(Sample& out) = 0;
};

// Synthetic EEG: 10 Hz alpha (toggling on/off) + 20 Hz beta + 50 Hz mains hum + noise + blinks.
std::unique_ptr<SampleSource> makeSimulatedSource(double fs, bool realtime);

// Reads protocol frames from a serial port (POSIX: Linux/macOS), e.g. "/dev/ttyUSB0".
std::unique_ptr<SampleSource> makeSerialSource(const std::string& port, int baud = 921600);

}  // namespace nt
