#!/bin/sh

# prefer cmake3 over "regular" cmake (cmake == cmake2 on RHEL)
if command -v cmake3 >/dev/null 2>&1 ; then
  CMakeCommand="cmake3"
else
  CMakeCommand="cmake"
fi

mkdir build
cd build
$CMakeCommand -DCMAKE_BUILD_TYPE=$CAF_BUILD_TYPE ..
$CMakeCommand --build . --parallel $CAF_NUM_CORES
