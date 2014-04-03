#!/bin/sh
create_pngs_aa() {
  declare -a NAMES=("${!1}")
  for i in ${NAMES[*]}; do
    SVG="orig/$i.svgz"
    BIG_PNG="$i_tmp.png"
    PNG="$i.png"
    inkscape -e $BIG_PNG -h 512 -w 512 $SVG
    convert $BIG_PNG -strip -format png32 -resize 32x32 $PNG
    rm $BIG_PNG
  done
}

create_pngs_no_aa() {
  declare -a NAMES=("${!1}")
  for i in ${NAMES[*]}; do
    SVG="orig/$i.svgz"
    TMP_PNG="$i_tmp.png"
    PNG="$i.png"
    inkscape -e $TMP_PNG -h 32 -w 32 $SVG
    convert $TMP_PNG -strip -format png32 $PNG
    rm $TMP_PNG
  done
}

ICONS_AA=(boundary_condition boundary_ellipse boundary_polygon no_tool polygonal_selection)
ICONS_NO_AA=(blocks_creator rectangle_selection)

create_pngs_aa ICONS_AA[@]
create_pngs_no_aa ICONS_NO_AA[@]

