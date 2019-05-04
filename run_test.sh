#!/bin/bash

set -e

# Needs atleast one of `.in`, `.file`, or `.out`.

if [ $# -ne 0 ]; then
    echo "This script takes no arguments."
    echo "All tests are infered from the tests/ directory"
    echo "A test will be run if it has any of the following"
    echo " 1) An input file \"bin_name.input\""
    echo " 2) A parameter file \"bin_name.file\""
    echo " 3) An output file \"bin_name.output\""
    exit 0
fi

ls tests | sed 's/[.]\(input\|file\|output\)//' | uniq |
    while read r; do
	echo "Running test $r"
	# existance of file
	[ -e "$(echo $r | sed 's/[.].*//').bin"  ] || { echo "Unable to find $(echo $r | sed 's/[.].*//').bin" ; exit 1 ; }
	# existance of output
	if [ -e "tests/$r.input" ]; then
	    cat "tests/$r.input"
	fi |
	timeout 2 "./$(echo $r|sed 's/[.].*//').bin" $([ -e "tests/$r.file" ] && \
				 echo "tests/$r.file") 2>&1 |
	if [ -e "tests/$r.output" ]; then
	    if diff -baC 1 - "tests/$r.output"; then
		cat
	    else
		if [ -e "tests/$r.input" ]; then
		   echo "--- Pipe Input ---"
		   cat "tests/$r.input"
		fi
		if [ -r "tests/$r.file" ]; then
		   echo "--- File Input ---"
		   cat "tests/$r.file"
		fi
		exit 1
	    fi
	else
	    # if command failed
	    cat
	    # exit 1
	    # fi
	fi
    done
