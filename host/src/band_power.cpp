#include "neurotrace/band_power.h"

#include <cmath>
#include <complex>
#include <stdexcept>
#include <utility>

namespace nt {
namespace {

using cd = std::complex<double>;

// In-place iterative radix-2 FFT. a.size() must be a power of 2.
void fft(std::vector<cd>& a) {
  const size_t n = a.size();
  for (size_t i = 1, j = 0; i < n; ++i) {
    size_t bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) std::swap(a[i], a[j]);
  }
  for (size_t len = 2; len <= n; len <<= 1) {
    const cd wl = std::polar(1.0, -2.0 * M_PI / static_cast<double>(len));
    for (size_t i = 0; i < n; i += len) {
      cd w = 1;
      for (size_t k = 0; k < len / 2; ++k) {
        const cd u = a[i + k], v = a[i + k + len / 2] * w;
        a[i + k] = u + v;
        a[i + k + len / 2] = u - v;
        w *= wl;
      }
    }
  }
}

}  // namespace

BandPowerAnalyzer::BandPowerAnalyzer(double fs, size_t n) : fs_(fs), n_(n), window_(n) {
  if (n == 0 || (n & (n - 1)) != 0) throw std::invalid_argument("segment length must be power of 2");
  windowPower_ = 0;
  for (size_t i = 0; i < n; ++i) {
    window_[i] = 0.5 - 0.5 * std::cos(2.0 * M_PI * i / (n - 1));
    windowPower_ += window_[i] * window_[i];
  }
}

std::vector<double> BandPowerAnalyzer::psd(const std::vector<float>& x) const {
  std::vector<double> out(n_ / 2 + 1, 0.0);
  if (x.size() < n_) return out;

  const size_t hop = n_ / 2;
  size_t segments = 0;
  for (size_t start = 0; start + n_ <= x.size(); start += hop, ++segments) {
    double mean = 0;  // remove DC per segment
    for (size_t i = 0; i < n_; ++i) mean += x[start + i];
    mean /= static_cast<double>(n_);

    std::vector<cd> buf(n_);
    for (size_t i = 0; i < n_; ++i) buf[i] = (x[start + i] - mean) * window_[i];
    fft(buf);

    for (size_t k = 0; k <= n_ / 2; ++k) {
      double p = std::norm(buf[k]) / (fs_ * windowPower_);
      if (k != 0 && k != n_ / 2) p *= 2.0;  // one-sided
      out[k] += p;
    }
  }
  for (auto& p : out) p /= static_cast<double>(segments);
  return out;
}

BandPowers BandPowerAnalyzer::compute(const std::vector<float>& x) const {
  const auto p = psd(x);
  const double df = fs_ / static_cast<double>(n_);
  auto band = [&](double lo, double hi) {
    double sum = 0;
    for (size_t k = 0; k < p.size(); ++k) {
      const double f = k * df;
      if (f >= lo && f < hi) sum += p[k] * df;
    }
    return sum;
  };
  return {band(1, 4), band(4, 8), band(8, 13), band(13, 30)};
}

}  // namespace nt
