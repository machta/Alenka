#!/bin/bash

# Usage: ./misc/scripts/format.sh
#
# Format all source files with clang-format.

clang-format -i `./misc/scripts/all-sources.sh` `./misc/scripts/all-headers.sh`

