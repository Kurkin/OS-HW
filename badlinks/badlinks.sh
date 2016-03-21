#!/usr/bin/env bash

DIR=$1

linkchk () {
    FILES=$(find -L $1 -type l -mmin +10080)
    for element in $FILES; do
        echo $element
        #[ -h "$element" -a ! -e "$element" ] && echo \"$element\" 
    done
}

linkchk $DIR

