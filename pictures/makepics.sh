#!/bin/bash
mkdir ./pre;

for i in $(ls ../results/)
do 
	cat ../results/$i|grep "*"|tr -d '*'|./transpose.sh|head -n 24 > ./pre/$i;
	__ri__=$i
GPLT1=$( cat <<EOF
#!/usr/bin/gnuplot
set title  "$__ri__"
set xlabel "Threads"
set ylabel "Time, msec"
#plot '' u 1 w linespoints ti 'serial increment',   '$__ri__' u 2 w linespoints ti 'mutexes' ,'$__ri__' u 3 w linespoints ti '__sync_fetch_and_add' ,'$__ri__' u 4 w linespoints ti '__sync_bool_compare_and_swap'
set key left top
set terminal png
set output "$__ri__.png"
plot './pre/$__ri__' u 1 w linespoints ti 'serial increment',   './pre/$__ri__' u 2 w linespoints ti 'mutexes' ,'./pre/$__ri__' u 3 w linespoints ti '__sync_fetch_and_add' ,'./pre/$__ri__' u 4 w linespoints ti '__sync_bool_compare_and_swap', './pre/$__ri__' u 5 w linespoints ti 'spinlock'
EOF
)

	echo "$GPLT1"|gnuplot


done

for line in $(seq 1 5)
do

	case $line in
		1)
			title=serial
			;;
		2) 
			 title=mutexts
			;;
		3)  
			title=__sync_fetch_and_add
			;;
		4)  
			title=__sync_bool_compare_and_swap
			;;
		5)  
			title=spinlock
			;;
		*)
			title=unknown
	esac

	plotstring="plot "
	for i in $(ls ./pre)
	do
		plotstring="$plotstring './pre/$i' u $line w linespoints ti '$i',"
	done
	plotstring=$(echo $plotstring|sed s/.$//)

GPLT2=$( cat <<EOF
#!/usr/bin/gnuplot
set title  "$title"
set xlabel "Threads"
set ylabel "Time, msec"
set key left top
set terminal png
set output "$title.png"
$plotstring
EOF
)
	echo "$GPLT2"|gnuplot

done

rm -r ./pre
