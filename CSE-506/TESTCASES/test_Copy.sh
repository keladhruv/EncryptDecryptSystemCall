#!/bin/sh
# Test Copy Functionality with 10 bytes of data.
# The desired result is that there is no error thrown.

set +x

# Variable Decleration
PASSWORD="SOMEPASSWORD"
TESTFILE="input.txt"
OUTPUTFILE="output.txt"

#Creating required files and setting permissions accordingly.
touch $TESTFILE $OUTPUTFILE
chmod 777 $TESTFILE $OUTPUTFILE
echo "SOME WORDS" >> $TESTFILE
retval=$?
if [ $retval == 0 ] ; then
	echo "[TEST SCRIPT:INFO] SAMPLE INPUT FILE CREATED"
else
	echo "[TEST SCRIPT:INFO] SAMPLE INPUT FILE COULD NOT BE CREATED TO TEST FUNCTIONALITY."
	rm -f $TESTFILE $OUTPUTFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	exit 1
fi

#Running Copy functionality
./xhw1 "${TESTFILE}" "${OUTPUTFILE}" -c
retval=$?

if test $retval != 0 ; then
	echo "[TEST SCRIPT:INFO] sys_cpenc PROGRAM FAILED WITH ERROR": $retval
	rm -f $TESTFILE $OUTPUTFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	exit $retval
else
	echo "[TEST SCRIPT:INFO] sys_cpenc PROGRAM RAN SUCCESSFULLY"
fi

# now verify that the two files are the same
if cmp $TESTFILE $OUTPUTFILE ; then
	rm -f $TESTFILE $OUTPUTFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	echo "[TEST SCRIPT:SUCCESS] INPUT OUPUT FILE ARE THE SAME"
		exit 0
else
	rm -f $TESTFILE $OUTPUTFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	echo "[TEST SCRIPT:FAILURE] INPUT OUPUT FILE ARE THE SAME"
	exit 1
fi
