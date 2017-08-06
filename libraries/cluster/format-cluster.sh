#!/bin/bash

# Usage: ./format-cluster.sh
#
# Removes the first and the last comment block, and formats the code.

function removeCpp {
  TMP1=`mktemp`
  TMP2=`mktemp`

  awk -f removeFirstComment.awk $1 > $TMP1

  tac $TMP1 > $TMP2
  awk -f removeFirstComment.awk $TMP2 > $TMP1
  tac $TMP1 > $1

  rm $TMP1 $TMP2
}

function removeC {
  perl -0777 -i -pe 's/^\/\*.*?\*\/(.*)/\1/igs' $1
  perl -0777 -i -pe 's/(.*)\/\*.*?\*\/$/\1/igs' $1
}

for f in `find . -type f -name '*.cpp'` `find . -type f -name '*.h'`
do
  removeCpp $f
  removeC $f
  clang-format -i $f
done

