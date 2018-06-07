#!/bin/bash
progs="linkedList barnes"
modes="seq mutex spin semaforo psemaforo"
not_modes="ptrans tbb"
count="2 3 4 5 6 7 8 9 10 11"
n_procs="2 4 8 16"
dirs_home="$PWD"

echo Começando a execução a partir de $dirs_home

echo -e "\tLendo resultados"
echo

for p in $progs; do
    echo -e "\tLendo $p"
    echo
    touch saida_$p.txt
    echo "" >saida_$p.txt
      for m in $modes; do
        pm="$p"_"$m"
	      out=/$p/$pm
        echo "$pm" >> $dirs_home/saida_$p.txt
	      cd "$p/$pm/"
       	if [ "$m" == "seq" ]; then
             touch main_out.txt
		         for i in $count; do
				           echo "-----Lendo run $i-----"
		               if [ "$p" == "linkedList" ]; then
	                    grep -i "realizadas" out$i.txt | awk '{print $6}' >>main_out.txt
                      grep -i "insn" prefout$i.txt | awk '{print $4}' >>main_out.txt

	                 else
                      #BARNES
			                grep -i "context-switches" perfout$i.txt | awk '{print $1}' >>main_out.txt
                      grep -i "page-faults" perfout$i.txt | awk '{print $1}' >>main_out.txt
                      grep -i "insn" perfout$i.txt | awk '{print $4}' >>main_out.txt
                      grep -i "branch-misses" perfout$i.txt | awk '{print $4}' >>main_out.txt
                      grep -i "L1-dcache-load-misses" perfout$i.txt | awk '{print $4}' >>main_out.txt
                      grep -i "LLC-load-misses" perfout$i.txt | awk '{print $4}' >>main_out.txt
                      grep -i "seconds" perfout$i.txt | awk '{print $1}' >>main_out.txt

                   fi
                   echo "" >>main_out.txt
                   echo "-----Fim run $i-----"
			        done

              if [ "$p" == "linkedList" ]; then
                python $dirs_home/orgLinked.py $dirs_home/$p/$pm/main_out.txt >main_out_med.txt
              else
                python $dirs_home/orgBarnes.py $dirs_home/$p/$pm/main_out.txt >main_out_med.txt
              fi

              cat main_out_med.txt >> $dirs_home/saida_$p.txt
              echo "" >> $dirs_home/saida_$p.txt
        else
			       for n in $n_procs; do
                   echo $n >>$dirs_home/saida_$p.txt
				           echo "-----Lendo $pm com $n fluxos-----"
                   touch main_out_$n.txt
		               for i in $count; do
				                echo "-----Lendo run $i-----"
		                   if [ "$p" == "linkedList" ]; then
                            grep -i "realizadas" ./$n/out$i.txt | awk '{print $6}' >>main_out_$n.txt
                            grep -i "insn" ./$n/perfout$i.txt | awk '{print $4}' >>main_out_$n.txt

                       else
                          #BARNES
                          grep -i "context-switches" ./$n/perfout$i.txt | awk '{print $1}' >>main_out_$n.txt
                          grep -i "page-faults" ./$n/perfout$i.txt | awk '{print $1}' >>main_out_$n.txt
                          grep -i "insn" ./$n/perfout$i.txt | awk '{print $4}' >>main_out_$n.txt
                          grep -i "branch-misses" ./$n/perfout$i.txt | awk '{print $4}' >>main_out_$n.txt
                          grep -i "L1-dcache-load-misses" ./$n/perfout$i.txt | awk '{print $4}' >>main_out_$n.txt
                          grep -i "LLC-load-misses" ./$n/perfout$i.txt | awk '{print $4}' >>main_out_$n.txt
                          grep -i "seconds" ./$n/perfout$i.txt | awk '{print $1}' >>main_out_$n.txt
                       fi
                       echo "" >>main_out_$n.txt
		                   echo "-----Fim run $i-----"
	                 done

                   if [ "$p" == "linkedList" ]; then
                     python $dirs_home/orgLinked.py $dirs_home/$p/$pm/main_out_$n.txt >main_out_med_$n.txt
                   else
                     python $dirs_home/orgBarnes.py $dirs_home/$p/$pm/main_out_$n.txt >main_out_med_$n.txt
                   fi

                     cat main_out_med_$n.txt >> $dirs_home/saida_$p.txt
                     echo "" >> $dirs_home/saida_$p.txt
            done
        fi
 	      cd "../.."
        echo "-----Fim $pm-----"
	      echo
        done
done
echo
