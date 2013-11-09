#!/bin/sh
echo '{}' | valgrind ./funk_machine ./indispensibles/core.srfm 2>&1 | grep 'no leaks are possible' && exit 0
echo 'Memory leak detected when creating an empty grid'
exit 1
