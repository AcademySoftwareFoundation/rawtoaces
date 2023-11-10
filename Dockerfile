FROM ubuntu:bionic

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get -y install --no-install-recommends apt-utils dialog 2>&1 \
    && apt-get -y install curl git  \
    && apt-get -y install cmake build-essential libilmbase-dev libraw-dev libboost-all-dev libgoogle-glog-dev libatlas-base-dev libeigen3-dev libsuitesparse-dev \
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

RUN cd ~ && git clone https://github.com/ampas/aces_container.git \
    && mkdir aces_container/build && cd aces_container/build \
    && cmake .. && make -j 4 && make install

# Compiling "ceres-solver" requires a significant amount of memory, well over
# the 2Go that Docker allocates by default, thus you will need to increase the
# memory in Docker preferences: Preferences --> Resources --> Advanced, 8Go
# should be enough.
RUN cd ~ && curl -O http://ceres-solver.org/ceres-solver-1.14.0.tar.gz \
    && tar zxf ceres-solver-1.14.0.tar.gz \
    && mkdir ceres-solver-1.14.0/build && cd ceres-solver-1.14.0/build \
    && cmake .. && make -j 4 && make install

RUN cd ~ && git clone https://github.com/ampas/rawtoaces \
    && mkdir rawtoaces/build && cd rawtoaces/build \
    && cmake .. && make -j 4 && make install
