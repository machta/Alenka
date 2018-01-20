#!/bin/bash

# Usage: ./misc/scripts/format.sh
#
# Format all source files with clang-format.
#
# Note: As this formats all of the source files program-wide, you should only
#       use this with clang-format 6. Other versions are OK for small pieces
#       of code. TODO: Add a warning/error when used with a bad version.

clang-format -i `./misc/scripts/all-sources.sh` `./misc/scripts/all-headers.sh`

