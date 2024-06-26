#!/bin/env bash

clear
for n in {1..100};
do
	make
	echo "-- $n --"
done
