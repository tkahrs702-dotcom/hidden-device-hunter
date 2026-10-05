#!/bin/sh
# Render every SVG in docs/manual/diagrams to PNG for eyeballing (needs Chromium). Usage: render.sh [outdir]
OUT=${1:-/tmp/hdh-render}
mkdir -p "$OUT"
CHROME=${CHROME:-$(ls /opt/pw-browsers/chromium_headless_shell-*/*/headless_shell | head -1)}
for f in "$(dirname "$0")"/../../docs/manual/diagrams/*.svg; do
  w=$(grep -o 'width="[0-9]*"' "$f" | head -1 | tr -dc 0-9)
  h=$(grep -o 'height="[0-9]*"' "$f" | head -1 | tr -dc 0-9)
  html="$OUT/$(basename "$f" .svg).html"
  printf '<html><body style="margin:0"><img src="file://%s"></body></html>' "$(realpath "$f")" > "$html"
  "$CHROME" --no-sandbox --disable-gpu --hide-scrollbars --window-size="$w,$h" \
    --screenshot="$OUT/$(basename "$f" .svg).png" "file://$html" >/dev/null 2>&1
  rm -f "$html"
done
ls "$OUT"
