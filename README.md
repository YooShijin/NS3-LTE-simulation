# LTE Scheduler Benchmarking and MCS Prediction (ns-3)

This project has two parts. Part A studies how four LTE MAC schedulers behave under
mobility and load. Part B takes it further and trains a small machine learning model to
predict a user's MCS straight from its position, then feeds those predictions back into
the simulation.

## What each part does

**Part A** sets up 4 base stations and 40 moving users, runs the simulation across the
PF, Round Robin, Max Throughput, and PSS schedulers, and measures aggregate throughput,
per user throughput, Jain's Fairness Index, and delay. It also produces a radio
environment map. This is the classic scheduler comparison.

**Part B** uses a single cell with 10 moving users. While it runs, it records each user's
position and the MCS the network assigned to it every 0.2 seconds, building a dataset.
A model learns the position to MCS mapping, and then a second run predicts the MCS from
coordinates and logs the predicted value next to the real one.

## Files

- `lte_partA.cc` and `asg1_partb.cc` are the two ns-3 programs. They go in the `scratch`
  folder of your ns-3 tree.
- `train_mcs_model.py` trains the model and saves a test plot.
- `predict_mcs.py` turns the collected coordinates into predicted MCS values.
- `run_partA.sh` and `run_partb.sh` run each part end to end.
- `SETUP_AND_RUN.md` is the full step by step guide, from installing ns-3 to reading the
  results. Start there if you have not set anything up yet.

## Quick start

If ns-3.36.1 is already built and the two `.cc` files are in `scratch`:

```
cd ~/ns-allinone-3.36.1/ns-3.36.1
./ns3 run "scratch/lte_partA --schedulerType=PF --speed=10 --simTime=30 --rngRun=15"
```

For the full runs use the scripts and set your own values:

```
NS3_DIR=~/ns-allinone-3.36.1/ns-3.36.1 RNGRUN=15 ./run_partA.sh
NS3_DIR=~/ns-allinone-3.36.1/ns-3.36.1 MODEL=random_forest RNGRUN=34 \
  STUDENT_TAG=YourName_2105678 ./run_partb.sh
```

## Before you run Part B

Three things are personal to you:

- **MODEL**: from `(average of your enrollment numbers % 5) + 1`, where
  1 = random_forest, 2 = lstm, 3 = bilstm, 4 = rnn, 5 = xgboost.
- **RNGRUN**: the last two digits of your roll number.
- **STUDENT_TAG**: `Name_Enrollment` with no spaces. This is what shows up in the log.

## What you get out

Part A leaves throughput and fairness numbers in `output/` plus the aggregate table in
`output/data.txt` for plotting, and a `.rem` file for the coverage map. Part B leaves the
dataset, the trained model, a test plot per scheduler, and the log files showing predicted
against actual MCS.

Full details, checkpoints, and troubleshooting are in `SETUP_AND_RUN.md`.
