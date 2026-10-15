#!/bin/sh
# Usage: tools/dosrun.sh <dir-relative-to-repo> "<dos command>" ["<dos command>" ...]
# Runs DOS commands in DOSBox fully headless (no window, no audio, 120 s limit)
# with the repository mounted as C:.  Output goes to DOSRUN.LOG in <dir>.
ROOT=$(cd "$(dirname "$0")/.." && pwd)
DIR=$1; shift
mkdir -p "$ROOT/build" "$ROOT/$DIR"
CONF="$ROOT/build/dosbox.conf"
printf '[sdl]\nfullscreen=false\noutput=surface\n[dosbox]\nmemsize=32\n[cpu]\ncycles=max\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n' > "$CONF"
BAT="$ROOT/$DIR/DOSRUN.BAT"
{
    echo '@echo off'
    for c in "$@"; do printf '%s >> DOSRUN.LOG\r\n' "$c"; done
} > "$BAT"
rm -f "$ROOT/$DIR/DOSRUN.LOG"
DOSDIR=$(printf '%s' "$DIR" | tr '/' '\\')
env -u DISPLAY -u WAYLAND_DISPLAY SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    timeout 120 dosbox -conf "$CONF" -c "mount c \"$ROOT\"" -c "c:" -c "cd \\$DOSDIR" \
    -c "call DOSRUN.BAT" -c "exit" >/dev/null 2>&1
rc=$?
rm -f "$BAT"
[ -f "$ROOT/$DIR/DOSRUN.LOG" ] && tr -d '\r' < "$ROOT/$DIR/DOSRUN.LOG"
exit $rc
