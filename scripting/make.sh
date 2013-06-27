#!/bin/bash

compile()
{
	echo "compile $@"
	./compile.sh $@
	if [ $? -eq 1 ]; then
		exit
  fi
}

for FILE in `ls *.sp`; 
do 
LEN=${#FILE}; 
(( LEN -= 3 )); 
FILENAME=${FILE:0:$LEN}; 
SOURCEFILE="${FILENAME}.sp";
COMPILEDFILE="compiled/${FILENAME}.smx";
STIME=`stat -c %Y ${SOURCEFILE}`
CTIME=`stat -c %Y ${COMPILEDFILE}`
if [[ $STIME -lt $CTIME ]]; then
	continue
fi
compile ${SOURCEFILE} && cp ${COMPILEDFILE} ../plugins/; 
done
