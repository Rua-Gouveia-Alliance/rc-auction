#!/bin/bash

# Loop through values from 1 to 22
for i in {1..15}
do
    # Run the command with [i] replaced by the current value
    echo "running $i"
    command="echo \"193.136.128.108 58012 $i\" | nc tejo.tecnico.ulisboa.pt 59000 > report_$i.html"
    eval $command
    
    # Add a 1-second delay
    sleep 1
done
