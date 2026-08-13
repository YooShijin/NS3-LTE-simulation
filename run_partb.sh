#!/bin/bash
set -e

NS3_DIR="${NS3_DIR:-$HOME/ns-allinone-3.36.1/ns-3.36.1}"
MODEL="${MODEL:-random_forest}"
RNGRUN="${RNGRUN:-15}"
SPEED="${SPEED:-10}"
STUDENT_TAG="${STUDENT_TAG:-YourName_1234567}"
SCHEDULERS="${SCHEDULERS:-PF RR MT PSS}"
LOGCOMP="Asg1PartB=level_info"

HERE="$(pwd)"
OUT="$HERE/partb_output"
mkdir -p "$OUT"

cd "$NS3_DIR"

for S in $SCHEDULERS; do
  echo "==== Scheduler $S : data collection 500s ===="
  ./ns3 run "scratch/asg1_partb --schedulerType=$S --speed=$SPEED --rngRun=$RNGRUN --simTime=500 --phase=dump --dumpFile=$OUT/mcs_dataset_${S}.csv"

  echo "==== Scheduler $S : training model ($MODEL) ===="
  python3 "$HERE/train_mcs_model.py" --data "$OUT/mcs_dataset_${S}.csv" --model "$MODEL" --out "$OUT/mcs_model_${S}" --plot "$OUT/test_results_${S}.png"

  echo "==== Scheduler $S : coords dump 50s ===="
  ./ns3 run "scratch/asg1_partb --schedulerType=$S --speed=$SPEED --rngRun=$RNGRUN --simTime=50 --phase=dump --dumpFile=$OUT/coords_50s_${S}.csv"

  if [ "$MODEL" = "lstm" ] || [ "$MODEL" = "bilstm" ] || [ "$MODEL" = "rnn" ]; then
    MODELFILE="$OUT/mcs_model_${S}.keras"
  else
    MODELFILE="$OUT/mcs_model_${S}.joblib"
  fi

  echo "==== Scheduler $S : predicting MCS ===="
  python3 "$HERE/predict_mcs.py" --model "$MODELFILE" --coords "$OUT/coords_50s_${S}.csv" --out "$OUT/predicted_mcs_${S}.csv"

  echo "==== Scheduler $S : logging run 50s ===="
  NS_LOG="$LOGCOMP" ./ns3 run "scratch/asg1_partb --schedulerType=$S --speed=$SPEED --rngRun=$RNGRUN --simTime=50 --phase=log --predFile=$OUT/predicted_mcs_${S}.csv --studentTag=$STUDENT_TAG" 2> "$OUT/mcs_log_${S}.txt"

  echo "Log written to $OUT/mcs_log_${S}.txt"
done

echo "All done. Outputs in $OUT"
