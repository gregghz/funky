#!/bin/sh
UNIQUETHING="`date`"
echo $UNIQUETHING  > /tmp/crap.$$
echo "(read (stream '/tmp/crap.$$'))" | ./funk_machine ./indispensibles/core.srfm | grep "$UNIQUETHING" && rm /tmp/crap.$$ && exit 0
echo "I want to be able to create file stream objects that are treated as first-class objects and get closed when cleaned up by garbage collection."
rm /tmp/crap.$$
exit 1
