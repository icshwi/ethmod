#!/bin/bash

IN="$1"
N="$2"

echo "Multiplying input template $IN $N-times"

rm -f tmpl
for n in $(seq 0 15)
do
	sed -e "s/\$(N)/$n/g" $IN >> tmpl
done

exit 0
