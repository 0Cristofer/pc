#!/bin/bash
progs="linkedList barnes"
modes="seq mutex spin semaforo trans psemaforo"
not_modes="ptrans tbb"
count="1 2 3 4 5 6 7 8 9 10 11"
n_procs="2 4 8 16"
dirs_home="$PWD"

echo Começando a execução a partir de $dirs_home
echo

echo -e "\tCompilando programas"

for p in $progs; do
	echo
	echo -e "\tCompilando $p"
	echo
	for m in $modes; do
		pm="$p"_"$m"
		echo "Compilando $pm"
		cd "$p/$pm/"
		make
		cd "../../"
		echo "-----Fim $pm-----"
		echo
	done
	echo "-----Fim $p------"
done
echo

echo -e "\tExecutando programas"
echo

for p in $progs; do
        echo -e "\tExecutando $p"
        echo
        for m in $modes; do
		pm="$p"_"$m"
		out=$dirs_home/../out/$p/$pm
		echo "-----Executando $pm-----"
		cd "$p/$pm/"
               	if [ "$m" == "seq" ]; then
			for i in $count; do
				echo "-----Executando run $i-----"
				if [ "$p" == "linkedList" ]; then
					perf stat -d -o $out/prefout$i.txt ./$pm > $out/out$i.txt
				else
					perf stat -d -o $out/perfout$i.txt ./$pm < "./in/in" > $out/out$i.txt
				fi
				echo "-----Fim run $i-----"
			done
		else
			for n in $n_procs; do
				echo "-----Executando $pm com $n fluxos-----"
				for i in $count; do
					echo "-----Executando run $i-----"
					if [ "$p" == "linkedList" ]; then
						perf stat -d -o $out/$n/perfout$i.txt ./$pm -n "$n" > $out/$n/out$i.txt
					else
						perf stat -d -o $out/$n/perfout$i.txt ./$pm < "./in/in$n" > $out/$n/out$i.txt
					fi
					echo "-----Fim run $i-----"
				done
			done
		fi
               	cd "../.."
		echo "-----Fim $pm-----"
	echo
        done
done
echo

