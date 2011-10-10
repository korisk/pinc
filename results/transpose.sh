#!/bin/bash
declare -a array=( )                      # we build a 1-D-array

read -a line < "$1"                       # read the headline

COLS=${#line[@]}                          # save number of columns

index=0
while read -a line ; do
    for (( COUNTER=0; COUNTER<${#line[@]}; COUNTER++ )); do
        array[$index]=${line[$COUNTER]}
        ((index++))
    done
done < "$1"

for (( ROW = 0; ROW < COLS; ROW++ )); do
  for (( COUNTER = ROW; COUNTER < ${#array[@]}; COUNTER += COLS )); do
    printf "%s\t" ${array[$COUNTER]}
  done
  printf "\n" 
done

