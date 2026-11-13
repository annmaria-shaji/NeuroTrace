#pragma once
#include <cstddef>
#include <vector>

namespace nt {

struct BandPowers {
  double delta = 0;  // 1-4 Hz
  double theta = 0;  // 4-8 Hz
  double alpha = 0;  // 8-13 Hz
  double beta = 0;   // 13-30 Hz   (all in uV^2)
};

// Welch PSD estimate (Hann window, 50 % overlap) integrated over the EEG bands.
class BandPowerAnalyzer {
 public:
  explicit BandPowerAnalyzer(double fs, size_t segmentLength = 256);  // length must be a power of 2
  BandPowers compute(const std::vector<float>& x) const;
  std::vector<double> psd(const std::vector<float>& x) const;  // one-sided, uV^2/Hz

 private:
  double fs_;
  size_t n_;
  std::vector<double> window_;
  double windowPower_;
};

}  // namespace nt
