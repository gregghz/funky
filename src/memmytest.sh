#!/bin/sh
uname -a | grep OpenBSD && exit 0
echo "Testing for memory leaks..."
which valgrind || (echo "Valgrind needed for memory tests" && exit 1)
cat test/*.in | valgrind ./$1 $2 2>&1 | grep 'no leaks are possible' || exit 2

