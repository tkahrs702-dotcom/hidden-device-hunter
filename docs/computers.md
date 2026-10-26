# Computers: HaL and HaLjr

Both computers keep the project in the same place and follow the same routine, so you can switch between them mid-build.
GitHub (`tkahrs702-dotcom/hidden-device-hunter`, private) is the source of truth; neither computer's copy is.

## Layout (same on both)
```
~/Projects/hidden-device-hunter/     this repo
~/esp/esp-idf-v5.5.5/                ESP-IDF, Phase 3 (P4) only
~/.arduino15/                        arduino-cli config and board cores
```
Inside the repo:
| Folder | Holds |
|---|---|
| `firmware/` | one folder per sketch: `phase1-r4-finder`, `phase2-r4-display`, `phase2-xiao-sniffer`, `phase3-p4-rfcam` (placeholder) |
| `docs/manual/` | the build manual and its diagrams |
| `docs/worklog/` | one file per session: `YYYY-MM-DD-topic.md` |
| `tools/` | `setup.sh` (this page), `make_oui_table.py`, `diagrams/` |
| `build/` | compiler output; git ignores it |

## First time on a computer
```
mkdir -p ~/Projects && cd ~/Projects
git clone git@github.com:tkahrs702-dotcom/hidden-device-hunter.git   # or the https URL
cd hidden-device-hunter
tools/setup.sh
```
`tools/setup.sh` checks the repo location, installs the esp32 board URL and the two cores if missing, checks the `dialout` group,
compiles all three sketches, and lists the boards plugged in. Run it again any time; `tools/setup.sh --check` only reports.
`tools/setup.sh` does not install arduino-cli itself; it stops with a warning if it is missing. Install it without sudo into `~/.local/bin`
(already on PATH), pinned to the tested version:
```
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.local/bin sh -s 1.3.1
```
The esp32 core download is large (roughly 1-2 GB with its toolchains); renesas_uno is much smaller. On a slow link the default download
timeout cuts it off (`context deadline exceeded`); raise it first: `arduino-cli config set network.connection_timeout 1800s`.
If it says `dialout` is missing: `sudo usermod -aG dialout $USER`, log out, log back in.

| | HaL | HaLjr |
|---|---|---|
| arduino-cli + cores | installed 2026-10-05 (arduino-cli 1.3.1 in `~/.local/bin`, renesas_uno 1.6.0, esp32 3.3.12) | installed (arduino-cli 1.3.1, renesas_uno 1.6.0, esp32 3.3.12) |
| ESP-IDF v5.5.5 (Phase 3 only) | not needed until Phase 3 | installed at `~/esp/esp-idf-v5.5.5` |
| Other projects' boards also plugged in | possibly | possibly |

## Every session
1. `cd ~/Projects/hidden-device-hunter && git pull`
2. Read the README's "Where we left off" (state and next step).
3. Plug in boards, then `arduino-cli board list`.
4. Work. Log what you did and what you saw in `docs/worklog/YYYY-MM-DD-topic.md`; update the README's "Where we left off" if the state or next step changed.
5. `git add -A && git commit -m "..." && git push` before switching computers.

## Which board is which
`ttyACM0/1/2` numbers change with plug-in order. Identify boards by serial number:
```
arduino-cli board list                      # port, board name, FQBN
udevadm info -q property -n /dev/ttyACM0 | grep -E 'ID_SERIAL=|ID_MODEL='
ls -l /dev/serial/by-id/                    # stable names that include the serial number
```
| Board | Shows as | FQBN |
|---|---|---|
| Uno R4 WiFi | `Arduino UNO R4 WiFi` | `arduino:renesas_uno:unor4wifi` |
| XIAO ESP32-S3 | Espressif USB JTAG/serial (unknown board in the list is normal) | `esp32:esp32:XIAO_ESP32S3` |
| ESP32-P4 (this project's, Phase 3) | CH343 serial | ESP-IDF, not arduino-cli |
Write each board's serial number into the table below the first time you plug it in, then flash by `/dev/serial/by-id/...` path
instead of `ttyACM*`. Do not flash another project's board by mistake: if a port's serial number is not in this table, unplug it.

| Board | Serial number | Noted on |
|---|---|---|
| Uno R4 WiFi | (fill in) | |
| XIAO ESP32-S3 | (fill in) | |
| ESP32-P4 (this project) | (fill in) | |

## Flash commands (copy and fill in the port)
```
cd ~/Projects/hidden-device-hunter
# Phase 1, R4
arduino-cli upload --fqbn arduino:renesas_uno:unor4wifi -p <R4 port> firmware/phase1-r4-finder
# Phase 2, R4 and XIAO
arduino-cli upload --fqbn arduino:renesas_uno:unor4wifi -p <R4 port>   firmware/phase2-r4-display
arduino-cli upload --fqbn esp32:esp32:XIAO_ESP32S3      -p <XIAO port> firmware/phase2-xiao-sniffer
# Serial monitor
arduino-cli monitor -p <port> -c baudrate=115200
```
`upload` compiles first when needed. The manual's phase chapters say what to expect after each one.
