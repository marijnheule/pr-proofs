#!/usr/bin/env bash

for i in `seq 3 10`; do
    echo $i
    julia Mycielski-dproof.jl $i $((i-1)) > M$i.dpr
    julia Mycielski-proof.jl  $i $((i-1)) > M$i.pr
done
