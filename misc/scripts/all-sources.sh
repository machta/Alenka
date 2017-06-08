#!/bin/bash

# Usage: ./misc/scripts/all-sources.sh
#
# Print all source file names to stdout.

function f {
  DIR=`dirname $0`/../..

  find $DIR/src -type f -name '*.cpp'
  find $DIR/unit-test -type f -name '*.cpp'

  find $DIR/Alenka-File/src -type f -name '*.cpp'
  find $DIR/Alenka-Signal/src -type f -name '*.cpp'
}

realpath --relative-to=. `f`

