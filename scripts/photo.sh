#!/bin/bash

# Basic settings
base_dir="/home/qubi/projects/camera_server/frames"

# Create a directory for the current session with a name based on the date and time
timestamp=$(date +"%d_%m_%Y_%H:%M")
output_dir="$base_dir"

# Exposure times (shutter time)
shutter_times=(1000 5000 10000 20000 50000 100000 200000 500000 1000000)
  # Shutter time in milliseconds

# Loop to take photos
for i in "${!shutter_times[@]}"; do
    shutter=${shutter_times[$i]}
    output_file="$output_dir/frame_$shutter-µs.jpg"

    # Take a photo with different exposure times
    echo "Taking photo with shutter time: $shutter ms"
    libcamera-still -t 1000 --shutter $shutter -o "$output_file"

    echo "Saved photo: $output_file"
done

echo "Photo taking process completed! Photos saved in directory: $output_dir"
mkdir -p /home/qubi/images/temp/$timestamp
cp -r /home/qubi/projects/camera_server/frames/* /home/qubi/images/temp/$timestamp
