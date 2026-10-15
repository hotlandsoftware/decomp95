# Sourced by build scripts: headless private Wine environment for the MSVC tools.
# $ROOT must be set.  winpath converts a Unix path to Z:\... form.
winpath() { printf 'Z:%s' "$1" | tr '/' '\\'; }
export WINEPREFIX="$ROOT/build/wineprefix"
export WINEDEBUG=-all
export WINEDLLOVERRIDES="winex11.drv,winewayland.drv,winemenubuilder.exe,mscoree,mshtml="
unset DISPLAY WAYLAND_DISPLAY
export WINEPATH="$(winpath "$ROOT/msvc/BIN")"
export INCLUDE="$(winpath "$ROOT/msvc/INCLUDE")"
export LIB="$(winpath "$ROOT/msvc/LIB")"
