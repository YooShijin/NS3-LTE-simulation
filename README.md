# Complete setup and run guide (Part A and Part B)

This assumes Ubuntu 20.04 or 22.04. Target ns-3 version is 3.36.1, which uses the `./ns3`
command. Every command below is run in a terminal.

## Step 1: Install system dependencies
```
sudo apt update
sudo apt install -y build-essential g++ python3 python3-dev cmake ninja-build git \
  libsqlite3-dev libxml2-dev gnuplot python3-pip
```

## Step 2: Download and unpack ns-3.36.1
```
cd ~
wget https://www.nsnam.org/releases/ns-allinone-3.36.1.tar.bz2
tar xjf ns-allinone-3.36.1.tar.bz2
```
This gives you `~/ns-allinone-3.36.1/ns-3.36.1`. That inner folder is your NS3_DIR.

## Step 3: Configure and build ns-3 (first build takes 20 to 40 minutes)
```
cd ~/ns-allinone-3.36.1/ns-3.36.1
./ns3 configure --enable-examples --enable-tests
./ns3 build
```
Check it works with a built in example:
```
./ns3 run hello-simulator
```
You should see `Hello Simulator`. If you see that, ns-3 is installed correctly.

## Step 4: Install Python packages for the ML part
```
pip3 install pandas numpy scikit-learn matplotlib joblib
```
Add ONE of these depending on your assigned model:
```
pip3 install xgboost        # only if your model is xgboost
pip3 install tensorflow     # only if your model is lstm, bilstm or rnn
```

## Step 5: Copy the assignment files into place
Put both source files in the `scratch` folder so they build automatically with no extra config.
```
cp lte_partA.cc  ~/ns-allinone-3.36.1/ns-3.36.1/scratch/
cp asg1_partb.cc ~/ns-allinone-3.36.1/ns-3.36.1/scratch/
cd ~/ns-allinone-3.36.1/ns-3.36.1
mkdir -p output
./ns3 build
```
The build should end with `finished successfully`. If it fails, read Step 9.

## Step 6: Run Part A and confirm it works
A single quick run first:
```
cd ~/ns-allinone-3.36.1/ns-3.36.1
./ns3 run "scratch/lte_partA --schedulerType=PF --speed=10 --simTime=30 --rngRun=15"
cat output/throughput.txt
```
You should see per flow throughput lines and a Summary block with Aggregate throughput
and a Jains Fairness Index between 0 and 1. That means Part A works.

To run the full sweep across all schedulers and speeds, use the helper script
(from the folder that holds the scripts):
```
NS3_DIR=~/ns-allinone-3.36.1/ns-3.36.1 RNGRUN=15 ./run_partA.sh
```
Results collect in `output/data.txt` (speed, scheduler, aggregate throughput) which you
plot for the throughput graphs. A REM file `output/lte_assign2.rem` is also produced.

## Step 7: Pick your ML model number
Formula: `(average integer of your enrollment numbers % 5) + 1`
1 = random_forest, 2 = lstm, 3 = bilstm, 4 = rnn, 5 = xgboost.
Example: enrollments 2101234 and 2105678, average integer 2103456, `2103456 % 5 = 1`,
`1 + 1 = 2` gives lstm.

## Step 8: Run the whole Part B pipeline
Edit the top of `run_partb.sh` or pass values on the command line. Set:
- MODEL to your model name from Step 7
- RNGRUN to the last two digits of your roll number
- STUDENT_TAG to Name_Enrollment with no spaces
```
NS3_DIR=~/ns-allinone-3.36.1/ns-3.36.1 MODEL=random_forest RNGRUN=34 \
  STUDENT_TAG=YourName_2105678 ./run_partb.sh
```
This runs, for each scheduler:
1. 500 s data collection into `partb_output/mcs_dataset_<sched>.csv`
2. model training, saving `mcs_model_<sched>` and a test plot `test_results_<sched>.png`
3. 50 s coordinate dump
4. Python prediction into `predicted_mcs_<sched>.csv`
5. 50 s logging run producing `mcs_log_<sched>.txt`

Confirm Part B worked by opening a log file:
```
head partb_output/mcs_log_PF.txt
```
You should see pairs of lines like:
```
YourName_2105678 original MCS value is 20
YourName_2105678 Predicted MCS value is 19
```
The `test_results_<sched>.png` files are your testing graphs for the report.

## Step 9: Common problems
- Build error on `DlSchedulingCallbackInfo` or on `GetMac`: you are on an ns-3 older than
  3.28. Change `DlSchedTrace` in asg1_partb.cc to the flat signature
  `void DlSchedTrace (uint32_t f, uint32_t sf, uint16_t rnti, uint8_t mcsTb1, uint16_t sz1,
  uint8_t mcsTb2, uint16_t sz2, uint8_t cc) { g_ueMcs[rnti] = mcsTb1; }` and connect using
  `Config::Connect ("/NodeList/*/DeviceList/*/LteEnbMac/DlScheduling", MakeCallback (&DlSchedTrace));`
  instead of the per device loop.
- `Cannot open file` for output: make sure the `output` folder exists in NS3_DIR and that you
  run `./ns3 run` from NS3_DIR.
- Python `No module named ...`: rerun the matching pip3 install from Step 4.
- Very slow 500 s run: build in optimized mode once with
  `./ns3 configure --build-profile=optimized --enable-examples` then `./ns3 build`.

## What each file is
- lte_partA.cc: Part A simulation, 4 eNBs, 40 UEs, scheduler sweep, throughput, JFI, REM.
- asg1_partb.cc: Part B simulation, 1 eNB, 10 UEs, SetMCS data collection and GetMCS logging.
- train_mcs_model.py: trains and saves the MCS prediction model plus a test plot.
- predict_mcs.py: predicts MCS for the 50 s coordinate dump.
- run_partA.sh: runs the full Part A sweep.
- run_partb.sh: runs the full Part B pipeline.
