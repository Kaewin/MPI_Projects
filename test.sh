#!/bin/bash

echo "Points,Blocking_Time,NonBlocking_Time" > results.csv

for power in {10..26}; do
    points=$((2**power))
    echo "Testing with $points points..."
    
    # Run blocking version
    blocking_time=$(mpirun -n 6 ./monte_blocking $points | grep "Average" | awk '{print $5}')
    
    # Run non-blocking version
    nonblocking_time=$(mpirun -n 6 ./monte_non_blocking $points | grep "Average" | awk '{print $5}')
    
    echo "$points,$blocking_time,$nonblocking_time" >> results.csv
    
    # Stop if time exceeds ~60 seconds
    if (( $(echo "$blocking_time > 60" | bc -l) )); then
        break
    fi
done