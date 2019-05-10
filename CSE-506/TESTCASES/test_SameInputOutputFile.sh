#!/bin/sh
# 
# Test Encrypt Decrypt Functionality  with SAME INPUT FILE SAME AS THE OUTPUT FILE.
# DESIRED RESULT IS THAT THE PROGRAM THROWS AN ERROR.

set +x

#Variable Decleration
PASSWORD="SOMEPASSWORD"
TESTFILE="input.txt"
TEMPFILE="tempfile.txt"

#Create necessary files
touch $TESTFILE $TEMPFILE
chmod 777 $TESTFILE $TEMPFILE
echo "SOME WORDS" >> $TESTFILE
retval=$?
if [ $retval == 0 ] ; then
	echo "[TEST SCRIPT:INFO] SAMPLE INPUT FILE CREATED"
else
	echo "[TEST SCRIPT:INFO] SAMPLE INPUT FILE COULD NOT BE CREATED TO TEST FUNCTIONALITY."
	rm -f  $TESTFILE $TEMPFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	exit 1
fi

#Encrypt
./xhw1 "${TESTFILE}" "${TESTFILE}" -e -p "${PASSWORD}" #NOTE INPUT FILE SAME AS OUTPUT FILE
retval=$?

if test $retval == 0 ; then
	rm -f  $TESTFILE $TEMPFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	echo "[TEST SCRIPT:FAILURE] sys_cpenc PROGRAM DID NOT RUN AS EXPECTED"
	exit 1
else
	rm -f  $TESTFILE $TEMPFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	echo "[TEST SCRIPT:SUCCESS] sys_cpenc PROGRAM RAN AS EXPECTED"
	exit 0
fi
