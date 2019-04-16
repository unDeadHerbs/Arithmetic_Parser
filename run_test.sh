#!/bin/bash

ls tests | sed 's/[.]\(in\|out\)//' | uniq |
    while read r; do
	echo "Starting test $r"
	# existance of file
	[ -e "$(echo $r | sed 's/[.].*/.bin/')"  ] || { echo "Unable to find $(echo $r | sed 's/[.].*/.bin/')" ; exit 1 ; }
	# existance of input
	[ -e "$(echo $r | sed 's/.*/tests\/&.in/')"  ] || { echo "Unable to find $(echo $r | sed 's/.*/&.in/')" ; exit 1 ; }
	# existance of output
	if [ -e "$(echo $r | sed 's/.*/tests\/&.out/')"  ] ; then
	    $(echo $r | sed 's/[.].*/.bin/'|sed 's/.*/.\/&/') $(echo $r | sed 's/.*/tests\/&.in/') 2>&1 |
		diff -baC 1 - $(echo $r | sed 's/.*/tests\/&.out/') ||
		{ echo "\n--- From Input ---\n" ; cat $(echo $r | sed 's/.*/tests\/&.in/') ; exit 1 ; }
	else
	    echo
	    $(echo $r | sed 's/[.].*/.bin/'|sed 's/.*/.\/&/') $(echo $r | sed 's/.*/tests\/&.in/') 2>&1
	    echo
	fi
	echo "Test $r compleate."
    done
# list test files
# for each test/x.#.in find x.bin
#  print #
#  diff (x.bin test/x.#.in) with test/x.#.out
