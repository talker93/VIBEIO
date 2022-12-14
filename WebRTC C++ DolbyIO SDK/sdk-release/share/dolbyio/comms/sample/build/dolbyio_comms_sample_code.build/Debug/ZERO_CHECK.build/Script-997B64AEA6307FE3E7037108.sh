#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/shanjiang/Desktop/dolbyio_test/sdk-release/share/dolbyio/comms/sample/build
  make -f /Users/shanjiang/Desktop/dolbyio_test/sdk-release/share/dolbyio/comms/sample/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/shanjiang/Desktop/dolbyio_test/sdk-release/share/dolbyio/comms/sample/build
  make -f /Users/shanjiang/Desktop/dolbyio_test/sdk-release/share/dolbyio/comms/sample/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/shanjiang/Desktop/dolbyio_test/sdk-release/share/dolbyio/comms/sample/build
  make -f /Users/shanjiang/Desktop/dolbyio_test/sdk-release/share/dolbyio/comms/sample/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/shanjiang/Desktop/dolbyio_test/sdk-release/share/dolbyio/comms/sample/build
  make -f /Users/shanjiang/Desktop/dolbyio_test/sdk-release/share/dolbyio/comms/sample/build/CMakeScripts/ReRunCMake.make
fi

