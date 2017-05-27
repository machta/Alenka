#!/bin/bash

# Usage: ./download-data.sh
#
# This script downloads the test files from SourceForge.
# This is a workaround for GitHub's screwed up LFS system.
# Use Git Bash or a similar tool to run this on Windows.

function download
{
	curl -L 'https://sourceforge.net/projects/alenka-mirror/files/test-files/'$1'/download' > $1 &&
	unzip -o $1 &&
	rm $1
}

function md5
{
	ls `cat $1 | awk '{print $2}'` && md5sum -c $1
}

data0=skipped
md5 data0.md5 || download data0.zip && md5 data0.md5 && data0=OK || data0=fail

echo
echo ======= Download summary =======
echo "File                Status"
echo ================================
echo "data0               $data0"

