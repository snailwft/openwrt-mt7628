#!/bin/bash
##########################
# Copyright 2014-2015, Silicon Labs
#
# Shell script to build ALL targets for demo program.
# NOTE: this takes some time & uses AWK.
#
# $Id: buildall.sh 4766 2015-02-24 22:49:37Z nizajerk $
#
##########################


#
# Make sure build completed & rename file as needed.
#
function chk_bld() 
{
	if [ ! -e $1 ] 
	then
		echo "Failed to build! $1"
		exit
	fi

	fn=`echo $1 | awk -F . '{print $1}'`
	vmb_name=`echo $VMB| tr '[:upper:]' '[:lower:]'`
	fn2=$fn\_$vmb_name.exe

	cp $1 tmp/$fn2
	make clean

	count=$((count+1))

	return 0
}

##########################
#
# NOTE: file needs to be UNIX formatted... no \r's
#
# Iterate through a text file of targets.
#
#  set SCAN_CHK to scan-build for clang static analysis.
#  set BUILD_STRICT to "STRICT=1" for strict builds...

function build_all()
{
	for TARGET in `cat buildtargets.txt`; do
    		$SCAN_CHK make $makeopt $TARGET=1 $VMB=1 $BUILD_STRICT $MLT_BUILD
		fn=`echo $TARGET| tr '[:upper:]' '[:lower:]'`
		chk_bld $fn
		if (($? >0)); then 
			exit 1
		fi
	done
}

##########################

rm -rf tmp
mkdir tmp 2>/dev/null
make clean

count=0
#makeopt="-j 4"

if [ "`uname -s`" != "Linux" ]; then
  VMB=VMB1
  build_all
fi

VMB=VMB2
build_all

echo "Built $count targets"


