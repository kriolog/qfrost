#!/bin/bash
#with_stab_1.txt | awk '{print $1"\t"$2}' 
LAST_Y=0.0
for var in "$@"
do
    cat $var | awk -v last_y="$LAST_Y" '{print $1"\t"$2+last_y"\t"$3"\t"$4}'
    LAST_Y=$(echo "$LAST_Y + $(tail -n1 $var | cut -f2)" | bc)
done
