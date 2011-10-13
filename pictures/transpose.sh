#!/bin/bash
declare -a array=( )                      # we build a 1-D-array

#read -a line
# < "$1"                       # read the headline

index=0
COLS=0
tmpfile=./blah1234
while read -a line ; do
	if [[ ! -e $tmpfile ]]
	then
		echo ${#line[@]} > $tmpfile
	fi
    for (( COUNTER=0; COUNTER<${#line[@]}; COUNTER++ )); do
        array[$index]=${line[$COUNTER]}
        ((index++))
    done
done

COLS=$(cat $tmpfile)
rm $tmpfile

for (( ROW = 0; ROW < COLS; ROW++ )); do
  for (( COUNTER = ROW; COUNTER < ${#array[@]}; COUNTER += COLS )); do
    printf "%s\t" ${array[$COUNTER]}
  done
  printf "\n" 
done

