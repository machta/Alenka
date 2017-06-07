#!/bin/bash

# Usage: ./misc/scripts/all-headers.sh
#
# Print all source file names to stdout.

DIR=`dirname $0`/../..

find $DIR/src -type f -name '*.h'
find $DIR/unit-test -type f -name '*.h'

find $DIR/Alenka-File/src -type f -name '*.h'
find $DIR/Alenka-File/include -type f -name '*.h'

find $DIR/Alenka-Signal/src -type f -name '*.h'
find $DIR/Alenka-Signal/include -type f -name '*.h'

