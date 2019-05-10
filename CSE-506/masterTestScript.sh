#!/bin/sh

#This is the master test script which runs all the test scripts in TESTCASES/
#NOTE-If you want to run the test scripts individully,i.e, without master test script,-
#Go to the TESTCASES/ folder, copy the 'xhw1' after making and installing the module to the TESTCASES/ folder.
#Command to make module - 'make' [RUN IT IN CSE506 Folder]
#Command to install module - 'sh install_module.sh' [RUN IT IN CSE506 Folder]

#Variable Decleration
failiure=0
count=0

#running make clean
make clean
retval=$?
if [ $retval == 0 ] ; then
	echo "[MASTER TEST SCRIPT:INFO] MAKE CLEAN SUCCESSFULL"
else
	echo "[MASTER TEST SCRIPT:INFO] COULD NOT RUN MAKE CLEAN."
	exit 1
fi

#running make
make
retval=$?
if [ $retval == 0 ] ; then
	echo "[MASTER TEST SCRIPT:INFO] MAKE SUCCESSFULL"
else
	echo "[MASTER TEST SCRIPT:INFO] COULD NOT RUN MAKE."
	exit 1
fi

#installing module
sh install_module.sh
retval=$?
if [ $retval == 0 ] ; then
	echo "[MASTER TEST SCRIPT:INFO] SUCCESSFULLY INSTALLED MODULE"
else
	echo "[MASTER TEST SCRIPT:INFO] COULD NOT INSTALL MODULE."
	exit 1
fi

#Copying xhw1 to TESTCASES/
chmod 777 TESTCASES/*
cp xhw1 TESTCASES/
retval=$?
if [ $retval == 0 ] ; then
	echo "[MASTER TEST SCRIPT:INFO] COPIED xhw1"
else
	echo "[MASTER TEST SCRIPT:INFO] COULD NOT COPY xhw1."
	exit 1
fi

#Running each test in TESTCASES/
for file in TESTCASES/test*; do
	count=$(($count+1))
	./${file} 
	if [ $retval != 0 ] ; then
		echo "[MASTER TEST SCRIPT:FAILURE]TEST SCRIPT : ${file} FAILED"
		failiure=$(($failiure+1))
	else
		echo "[MASTER TEST SCRIPT:SUCCESS]TEST SCRIPT : ${file} PASSED"
	fi
	done
	
	count=$(($count-$failiure))
#Showing results
echo "[MASTER TEST SCRIPT:RESULT]"
echo "TESTS THAT RAN AS EXPECTED:$count"
echo "TESTS THAT FAILED $failiure"

#Removing copied xhw1
rm -f TESTCASES/xhw1
retval=$?
if [ $retval == 0 ] ; then
	echo "[MASTER TEST SCRIPT:INFO] REMOVED xhw1"
else
	echo "[MASTER TEST SCRIPT:INFO] COULD NOT REMOVE xhw1."
fi

if [ $failiure == 0 ]; then
	exit 0
else
	exit 1
fi


