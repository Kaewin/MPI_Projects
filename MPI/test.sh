#!/bin/bash
# Scaling Study

PROCS=6  # Match what you're using

# Output file
echo "Points,Blocking_Time,NonBlocking_Time" > results.csv

# Start with 2^10 points
power=10

while true; do
    points=$((2**power))
    echo "Testing with $points points..."
    
    # Run blocking version and extract time
    blocking_output=$(mpirun -n $PROCS ./monte_blocking $points 2>&1)
    # Look for "Average time per rank:" and get the number
    blocking_time=$(echo "$blocking_output" | grep "Average time per rank:" | awk '{print $5}')
    
    # Run non-blocking version and extract time
    nonblocking_output=$(mpirun -n $PROCS ./monte_non_blocking $points 2>&1)
    nonblocking_time=$(echo "$nonblocking_output" | grep "Average time per rank:" | awk '{print $5}')
    
    # Save to CSV
    echo "$points,$blocking_time,$nonblocking_time" >> results.csv
    echo "  Blocking: ${blocking_time}s, Non-blocking: ${nonblocking_time}s"
    
    # Check if we've exceeded 60 seconds
    if [ -n "$blocking_time" ]; then
        if (( $(echo "$blocking_time > 60" | bc -l) )); then
            echo "Blocking version exceeded 60 seconds. Stopping."
            break
        fi
    fi
    
    # Increment power for next iteration
    power=$((power + 1))
    
    # Safety check
    if [ $power -gt 30 ]; then
        echo "Reached maximum points (2^30). Stopping."
        break
    fi
done

echo "Results saved to results.csv"