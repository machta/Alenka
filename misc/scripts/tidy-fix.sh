#!/bin/bash

# Usage: ../misc/scripts/tidy-fix.sh [OPTIONS...]
#
# Run clang-tidy and apply fixes.
#
# This takes a lot longer than just tidy, but it should avoid weird errors when
# the same fix was applied multiple times.

F='Alenka/src|Alenka/unit-test/src|include/AlenkaFile|include/AlenkaSignal'

../misc/scripts/all-sources.sh | xargs -n1 clang-tidy -fix -header-filter=$F $@

