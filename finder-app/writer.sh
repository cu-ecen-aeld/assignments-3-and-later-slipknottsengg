#!/bin/bash

if [ $# -lt 2 ]
then
	echo "There are missing arguments."
	exit 1
else
	if [ ! -d $(dirname "$1") ]
	then
		mkdir -p $(dirname "$1")
	fi
	echo "$2" > "$1"
	if [ $? -eq 1 ]
	then
		echo "File could not be created."
		exit 1
	fi
fi
