#!/bin/sh
# 
# Test Encrypt Decrypt Functionality  with WRONG PASSWORD.
# DESIRED RESULT FAILURE.

set +x

#Variable Decleration
PASSWORD="SOMEPASSWORD"
PASSWORD_2="SOMEWRONGPASSWORD"
TESTFILE="input.txt"
OUTPUTFILE="output.txt"
TEMPFILE="tempfile.txt"

#Creating Necessary Files
touch $TESTFILE $OUTPUTFILE $TEMPFILE
chmod 777 $TESTFILE $OUTPUTFILE $TEMPFILE
echo "SOME WORDS" >> $TESTFILE
retval=$?
if [ $retval == 0 ] ; then
	echo "[TEST SCRIPT:INFO] SAMPLE INPUT FILE CREATED"
else
	echo "[TEST SCRIPT:INFO] SAMPLE INPUT FILE COULD NOT BE CREATED TO TEST FUNCTIONALITY."
	rm -f $TESTFILE $OUTPUTFILE $TEMPFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	exit 1
fi

#Encrypt
./xhw1 "${TESTFILE}" "${OUTPUTFILE}" -e -p "${PASSWORD}"
retval=$?
if [ $retval == 0 ] ; then
	echo "[INFO] FILE ENCRYPTED"
else
	echo "[INFO] FILE COULD NOT BE ENCRYPTED."
	rm -f $TESTFILE $OUTPUTFILE $TEMPFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	exit 1
fi

#Decrypt

./xhw1 "${OUTPUTFILE}" "${TEMPFILE}" -d -p "${PASSWORD_2}" #NOTE WRONG PASSWORD GIVEN
retval=$?

if test $retval == 0 ; then
	rm -f $TESTFILE $TEMPFILE $OUTPUTFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	echo "[TEST SCRIPT:FAILURE] sys_cpenc PROGRAM DID NOT RUN AS EXPECTED"
	exit 1
else
	rm -f $TESTFILE $TEMPFILE $OUTPUTFILE 
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	echo "[TEST SCRIPT:SUCCESS] sys_cpenc PROGRAM RAN AS EXPECTED"
	exit 0
fi
