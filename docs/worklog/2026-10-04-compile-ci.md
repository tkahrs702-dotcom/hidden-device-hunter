# 2026-10-04 compile check in CI
Added `.github/workflows/compile.yml`: on every PR and every push to main, GitHub Actions installs arduino-cli 1.3.1 with
arduino:renesas_uno 1.6.0 and esp32:esp32 3.3.12 (same as HaLjr) and compiles:
- `firmware/phase1-r4-finder` and `firmware/phase2-r4-display` for `arduino:renesas_uno:unor4wifi`
- `firmware/phase2-xiao-sniffer` for `esp32:esp32:XIAO_ESP32S3`

Each sketch is its own job, so one failure doesn't hide the others. Board cores are cached between runs.
Could not run the compile inside the Claude cloud session (its network policy blocks downloads.arduino.cc),
so the first GitHub run is the test. Phase 3 has no firmware yet; add it to the matrix when it does.
