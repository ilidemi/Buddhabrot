#!/bin/bash

base_name=$1
scaling_factor=$2
pics="$(ls -1 | grep -P "$base_name""_\d+_\d+\.tiff")"
pics_count=$(echo $pics | wc -l)
pics_side=$(echo 'sqrt('$pics_count')' | bc)

# Convert everything to ppm
for pic in $pics
do
	tifftopnm $pic > ${pic%%.*}.ppm
done

# Merge
pamundice ''$base_name''_%3d_%3a.ppm -down $pics_side -across $pics_side > ''$base_name''.ppm

# Scale
pamscale ''$base_name''.ppm -reduce $scaling_factor > ''$base_name''_scaled.ppm

# Convert to tiff
pamtotiff ''$base_name''_scaled.ppm > ''$base_name''.tiff

# Remove temporary stuff
rm ''$base_name''_*_*.tiff ''$base_name''_*_*.ppm ''$base_name''.ppm ''$base_name''_scaled.ppm