#!/bin/bash

# Loop through values from 1 to 22
for i in {1..22}
do
    # Run the command with [i] replaced by the current value
    command="echo \"193.136.128.103 58012 $i\" | nc tejo.tecnico.ulisboa.pt 59000 > report_$i.html"
    eval $command
    
    # Add a 1-second delay
    sleep 1
done
