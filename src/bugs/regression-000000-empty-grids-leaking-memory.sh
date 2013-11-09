#!/bin/sh
uname -a | grep OpenBSD && echo "There's a better way to do this in OpenBSD and I need to implement that" && exit 0
echo '{}' | valgrind ./funk_machine ./indispensibles/core.srfm 2>&1 | grep 'no leaks are possible' && exit 0
echo 'Memory leak detected when creating an empty grid'
exit 1
