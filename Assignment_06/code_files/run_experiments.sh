#!/bin/bash

# Compile the maker utility if not present
if [ ! -f maker ]; then
    g++ -O3 input_file_maker.cpp -o maker
fi

# Ensure main programs are compiled cleanly
make clean
make all

# Initialize CSV file
if [ ! -f results.csv ]; then
    echo "Config,Mode,Threads,Time" > results.csv
fi

# Define the 5 configurations in an associative array
declare -A configs
configs[input_A.bin]="250 100 900000 10"
configs[input_B.bin]="250 100 5000000 10"
configs[input_C.bin]="500 200 3600000 10"
configs[input_D.bin]="500 200 20000000 10"
configs[input_E.bin]="1000 400 14000000 10"

# Loop sequentially to save disk space
for CONFIG in "input_A.bin" "input_B.bin" "input_C.bin" "input_D.bin" "input_E.bin"; do
    
    PARAMS=${configs[$CONFIG]}
    echo "========================================"
    echo " Processing Configuration: $CONFIG"
    echo " Parameters: NX NY Points Iters -> $PARAMS"
    echo "========================================"

    # 1. Generate the large binary file for THIS config only
    ./maker $CONFIG $PARAMS

    # 2. Run Parallel tests (Averaging 3 runs)
    THREADS=(2 4 6 8 10 12 14 16)
    for t in "${THREADS[@]}"; do
        export OMP_NUM_THREADS=$t
        T1=$(./pic_parallel $CONFIG)
        T2=$(./pic_parallel $CONFIG)
        T3=$(./pic_parallel $CONFIG)
        AVG=$(echo "scale=6; ($T1 + $T2 + $T3) / 3" | bc)
        echo "Parallel | $CONFIG | Threads: $t | Time: $AVG s"
        echo "${CONFIG%.*},Parallel,$t,$AVG" >> results.csv
    done

    # 3. Run Serial test (Averaging 3 runs)
    T1=$(./pic_serial $CONFIG)
    T2=$(./pic_serial $CONFIG)
    T3=$(./pic_serial $CONFIG)
    AVG=$(echo "scale=6; ($T1 + $T2 + $T3) / 3" | bc)
    echo "Serial   | $CONFIG | Threads: 1 | Time: $AVG s"
    echo "${CONFIG%.*},Serial,1,$AVG" >> results.csv

    # 4. DELETE the large binary file to preserve quota!
    rm -f $CONFIG
    echo ">> Deleted $CONFIG to save disk space."
    echo ""

done

echo "========================================"
echo " All configurations processed! "
echo " Run 'python3 plot_results.py' to get your graphs."
echo "========================================"