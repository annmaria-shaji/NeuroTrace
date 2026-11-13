#include "ads1299.h"
#include "neurotrace_protocol.h"

void ADS1299::command(uint8_t cmd) {
  spi_.beginTransaction(settings_);
  digitalWrite(p_.cs, LOW);
  spi_.transfer(cmd);
  delayMicroseconds(4);
  digitalWrite(p_.cs, HIGH);
  spi_.endTransaction();
}

uint8_t ADS1299::readRegister(uint8_t addr) {
  spi_.beginTransaction(settings_);
  digitalWrite(p_.cs, LOW);
  spi_.transfer(RREG | addr);
  spi_.transfer(0x00);  // read 1 register
  const uint8_t v = spi_.transfer(0x00);
  digitalWrite(p_.cs, HIGH);
  spi_.endTransaction();
  return v;
}

void ADS1299::writeRegister(uint8_t addr, uint8_t value) {
  spi_.beginTransaction(settings_);
  digitalWrite(p_.cs, LOW);
  spi_.transfer(WREG | addr);
  spi_.transfer(0x00);  // write 1 register
  spi_.transfer(value);
  digitalWrite(p_.cs, HIGH);
  spi_.endTransaction();
}

bool ADS1299::begin(bool useTestSignal) {
  pinMode(p_.cs, OUTPUT);    digitalWrite(p_.cs, HIGH);
  pinMode(p_.start, OUTPUT); digitalWrite(p_.start, LOW);
  pinMode(p_.reset, OUTPUT); digitalWrite(p_.reset, HIGH);
  pinMode(p_.drdy, INPUT);

  delay(200);                                    // power-up settling
  digitalWrite(p_.reset, LOW);  delayMicroseconds(10);
  digitalWrite(p_.reset, HIGH); delay(1);
  command(SDATAC);                               // registers can only be written in SDATAC

  const uint8_t id = readRegister(ID);           // check datasheet for the expected ID value
  if (id == 0x00 || id == 0xFF) return false;    // no chip / bus problem

  writeRegister(CONFIG1, 0x96);                  // 250 SPS, internal clock
  writeRegister(CONFIG2, useTestSignal ? 0xD0 : 0xC0);  // 0xD0: internal test signal on
  writeRegister(CONFIG3, 0xEC);                  // internal reference + bias buffer on (verify!)

  const uint8_t chCfg = useTestSignal ? 0x65 : 0x60;    // gain 24; MUX = test signal / normal
  writeRegister(CH1SET + 0, chCfg);
  writeRegister(CH1SET + 1, chCfg);
  for (int c = 2; c < kChannels; ++c) writeRegister(CH1SET + c, 0x81);  // power down, input shorted

  writeRegister(BIAS_SENSP, 0x03);               // drive-right-leg from channels 1-2
  writeRegister(BIAS_SENSN, 0x03);
  return true;
}

void ADS1299::startStreaming() {
  command(RDATAC);
  digitalWrite(p_.start, HIGH);
}

void ADS1299::stopStreaming() {
  digitalWrite(p_.start, LOW);
  command(SDATAC);
}

void ADS1299::readFrame(int32_t* ch, int n) {
  uint8_t buf[3 + 3 * kChannels];
  spi_.beginTransaction(settings_);
  digitalWrite(p_.cs, LOW);
  for (auto& b : buf) b = spi_.transfer(0x00);
  digitalWrite(p_.cs, HIGH);
  spi_.endTransaction();
  // buf[0..2] = status word (lead-off bits), then 3 bytes per channel
  for (int i = 0; i < n && i < kChannels; ++i) {
    const uint32_t v = (uint32_t(buf[3 + 3 * i]) << 16) | (uint32_t(buf[4 + 3 * i]) << 8) | buf[5 + 3 * i];
    ch[i] = nt::signExtend24(v);
  }
}
