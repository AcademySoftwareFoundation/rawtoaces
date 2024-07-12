# https://hub.docker.com/r/aswf/ci-vfxall/tags
FROM aswf/ci-vfxall:2024

# Base Building Dependencies.
RUN yum install --setopt=tsflags=nodocs -y \
    blas-devel \
    lapack-devel \
    eigen3-devel \
    gflags-devel \
    glog-devel \
    suitesparse-devel \
    && ln -s /usr/include/eigen3/Eigen /usr/include/Eigen

# aces-container
WORKDIR /tmp
RUN git clone https://github.com/ampas/aces_container.git \
    && mkdir aces_container/build && cd aces_container/build \
    && cmake .. && make -j 4 && make install

# ceres-solver
# Compiling "ceres-solver" requires a significant amount of memory, well over
# the 2Go that Docker allocates by default, thus you will need to increase the
# memory in Docker preferences: Preferences --> Resources --> Advanced, 8Go
# should be enough.
WORKDIR /tmp
RUN curl -O http://ceres-solver.org/ceres-solver-1.14.0.tar.gz \
    && tar zxf ceres-solver-1.14.0.tar.gz \
    && mkdir ceres-solver-1.14.0/build && cd ceres-solver-1.14.0/build \
    && cmake .. && make -j 4 && make install

# libraw
WORKDIR /tmp
RUN git clone https://github.com/LibRaw/LibRaw.git \
    && cd LibRaw && git checkout 0.21.1 \
    && autoreconf --install \
    && ./configure && make -j 4 && make install

ARG CACHE_DATE

RUN mkdir -p /home/aswf/rawtoaces
WORKDIR /home/aswf/rawtoaces
COPY . /home/aswf/rawtoaces
RUN mkdir build && cd build \
    && cmake .. && make -j 4 && make install
