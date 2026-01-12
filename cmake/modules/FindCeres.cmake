#
# This is a hack to make rawtoaces build on Centos 7 on CI.
# ceres-solver-1.12 which is available via yum on Centos 7 comes with
# broken Config.cmake file. This hack works around that. It requires that
# ceres-solver-1.12 is already installed via yum.
#

set (Ceres_VERSION_MAJOR 0)
set (CERES_INCLUDE_DIRS "/usr/include")
set (CERES_LIBRARIES "/usr/lib64/libceres.so")
set (Ceres_FOUND 1)
