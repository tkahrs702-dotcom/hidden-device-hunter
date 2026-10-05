#!/usr/bin/env bash
# Hidden Device Hunter: set up (or re-check) a computer for building and flashing. Safe to run again.
# Used on HaL and HaLjr (Linux). Usage, from the repo root:  tools/setup.sh          (setup + compile check)
#                                                             tools/setup.sh --check  (report only, change nothing)
# Tested toolchain (HANDOFF.md): arduino-cli 1.3.1, arduino:renesas_uno 1.6.0, esp32:esp32 3.3.12.
set -euo pipefail

CHECK_ONLY=0
[[ "${1:-}" == "--check" ]] && CHECK_ONLY=1
ESP32_URL="https://espressif.github.io/arduino-esp32/package_esp32_index.json"
REPO="$(cd "$(dirname "$0")/.." && pwd)"
ok()   { printf '  ok    %s\n' "$*"; }
warn() { printf '  WARN  %s\n' "$*"; }
step() { printf '\n== %s\n' "$*"; }

step "Computer: $(hostname)"
[[ "$REPO" == "$HOME/Projects/hidden-device-hunter" ]] && ok "repo at ~/Projects/hidden-device-hunter" \
  || warn "repo is at $REPO; the docs assume ~/Projects/hidden-device-hunter"
git -C "$REPO" fetch -q origin 2>/dev/null && {
  behind=$(git -C "$REPO" rev-list --count HEAD..@{u} 2>/dev/null || echo "?")
  [[ "$behind" == "0" ]] && ok "git up to date with origin" || warn "git is $behind commit(s) behind origin: run git pull"
} || warn "could not reach GitHub (offline?)"

step "arduino-cli"
if ! command -v arduino-cli >/dev/null; then
  warn "arduino-cli not installed. Install: https://arduino.github.io/arduino-cli/latest/installation/ then re-run."
  exit 1
fi
ok "$(arduino-cli version | head -1)"

step "Board package URL and cores"
# Creates ~/.arduino15/arduino-cli.yaml the first time; fails harmlessly when it already exists.
[[ $CHECK_ONLY == 0 ]] && { arduino-cli config init >/dev/null 2>&1 || true; }
if arduino-cli config dump | grep -q "$ESP32_URL"; then ok "esp32 board URL configured"
elif [[ $CHECK_ONLY == 0 ]]; then arduino-cli config add board_manager.additional_urls "$ESP32_URL"; ok "added esp32 board URL"
else warn "esp32 board URL missing"; fi
if [[ $CHECK_ONLY == 0 ]]; then arduino-cli core update-index >/dev/null; fi
for core in arduino:renesas_uno esp32:esp32; do
  v=$(arduino-cli core list | awk -v c="$core" '$1==c {print $2}')
  if [[ -n "$v" ]]; then ok "$core $v"
  elif [[ $CHECK_ONLY == 0 ]]; then arduino-cli core install "$core"; ok "installed $core"
  else warn "$core not installed"; fi
done

step "Serial port permission"
if id -nG | grep -qw dialout; then ok "$USER is in dialout"
elif id -nG "$USER" | grep -qw dialout; then warn "$USER was added to dialout but this login predates it: log out and back in"
else warn "$USER is not in dialout: sudo usermod -aG dialout $USER, then log out and back in"; fi

if [[ $CHECK_ONLY == 0 ]]; then
  step "Compile check (output in build/, which git ignores)"
  for s in "arduino:renesas_uno:unor4wifi firmware/phase1-r4-finder" \
           "arduino:renesas_uno:unor4wifi firmware/phase2-r4-display" \
           "esp32:esp32:XIAO_ESP32S3 firmware/phase2-xiao-sniffer"; do
    set -- $s
    if arduino-cli compile --fqbn "$1" --build-path "$REPO/build/$(basename "$2")" "$REPO/$2" >/dev/null 2>&1; then
      ok "$2"
    else warn "$2 failed to compile: run  arduino-cli compile --fqbn $1 $2  to see why"; fi
  done
fi

step "Boards plugged in now (match by serial number, not ttyACM order)"
arduino-cli board list || true
echo
echo "Next: docs/computers.md (which board is which), then the phase you are on in docs/manual/."
