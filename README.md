# NeuroTrace: battery-powered 2-channel EEG (ADS1299 + ESP32)

```
shared/    wire protocol (used by firmware and host)
firmware/  ESP32 (PlatformIO, Arduino framework): ADS1299 driver, USB streaming
host/      C++17 app: filters, Welch band power, blink detection, simulator, tests
```

## Host (no hardware needed)
    cd host && cmake -B build && cmake --build build
    ./build/test_dsp
    ./build/neurotrace_host --sim

## Firmware
    cd firmware && pio run -t upload
Start with `kUseTestSignal = true` (internal square wave) to verify the chain,
then switch to real electrodes.

## Safety
Battery power only. Use an isolated USB link while electrodes are attached.
Research/hobby device, not for diagnosis.

## Roadmap
- [ ] BLE transport (firmware TODO)
- [ ] Dropped-frame counter via the packet counter
- [ ] Visualisation (Dear ImGui/ImPlot, or Unreal/Unity via UDP/OSC)
- [ ] Lead-off detection from the ADS1299 status bits
