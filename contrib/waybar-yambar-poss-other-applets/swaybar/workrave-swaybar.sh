#!/bin/sh

~/bin/workrave_swaybar_gen_json.py &

# Listening for STDIN events
while read line;
do
  if [[ $line == *"name"*"workrave"* ]]; then
    ~/bin/workrave-open.py
  fi  
done
