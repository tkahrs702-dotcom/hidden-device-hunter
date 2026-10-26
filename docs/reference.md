# Reference: early decisions and notes

Moved from the old `HANDOFF.md` on 2026-10-26. Current state and next step are in the README's "Where we left off".

## Decisions made on copy (2026-10-03)
- Level shifting: the Build Plan said a 1k+2k divider; the Setup Guide (later, more detailed) said an Adafruit 757 shifter. The shifter is used everywhere (also needed for the two-way UART).
- Cost: the Setup Guide's $95-145 (checked prices) over the Build Plan's $95-160.
- Toolchain: HaLjr has ESP-IDF v5.5.5 and arduino-cli; `tools/setup.sh` checks them. The guides' generic install steps are only needed for the Arduino IDE.
- 2026-10-26: this project is fully separate from SENTINEL. Earlier plans to reuse SENTINEL's P4, TTGO/T3 boards or its R4 scanner are dropped.

## Claude app copy
Claude app Project "Hidden Device Hunter" (https://claude.ai/code/project/chan_01D93rrNRNTZUUnDzP1evSvQ): goal and status in its instructions, repo attached; refresh the instructions from this repo when the state changes. Status doc: https://claude.ai/artifact/JRZNRaVzCsuKZQ8Rz8ohqj. Companion docs: Build Plan https://claude.ai/artifact/PuYyKuxAEt5E7tMc9CCF2e and Setup Guide https://claude.ai/artifact/QAQKwawgR31SS5VzCug1D1 (their in-body titles may still show the old name). The repo is the source of truth.

## Manual
Full manual in `docs/manual/` (written 2026-10-03, rebuilt 2026-10-05 as a step-by-step build guide): parts, SVG wiring diagram and schematic per phase (`docs/manual/diagrams/`, made by `tools/diagrams/make_diagrams.py`), wire list with IDs (W/P/S/A/T/I/O), assembly, meter checks before power, flashing with arduino-cli, use, troubleshooting. Phase 2 wiring and schematic: `docs/manual/phase2-xiao-c5-wiring.svg` and `docs/manual/phase2-xiao-c5-schematic.svg` (2026-10-20 worklog).
2026-10-05 build decisions: a half-size breadboard holds the Phase 1 buzzer and spreads Phase 2's 5 V (the R4 has one 5V pin); the jumper kit needs M-M, M-F and F-F; a generic BSS138 shifter (LV1-4/HV1-4) is mapped to the Adafruit 757 (A/B) in 02 sec. 2.2; Phase 3 GND pins swapped (XIAO GND -> P4 pin 5, BNO085 GND -> pin 10).

## Older open items
- The Setup Guide's sources list had stale links (7-DSI-TOUCH-A, RPi Camera B); dropped in this copy.
- HaL and HaLjr setup: `docs/computers.md`; `tools/setup.sh` sets up or checks either one.
