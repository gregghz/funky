#!/bin/sh
inputfile=$1
outputfile=`echo $1 | sed -e 's/\.in/.out/'`
goodfile=`echo $1 | sed -e 's/\.in/.gud/'`
./funk_machine ./indispensibles/core.srfm -b $inputfile 2>&1 | sed -e 's/@[0-9]*@/@address-omitted@/g' > $outputfile
diff $goodfile $outputfile && echo 'Done' && exit 0
echo "Output for test $inputfile differs from known good results."
cat $outputfile
exit 1
