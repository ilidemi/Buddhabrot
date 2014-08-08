#!/bin/bash

pics=$(ls -1 | grep -P 'pic_\d+_\d+\.tiff')
pics_count=$(echo $pics | wc -l)
pics_side=$(echo 'sqrt('$pics_count')' | bc)

# Convert everything to ppm
for pic in $pics
do
	tifftopnm $pic > ${pic%%.*}.ppm
done

# Merge
pamundice pic_%3d_%3a.ppm -down 4 -across 4 > pic.ppm

# Scale
pamscale pic.ppm -reduce 2 > pic_scaled.ppm

# Convert to tiff
pamtotiff pic_scaled.ppm > pic.tiff

# Remove temporary stuff
rm pic_*_*.tiff pic_*_*.ppm pic.ppm pic_scaled.ppm