#!/bin/bash

# Compile the input maker and the main simulation program
echo "Compiling files..."
g++ input_file_maker.cpp -o input_maker.out
g++ -fopenmp main.cpp utils.cpp init.cpp -lm -O3 -o main.out
echo "Compilation successful!"
echo "---------------------------------------------------"

# Create the CSV file with ALL the headers
echo "Config,Cores,Total_Alg_Time,Int_Time,Norm_Time,Mover_Time,Denorm_Time,Voids" > results.csv

declare -a configs=(
    "A 250 100 900000 10"
    "B 250 100 5000000 10"
    "C 500 200 3600000 10"
    "D 500 200 20000000 10"
    "E 1000 400 14000000 10"
)

cores=(1 2 4 6 8 10 12 14 16)

for config_info in "${configs[@]}"; do
    read -r config nx ny points iter <<< "$config_info"
    
    echo "==================================================="
    echo "Preparing Configuration $config: Grid ${nx}x${ny}, Points $points"
    
    # Generate the input
    echo "$nx $ny $points $iter" | ./input_maker.out > /dev/null

    for core in "${cores[@]}"; do
        export OMP_NUM_THREADS=$core
        
        # Run the program and capture the output
        OUTPUT=$(./main.out input.bin)
        
        # Extract EVERY specific timing metric using awk
        ALG_TIME=$(echo "$OUTPUT" | grep "Total Algorithm Time" | awk '{print $5}')
        INT_TIME=$(echo "$OUTPUT" | grep "Total Interpolation Time" | awk '{print $5}')
        NORM_TIME=$(echo "$OUTPUT" | grep "Total Normalization Time" | awk '{print $5}')
        MOVER_TIME=$(echo "$OUTPUT" | grep "Total Mover Time" | awk '{print $5}')
        DENORM_TIME=$(echo "$OUTPUT" | grep "Total Denormalization Time" | awk '{print $5}')
        
        # Extract the Number of Voids (This is the 6th word in the sentence)
        VOIDS=$(echo "$OUTPUT" | grep "Total Number of Voids" | awk '{print $6}')
        
        # Print a nice summary to the terminal
        echo "  -> $core cores | Alg: ${ALG_TIME}s | Int: ${INT_TIME}s | Norm: ${NORM_TIME}s | Mover: ${MOVER_TIME}s | Denorm: ${DENORM_TIME}s | Voids: $VOIDS"
        
        # Save ALL the detailed metrics to the CSV
        echo "$config,$core,$ALG_TIME,$INT_TIME,$NORM_TIME,$MOVER_TIME,$DENORM_TIME,$VOIDS" >> results.csv
    done
done

echo "==================================================="
echo "All experiments complete! Full dataset saved in results.csv."
