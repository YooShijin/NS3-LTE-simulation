#!/bin/bash
set -e

NS3_DIR="${NS3_DIR:-$HOME/ns-allinone-3.36.1/ns-3.36.1}"
RNGRUN="${RNGRUN:-15}"
SCHEDULERS="${SCHEDULERS:-PF RR MT PSS}"
SPEEDS="${SPEEDS:-0 10 20 30}"

cd "$NS3_DIR"
mkdir -p output
rm -f output/data.txt

for S in $SCHEDULERS; do
  for V in $SPEEDS; do
    echo "==== Part A : scheduler $S speed $V ===="
    ./ns3 run "scratch/lte_partA --schedulerType=$S --speed=$V --simTime=30 --rngRun=$RNGRUN --fullBuffer=false"
    cp output/throughput.txt "output/throughput_${S}_v${V}.txt"
  done
done

echo "==== Part A : generating one REM (PF, speed 0) ===="
./ns3 run "scratch/lte_partA --schedulerType=PF --speed=0 --simTime=30 --rngRun=$RNGRUN --generateRem=true"

echo "Part A done. Aggregate table in $NS3_DIR/output/data.txt"
