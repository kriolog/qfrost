#!/bin/sh
create_pngs() {
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

ICONS=(boundary_condition boundary_ellipse boundary_polygon no_tool polygonal_selection ellipse_selection blocks_creator rectangle_selection curve_plot)

create_pngs ICONS[@]
