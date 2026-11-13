#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include "neurotrace/sample_source.h"

namespace nt {
namespace {

speed_t toSpeed(int baud) {
  switch (baud) {
    case 115200: return B115200;
    case 230400: return B230400;
#ifdef B921600
    case 921600: return B921600;
#endif
    default: return B115200;
  }
}

class SerialSource : public SampleSource {
 public:
  SerialSource(const std::string& port, int baud) {
    fd_ = ::open(port.c_str(), O_RDONLY | O_NOCTTY);
    if (fd_ < 0) { std::cerr << "Cannot open " << port << ": " << std::strerror(errno) << "\n"; return; }
    termios tty{};
    tcgetattr(fd_, &tty);
    cfmakeraw(&tty);
    cfsetispeed(&tty, toSpeed(baud));
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;
    tcsetattr(fd_, TCSANOW, &tty);
  }
  ~SerialSource() override { if (fd_ >= 0) ::close(fd_); }

  bool read(Sample& out) override {
    if (fd_ < 0) return false;
    // Resynchronise on the start byte, then read the rest of the frame.
    uint8_t frame[kFrameSize];
    for (;;) {
      uint8_t b;
      if (::read(fd_, &b, 1) != 1) return false;
      if (b != kStartByte) continue;
      frame[0] = b;
      size_t got = 1;
      while (got < kFrameSize) {
        const ssize_t r = ::read(fd_, frame + got, kFrameSize - got);
        if (r <= 0) return false;
        got += static_cast<size_t>(r);
      }
      uint8_t counter;
      int32_t raw[kNumChannels];
      if (!decodeFrame(frame, counter, raw)) continue;  // bad frame, resync
      // TODO: track `counter` to count dropped frames.
      for (size_t c = 0; c < kNumChannels; ++c) out.uv[c] = rawToMicrovolts(raw[c]);
      return true;
    }
  }

 private:
  int fd_ = -1;
};

}  // namespace

std::unique_ptr<SampleSource> makeSerialSource(const std::string& port, int baud) {
  return std::make_unique<SerialSource>(port, baud);
}

}  // namespace nt
