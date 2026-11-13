#pragma once
#include <Arduino.h>
#include <SPI.h>

// Minimal ADS1299 driver (RDATAC mode, 250 SPS, gain 24).
// Always verify register values against the datasheet (SBAS499).
class ADS1299 {
 public:
  struct Pins { int cs, drdy, start, reset; };
  static constexpr int kChannels = 8;

  ADS1299(SPIClass& spi, Pins pins) : spi_(spi), p_(pins) {}

  // Resets the chip and configures channels 1-2 (rest powered down).
  // useTestSignal = true routes the internal square wave to the inputs (bring-up).
  bool begin(bool useTestSignal = false);
  void startStreaming();
  void stopStreaming();
  // Reads one frame (call after DRDY went low). Fills the first `n` channels.
  void readFrame(int32_t* ch, int n);

  uint8_t readRegister(uint8_t addr);
  void writeRegister(uint8_t addr, uint8_t value);

 private:
  enum Cmd : uint8_t {
    WAKEUP = 0x02, STANDBY = 0x04, RESET = 0x06, START = 0x08, STOP = 0x0A,
    RDATAC = 0x10, SDATAC = 0x11, RDATA = 0x12, RREG = 0x20, WREG = 0x40
  };
  enum Reg : uint8_t {
    ID = 0x00, CONFIG1 = 0x01, CONFIG2 = 0x02, CONFIG3 = 0x03, LOFF = 0x04,
    CH1SET = 0x05, BIAS_SENSP = 0x0D, BIAS_SENSN = 0x0E
  };

  void command(uint8_t cmd);
  SPIClass& spi_;
  Pins p_;
  SPISettings settings_{4000000, MSBFIRST, SPI_MODE1};  // ADS1299 uses CPOL=0, CPHA=1
};
