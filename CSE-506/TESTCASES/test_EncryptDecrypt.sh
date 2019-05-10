#!/bin/sh
# Test Encrypt Decrypt Functionality  [10 bytes of data].
# The desired result is that it runs without any error.

set +x

#Variable Decleration
PASSWORD="SOMEPASSWORD"
TESTFILE="input.txt"
OUTPUTFILE="output.txt"
TEMPFILE="tempfile.txt"

#Creating required files and setting permissions accordingly.
touch $TESTFILE $OUTPUTFILE $TEMPFILE
chmod 777 $TESTFILE $OUTPUTFILE $TEMPFILE
echo "SOME WORDS" >> $TESTFILE
retval=$?
if [ $retval == 0 ] ; then
	echo "[TEST SCRIPT:INFO] SAMPLE INPUT FILE CREATED"
else
	echo "[TEST SCRIPT:INFO] SAMPLE INPUT FILE COULD NOT BE CREATED TO TEST FUNCTIONALITY."
	rm -f  $TESTFILE $OUTPUTFILE $TEMPFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
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
	rm -f  $TESTFILE $OUTPUTFILE $TEMPFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	exit 1
fi

#Decrypt

./xhw1 "${OUTPUTFILE}" "${TEMPFILE}" -d -p "${PASSWORD}"
retval=$?
if [ $retval == 0 ] ; then
	echo "[TEST SCRIPT:INFO] FILE DECRYPTED"
else
	echo "[TEST SCRIPT:INFO] FILE COULD NOT BE DECRYPTED."
	exit 1
fi


# now verify that the two files are the same
if cmp $TESTFILE $TEMPFILE ; then
	rm -f  $TESTFILE $OUTPUTFILE $TEMPFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
		echo "[TEST SCRIPT:SUCCESS] INPUT OUPUT FILE ARE THE SAME"
		exit 0
else
	echo "[TEST SCRIPT:FAILURE] INPUT OUPUT FILE ARE THE SAME"
	rm -f  $TESTFILE $OUTPUTFILE $TEMPFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	echo "[TEST SCRIPT:FAILURE] INPUT OUPUT FILE ARE THE SAME"
	exit 1
fi
