FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive \
    DISPLAY=:99 \
    QT_QPA_PLATFORM=xcb

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git pkg-config \
    qt6-base-dev qt6-tools-dev \
    libraw-dev libceres-dev libeigen3-dev libsuitesparse-dev \
    libgflags-dev libgoogle-glog-dev \
    libboost-dev libboost-system-dev libboost-test-dev \
    libopenimageio-dev \
    xauth x11vnc xvfb openbox novnc websockify \
    ca-certificates curl \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/rawtoaces
COPY . /opt/rawtoaces

# Build rawtoaces CLI and GUI
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRAWTOACES_BUILD_GUI=ON \
 && cmake --build build --target all -j$(nproc) \
 && cmake --install build

# Simple startup: Xvfb + window manager + VNC + noVNC + app
COPY gui/docker/start.sh /usr/local/bin/start.sh
RUN chmod +x /usr/local/bin/start.sh

EXPOSE 6080
VOLUME ["/data"]

CMD ["/usr/local/bin/start.sh"]
