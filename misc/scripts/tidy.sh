#!/bin/bash

# Usage: ../misc/scripts/tidy.sh [OPTIONS...]
#
# Run clang-tidy in parallel.
#
# The -fix switch causes sometimes problems, so don't use it.

F='Alenka/src|Alenka/unit-test/src|include/AlenkaFile|include/AlenkaSignal'

../misc/scripts/all-sources.sh | xargs -n1 -P4 clang-tidy -header-filter=$F $@

