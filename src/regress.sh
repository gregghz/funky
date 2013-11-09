#!/bin/sh
for x in `ls bugs/*.sh`; do
    ./$x
done
