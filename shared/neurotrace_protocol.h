// Shared wire protocol between firmware and host. Header-only, no dependencies.
// Frame (9 bytes): [0xA0][counter][ch1: 3B big-endian][ch2: 3B big-endian][0xC0]
#pragma once
#include <cstddef>
#include <cstdint>

namespace nt {

constexpr uint8_t kStartByte = 0xA0;
constexpr uint8_t kEndByte = 0xC0;
constexpr size_t kNumChannels = 2;
constexpr size_t kFrameSize = 1 + 1 + 3 * kNumChannels + 1;

inline int32_t signExtend24(uint32_t v) {
  return (v & 0x800000u) ? static_cast<int32_t>(v | 0xFF000000u) : static_cast<int32_t>(v);
}

inline void encodeFrame(uint8_t counter, const int32_t (&raw)[kNumChannels],
                        uint8_t (&out)[kFrameSize]) {
  size_t i = 0;
  out[i++] = kStartByte;
  out[i++] = counter;
  for (size_t c = 0; c < kNumChannels; ++c) {
    const uint32_t v = static_cast<uint32_t>(raw[c]) & 0xFFFFFFu;
    out[i++] = static_cast<uint8_t>(v >> 16);
    out[i++] = static_cast<uint8_t>(v >> 8);
    out[i++] = static_cast<uint8_t>(v);
  }
  out[i] = kEndByte;
}

inline bool decodeFrame(const uint8_t (&in)[kFrameSize], uint8_t& counter,
                        int32_t (&raw)[kNumChannels]) {
  if (in[0] != kStartByte || in[kFrameSize - 1] != kEndByte) return false;
  counter = in[1];
  for (size_t c = 0; c < kNumChannels; ++c) {
    const size_t o = 2 + 3 * c;
    raw[c] = signExtend24((uint32_t(in[o]) << 16) | (uint32_t(in[o + 1]) << 8) | in[o + 2]);
  }
  return true;
}

// ADS1299: LSB = 2*Vref/gain / 2^24, Vref = 4.5 V. Returns microvolts.
inline float rawToMicrovolts(int32_t raw, float gain = 24.0f) {
  return static_cast<float>(raw) * (4.5f / gain / 8388607.0f) * 1e6f;
}

}  // namespace nt
