#!/bin/sh
uname -a | grep OpenBSD && exit 0
for x in `find ./test -name '*.in' | sort`; do
    printf "Running $x... ";
    ./single-test.sh $x || exit 1
done
echo "All tests passed."
