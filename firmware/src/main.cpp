// NeuroTrace firmware: ADS1299 -> ESP32 -> USB serial (BLE: see TODO).
#include <Arduino.h>
#include <SPI.h>
#include "ads1299.h"
#include "neurotrace_protocol.h"

constexpr bool kUseTestSignal = true;  // set false once real electrodes are connected

static SPIClass vspi(VSPI);                       // SCK 18, MISO 19, MOSI 23
static ADS1299 adc(vspi, {/*cs*/ 5, /*drdy*/ 4, /*start*/ 16, /*reset*/ 17});

static volatile bool g_drdy = false;
static void IRAM_ATTR onDrdy() { g_drdy = true; }

void setup() {
  Serial.begin(921600);
  vspi.begin(18, 19, 23, 5);

  if (!adc.begin(kUseTestSignal)) {
    // Blink the built-in LED forever: wiring or power problem.
    pinMode(LED_BUILTIN, OUTPUT);
    for (;;) { digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); delay(150); }
  }
  attachInterrupt(digitalPinToInterrupt(4), onDrdy, FALLING);
  adc.startStreaming();
}

void loop() {
  static uint8_t counter = 0;
  if (!g_drdy) return;
  g_drdy = false;

  int32_t raw[nt::kNumChannels];
  adc.readFrame(raw, nt::kNumChannels);

  uint8_t frame[nt::kFrameSize];
  nt::encodeFrame(counter++, raw, frame);
  Serial.write(frame, sizeof(frame));
  // TODO(BLE): push `frame` into a BLE notify characteristic (batch several frames per packet).
  // TODO(safety): battery only; use an isolated USB link while electrodes are attached.
}
