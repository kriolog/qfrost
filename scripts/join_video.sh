#!/bin/sh
DEFAULT_FILENAME=plot.mp4
[[ ! -z "$1" ]] && FILE1="$1" || FILE1="with/$DEFAULT_FILENAME"
[[ ! -z "$2" ]] && FILE2="$2" || FILE2="without/$DEFAULT_FILENAME"

ffmpeg -y -i "$FILE1" -vf "[in] pad=2*iw:ih [left]; movie="$FILE2" [right]; 
[left][right] overlay=main_w/2:0 [out]" -b:v 20000k plots.mp4
