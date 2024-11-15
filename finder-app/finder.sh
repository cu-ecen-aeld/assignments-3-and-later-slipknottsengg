#!/bin/sh

if [ $# -lt 2 ]
then
	echo "There are missing arguments."
	exit 1
elif [ ! -d "$1" ]
then
	echo "The directory does not exist."
	exit 1
else
	num_files=$( find "$1" -type f | wc -l )
	num_match=$( grep -r "$2" $1 | wc -l )
	echo "The number of files are $num_files and the number of matching lines are $num_match"
fi
