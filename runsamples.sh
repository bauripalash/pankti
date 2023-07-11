#! /bin/env sh

export CPANK_EXE=./zig-out/bin/pankti

for SAMPLE in sample/*.pank; do 
	echo -en "\nRunning:" $SAMPLE "\n";
	$CPANK_EXE $SAMPLE; 
done
