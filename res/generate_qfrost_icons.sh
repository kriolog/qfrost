#!/bin/sh
create_pngs() {
  declare -a sizes=("${!1}")
  for i in ${sizes[*]}; do
    size="${i}x${i}"
    convert $2 -strip -format png32 -resize ${size} ${3}_${size}.png
  done
}

rm qfrost_*x*.png

inkscape -D -e tmp_small.png -h 1024 -w 1024 orig/qfrost_no_shadow.svg
inkscape -e tmp_big.png -h 1024 -w 1024 orig/qfrost.svg

# http://msdn.microsoft.com/en-us/library/aa511280.aspx#size
# http://www.visualpharm.com/articles/icon_sizes.html
# http://www.creativefreedom.co.uk/icon-designers-blog/windows-7-icon-sizes/
# http://iconhandbook.co.uk/reference/chart/osx/
sizes_small=(8 10 14 16 20 22 24)
create_pngs sizes_small[@] tmp_small.png qfrost
sizes_big=(32 40 48 64 72 96 128 180 256 512)
create_pngs sizes_big[@] tmp_big.png qfrost

# http://www.netmagazine.com/features/create-perfect-favicon
sizes_favicon_small=(16 24)
create_pngs sizes_favicon_small[@] tmp_small.png favicon
sizes_favicon_big=(32 57 72 96 114 128 195)
create_pngs sizes_favicon_big[@] tmp_big.png favicon

# Лого для сайта
# Тут важен strip как минимум потому, что оно убирает gAMA chunk, который делает картинку темнее IE6, да и в принципе лишний для веба
convert tmp_big.png -strip -format png24 -resize x115 -trim +repage -background '#D6D6E7' -flatten logo.png

rm tmp_small.png tmp_big.png

convert qfrost_*x*.png qfrost.ico
convert favicon_*x*.png favicon.ico

rm favicon_*x*.png
