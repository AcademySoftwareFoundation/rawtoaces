#!/usr/bin/env bash
set -e

export DISPLAY=:99

# Start virtual framebuffer
Xvfb :99 -screen 0 1920x1080x24 &

# Start a minimal window manager
openbox &

# Start x11vnc and noVNC
x11vnc -display :99 -nopw -forever -shared -rfbport 5900 &
websockify --web=/usr/share/novnc/ 6080 localhost:5900 &

# Launch GUI
if command -v rawtoaces-gui >/dev/null 2>&1; then
  rawtoaces-gui &
else
  /opt/rawtoaces/build/gui/rawtoaces-gui &
fi

# Keep container running
wait -n
