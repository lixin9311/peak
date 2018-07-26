#!/bin/bash

PROG=$1
FROM=${FROM:-800}
TO=${TO:-$(($FROM*4096))}
MULT=${MULT:-2}
REPEAT=${REPEAT:-10}
N=${N:-$((8*14*4))}

>&2 echo "Program: $PROG"
>&2 echo "Range: $FROM ~ $TO, mult $MULT"
>&2 echo "Repeat: $REPEAT"
>&2 echo "N: $N"

for ((i=$FROM; i<=$TO; i*=$MULT)); do
    echo "Size $((N * i))" >&2
    echo -ne "$((N * i))\t"
    for ((j=0; j < $REPEAT; j++)); do
        echo -ne "$($PROG $i)\t"
    done
    echo ""
done