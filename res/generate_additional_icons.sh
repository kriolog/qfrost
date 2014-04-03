#!/bin/bash
create_pngs() {
  declare -a sizes=("${!1}")
  for i in ${sizes[*]}; do
    size="${i}x${i}"
    convert $2 -strip -format png32 -resize ${size} ${3}_${size}.png
  done
}

#create_pngs() {
#  declare -a sizes=("${!1}")
#  for i in ${sizes[*]}; do
#    size="${i}x${i}"
#    inkscape -e "${2}_${size}.png" -h ${i} -w ${i} ${3}
#  done
#}

sizes1=(22 32 48)
sizes2=(16)

inkscape -e tmp.png -h 1024 -w 1024 orig/discretize-colors.svgz
inkscape -e tmp16.png -h 1024 -w 1024 orig/discretize-colors16.svgz
create_pngs sizes1[@] tmp.png discretize-colors
create_pngs sizes2[@] tmp16.png discretize-colors
#create_pngs sizes1[@] discretize-colors orig/discretize-colors.svgz
#create_pngs sizes2[@] discretize-colors orig/discretize-colors16.svgz
rm tmp.png tmp16.png
