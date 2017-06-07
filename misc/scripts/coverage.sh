#!/bin/bash

# Usage: ../Alenka/misc/scripts/coverage.sh
#
# 

# rm `find .. -name *.gcda`

rm moc_*.gcda
rm -r test.info test-html

lcov -t "test" -o test.info -c \
  -d . \
  -d ../Alenka/Alenka-Signal/build-Debug-64/CMakeFiles/alenka-signal.dir/src \
  -d ../Alenka/Alenka-Signal/build-Debug/CMakeFiles/alenka-signal.dir/src \
  -d ../Alenka/Alenka-File/build-Debug-64/CMakeFiles/alenka-file.dir/src \
  -d ../Alenka/Alenka-File/build-Debug/CMakeFiles/alenka-file.dir/src

#lcov -r test.info '*moc_*' -o $PWD/test.info

#lcov -r test.info '*include*' -o $PWD/test.info

lcov -r test.info '*resources*' -o $PWD/test.info
lcov -r test.info '*boost*' -o $PWD/test.info
lcov -r test.info '*google*' -o $PWD/test.info
lcov -r test.info '/usr/*' -o $PWD/test.info
lcov -r test.info '/opt/*' -o $PWD/test.info

lcov -r test.info '*EDFlib*' -o $PWD/test.info
lcov -r test.info '*alglib*' -o $PWD/test.info
lcov -r test.info '*eigen*' -o $PWD/test.info
lcov -r test.info '*clFFT.h*' -o $PWD/test.info
lcov -r test.info '*libgdf*' -o $PWD/test.info

genhtml -o test-html test.info

