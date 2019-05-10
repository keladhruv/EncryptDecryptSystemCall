#!/bin/sh
# Test Encrypt Functionality with PASSWORD LENGTH LESS THAN 6.
# DESIRED RESULT IS THAT THE PROGRAM THROWS AN ERROR.

set +x

# Variable Decleration
PASSWORD="SOME"	#PASSWORD LENGTH IS 4
TESTFILE="input.txt"
OUTPUTFILE="output.txt"
TEMPFILE="tempfile.txt"
touch $TESTFILE $OUTPUTFILE 
chmod 777 $TESTFILE $OUTPUTFILE 


#Encrypt
./xhw1 "${TESTFILE}" "${OUTPUTFILE}" -e -p "${PASSWORD}"
retval=$?

if test $retval == 0 ; then
	rm -f  $TESTFILE $OUTPUTFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	echo "[TEST SCRIPT:FAILURE] sys_cpenc PROGRAM DID NOT RUN AS EXPECTED"
	exit 1
else
	rm -f  $TESTFILE $OUTPUTFILE 
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	echo "[TEST SCRIPT:SUCCESS] sys_cpenc PROGRAM RAN AS EXPECTED"
	exit 0
fi



