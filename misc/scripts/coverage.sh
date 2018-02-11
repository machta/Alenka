#!/bin/bash

# Usage: ../Alenka/misc/scripts/coverage.sh
#
# Must be be executed from one dir above Alenka root dir (because of the
# hardcoded paths).
#
# To clean all coverage data run:
#   rm `find .. -name *.gcda`

BUILD_DIR=build-Debug

# Remove the report from the last run
#rm moc_*.gcda
rm -r testCovRep.info testCovRep

# Generate the coverage data file.
lcov -t "Report" -o testCovRep.info -c -d ../Alenka/$BUILD_DIR/

# Remove external code from the data.
lcov -r testCovRep.info '*moc_*' -o $PWD/testCovRep.info
lcov -r testCovRep.info '*qrc_*.cpp' -o $PWD/testCovRep.info
# lcov -r testCovRep.info '*resources*' -o $PWD/testCovRep.info

lcov -r testCovRep.info 'unit-test/*' -o $PWD/testCovRep.info
# lcov -r testCovRep.info '*include*' -o $PWD/testCovRep.info

lcov -r testCovRep.info '/usr/*' -o $PWD/testCovRep.info
lcov -r testCovRep.info '/opt/*' -o $PWD/testCovRep.info

lcov -r testCovRep.info 'libraries/boost*' -o $PWD/testCovRep.info
lcov -r testCovRep.info 'libraries/google*' -o $PWD/testCovRep.info
lcov -r testCovRep.info 'libraries/EDFlib*' -o $PWD/testCovRep.info
lcov -r testCovRep.info 'libraries/alglib*' -o $PWD/testCovRep.info
lcov -r testCovRep.info 'libraries/eigen*' -o $PWD/testCovRep.info
lcov -r testCovRep.info 'libraries/clFFT*' -o $PWD/testCovRep.info
lcov -r testCovRep.info 'libraries/libsamplerate*' -o $PWD/testCovRep.info

# Generate the HTML report.
genhtml -o testCovRep testCovRep.info
