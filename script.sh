#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Utilizare: bash script.sh <c>"
    exit 1
fi

valoare=$1
cnt=0

# Citire din pipe
while read -r linie || [[ -n "$linie" ]]; do
    if [[ "$linie" =~ ^[[:upper:]][[:alnum:],[:space:].!]*[.?!]$ && "$linie" != *",si "* ]]; then
        echo "$linie"
        ((cnt++))
    fi
done

echo "Propozitii corecte care contin caracterul '$valoare': $cnt"
