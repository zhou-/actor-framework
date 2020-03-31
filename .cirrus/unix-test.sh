#!/bin/sh

# prefer cmake3 over "regular" cmake (cmake == cmake2 on RHEL)
if command -v ctest3 >/dev/null 2>&1 ; then
  CTestCommand="ctest3"
else
  CTestCommand="ctest"
fi

cd build
$CTestCommand --output-on-failure
