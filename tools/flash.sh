#!/usr/bin/env bash
# Flash both finder boards with one command, on HaLjr (Linux):
#   bash ~/Projects/hidden-device-hunter/tools/flash.sh        both boards (C5 first, then R4)
#   bash ~/Projects/hidden-device-hunter/tools/flash.sh c5     only the C5
#   bash ~/Projects/hidden-device-hunter/tools/flash.sh r4     only the R4
# It gets the latest code from main, then asks you to plug in each board.
# Plug in only one board at a time. The computer must be on home Wi-Fi (not HDH-Finder).
set -u
REPO=~/Projects/hidden-device-hunter
WHICH=$(echo "${1:-both}" | tr "[:upper:]" "[:lower:]")
cd "$REPO" || { echo "No folder $REPO"; exit 1; }

echo "== Getting the latest code (main) =="
if ! git checkout -q main 2>/dev/null; then
  git stash -q && echo "Your local changes were saved with git stash." && git checkout -q main || { echo "Could not switch to main."; exit 1; }
fi
git pull -q || { echo "git pull failed (see the message above). Fix that first, or the boards get old code."; exit 1; }

wait_for() {  # wait_for <command that prints a port> <seconds>; prints the port
  local i
  for ((i = 0; i < $2; i++)); do
    local p
    p=$(eval "$1")
    if [ -n "$p" ]; then echo "$p"; return 0; fi
    sleep 1
  done
  return 1
}

flash_c5() {
  echo
  echo "== C5 (XIAO ESP32-C5) =="
  echo "Unplug the R4. Hold the C5's BOOT button, plug the C5 in, let go of BOOT."
  read -r -p "Press Enter when done... " _
  local port
  port=$(wait_for "ls /dev/serial/by-id/*Espressif* 2>/dev/null | head -1" 15) || {
    echo "The computer cannot see the C5. Check the cable (data cable, pushed in firmly) and run this again."
    return 1
  }
  echo "Found the C5 at $port. Compiling and uploading (about 2 minutes)..."
  if arduino-cli compile --upload -p "$port" --fqbn esp32:esp32:XIAO_ESP32C5:CDCOnBoot=cdc firmware/phase2-xiao-sniffer; then
    echo "C5 done. Unplug it and plug it back in (no BOOT) to start the new code."
    return 0
  fi
  echo "Normal upload failed; trying the slow method."
  echo "Hold BOOT, tap RESET on the C5, let go of BOOT."
  read -r -p "Press Enter when done... " _
  port=$(wait_for "ls /dev/serial/by-id/*Espressif* 2>/dev/null | head -1" 15) || { echo "C5 not found."; return 1; }
  local esptool
  esptool=$(ls ~/.arduino15/packages/esp32/tools/esptool_py/*/esptool 2>/dev/null | tail -1)
  [ -n "$esptool" ] || { echo "esptool not found in ~/.arduino15; cannot use the slow method."; return 1; }
  arduino-cli compile --output-dir /tmp/c5 --fqbn esp32:esp32:XIAO_ESP32C5:CDCOnBoot=cdc firmware/phase2-xiao-sniffer &&
    "$esptool" --chip esp32c5 -p "$port" --no-stub write-flash 0x0 /tmp/c5/phase2-xiao-sniffer.ino.merged.bin &&
    { echo "C5 done. Unplug it and plug it back in to start the new code."; return 0; }
  echo "C5 flash failed."
  return 1
}

r4_port() { arduino-cli board list 2>/dev/null | grep unor4wifi | awk '{print $1}' | head -1; }

flash_r4() {
  echo
  echo "== R4 (UNO R4 WiFi) =="
  echo "Unplug the C5. Plug in the R4 on its own."
  read -r -p "Press Enter when done... " _
  local port
  port=$(wait_for r4_port 10) || {
    echo "R4 not found yet. Double-tap the R4's RESET button now."
    port=$(wait_for r4_port 15) || {
      echo "Still no R4. One-time Linux fix: sudo ~/.arduino15/packages/arduino/hardware/renesas_uno/*/post_install.sh"
      echo "Then unplug and replug the R4 and run this again."
      return 1
    }
  }
  echo "Found the R4 at $port. Compiling and uploading..."
  if arduino-cli compile --upload -p "$port" --fqbn arduino:renesas_uno:unor4wifi firmware/phase2-r4-display; then
    echo "R4 done."
    return 0
  fi
  echo "Upload failed. Double-tap RESET on the R4, then press Enter to try once more."
  read -r -p "" _
  port=$(wait_for r4_port 15) || { echo "R4 not found."; return 1; }
  arduino-cli compile --upload -p "$port" --fqbn arduino:renesas_uno:unor4wifi firmware/phase2-r4-display && { echo "R4 done."; return 0; }
  echo "R4 flash failed."
  return 1
}

ok=0
case "$WHICH" in
  c5) flash_c5 || ok=1 ;;
  r4) flash_r4 || ok=1 ;;
  both) flash_c5 || ok=1; flash_r4 || ok=1 ;;
  *) echo "Use: flash.sh, flash.sh c5 or flash.sh r4"; exit 1 ;;
esac
echo
if [ $ok -eq 0 ]; then echo "All done. Power both boards from the power bank, and the R4 bars and beeps start."; else echo "Something failed; see the message above."; fi
exit $ok
