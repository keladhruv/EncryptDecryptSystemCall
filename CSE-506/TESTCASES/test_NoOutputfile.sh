#!/bin/sh
# OUTPUT FILE IS NOT CREATED 
# DESIRED RESULT SUCCESSFULL ENCRYPTION/DECRYPTION.
# Test Encrypt Decrypt Functionality without a createed output file..

set +x

#Variable Decleration
PASSWORD="SOMEPASSWORD"
TESTFILE="input.txt"
OUTPUTFILE="output.txt"
TEMPFILE="tempfile.txt"

#Creating Necessary files
touch $TESTFILE $TEMPFILE #NO OUTPUT FILE CREATED
chmod 777 $TESTFILE $TEMPFILE
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
	echo "[TEST SCRIPT:INFO] FILE ENCRYPTED"
else
	echo "[TEST SCRIPT:INFO] FILE COULD NOT BE ENCRYPTED."
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
	rm -f $TESTFILE $OUTPUTFILE $TEMPFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
		echo "[SUCCESS] INPUT OUPUT FILE ARE THE SAME"
		exit 0
else
	echo "[FAILURE] INPUT OUPUT FILE ARE THE SAME"
	rm -f $TESTFILE $OUTPUTFILE $TEMPFILE
	retval=$?
	if [ $retval == 0 ] ; then
		echo "[TEST SCRIPT:INFO] REMOVED INPUT/OUTPUT FILES"
	else
		echo "[TEST SCRIPT:INFO] COULD NOT REMOVE INPUT/OUTPUT FILES."
	fi
	echo "[FAILURE] INPUT OUPUT FILE ARE THE SAME"
	exit 1
fi
