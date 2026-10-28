# Phase 3 plan: painless phone access (2026-10-28)

Goal (Taelor): the phone shows the finder without a slow R4 Wi-Fi server, a file hunt, or browser blocking.
Gemini's summary agrees with what we found: `tools/finder/finder.html` already streams the same lines
(`L U T S B D M`) over Bluetooth from the C5 and sends `K U V M` back. What is missing is a painless way to open it.

## Why opening it is the hard part
Web Bluetooth only works in Chrome on Android, on a page from https (or a local file). It does not work in Brave by
default, inside the claude.ai artifact frame, or from a raw GitHub link (the repo is private, and raw links are plain text).

## Options for delivery
1. Host the page on https as a public GitHub Pages site (a small separate repo holding only the page, no secrets in it),
   then "Add to Home screen" in Chrome so it opens like an app, offline after the first load. Needs Taelor's OK because it publishes the page.
2. Keep a saved file on the phone and open it from Downloads in Chrome. No hosting, but manual.
3. Skip the phone: the C5 plus R4 show bars, a peak dot and beeps by themselves (already works); the R4 Wi-Fi page stays as a backup.

Recommended: option 1, with option 3 as the fallback that always works.

## Done so far
- Page reconnects by itself (retry every 2 s, and Chrome can offer back the paired finder), keeps the screen on, works in Demo with the mode switch. Tested in headless Chrome demo mode only.

## Rewrite and review steps once delivery is chosen
- Page: one file, offline-capable (manifest + service-worker-free "add to home screen" is enough), reconnect automatically,
  big targets, a Wi-Fi / Bluetooth / Both switch, tap a device to track, volume.
- C5: keep the BLE link as is; confirm BLE and Wi-Fi sniffing together on hardware before trusting "Both".
- Code review of both sketches and the page after the rewrite; fix what it finds before asking Taelor to flash.
- Hardware checks Taelor can do without a monitor: page shows "Receiving from the finder", device list fills, tap Track locks and the R4 beeps.
