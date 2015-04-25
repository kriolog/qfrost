#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from os.path import splitext
import argparse

from journal_plot_1d import JournalPlot1D
from qfrost_plot_basics import QFrostVType

parser = argparse.ArgumentParser(prog='qfrost_plot',
                                 description='Generates various plots using data from QFrost one-dimensional journal(s). '
                                             'There are two possible plots: 2D (isopleths / color maps) and animated graphs.')

formats = parser.add_argument_group(title='output options (exclusive)')
format_group = formats.add_mutually_exclusive_group()
format_group.add_argument('-f', '--factor',
                          action='store',
                          type=float,
                          metavar='S',
                          help='scale factor for PNG files '
                               '(default: 1.0 ~ 700px height)',
                          default=1.0)
format_group.add_argument('-s', '--scalable',
                          action='store_true',
                          help='save to PDF (instead of PNG)')
format_group.add_argument('-a', '--anim',
                          action='store_true',
                          help='create 600x900 MP4 animation')

parser.add_argument('-d', '--depth',
                    action='store',
                    type=float,
                    metavar='D',
                    help='maximum depth in meters (default: 8.0)',
                    default=8.0)

parser.add_argument('-m', '--mark-thawed',
                    action='store_true',
                    help='mark thawed zone on T maps (with hatched Vth map)')


parser.add_argument('FILE',
                    nargs='*',
                    type=argparse.FileType('r'),
                    help='input file (with QFrost 1D journal)')

args = parser.parse_args()


for file in args.FILE:
    print("****** Processing journal from '%s' ******" % file.name)
    journal_plot = JournalPlot1D.from_file(file, args.depth, args.mark_thawed)
    file.close()

    if journal_plot is None:
        continue

    filename_base = splitext(file.name)[0]

    if args.anim:
        filename_anim = filename_base + '.mp4'
        print(' * Animating T&V [movie with graphs]')
        journal_plot.saveAnimation(filename_anim)
    else:
        stuff_extension = '.pdf' if args.scalable else '.png'
        scale_factor = 1.0 if args.scalable else args.factor

        filename_t = filename_base + '_t' + stuff_extension
        print(' * Plotting T [color map & contours]')
        journal_plot.savePlot2D(filename_t,
                                scale_factor,
                                QFrostVType.temperature,
                                QFrostVType.temperature)

        filename_tmap = filename_base + '_t_map' + stuff_extension
        print(' * Plotting T [standalone color map]')
        journal_plot.savePlot2D(filename_tmap,
                                scale_factor,
                                QFrostVType.temperature,
                                QFrostVType.none)

        filename_vmap = filename_base + '_v_map' + stuff_extension
        print(' * Plotting V [standalone color map]')
        journal_plot.savePlot2D(filename_vmap,
                                scale_factor,
                                QFrostVType.thawed_part,
                                QFrostVType.none)
        """
        filename_tv  = filename_base + '_both' + stuff_extension
        print(' * Plotting T&V [T map & V contours]')
        journal_plot.savePlot2D(filename_tv,
                                scale_factor,
                                QFrostVType.thawed_part,
                                QFrostVType.temperature)
        """
