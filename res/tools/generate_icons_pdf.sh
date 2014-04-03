#!/bin/sh
rm *.pdf
inkscape -A blocks_creator.pdf orig/blocks_creator.svgz
inkscape -A boundary_polygon.pdf orig/boundary_polygon.svgz
inkscape -A boundary_ellipse.pdf orig/boundary_ellipse.svgz
inkscape -A rectangle_selection.pdf orig/rectangle_selection.svgz
inkscape -A no_tool.pdf orig/no_tool.svgz
inkscape -A boundary_condition.pdf orig/boundary_condition.svgz
inkscape -A polygonal_selection.pdf orig/polygonal_selection.svgz
