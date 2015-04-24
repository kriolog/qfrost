#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from os.path import splitext
from math import floor, ceil
from pylab import datestr2num
import argparse

from journal_plot_1d import JournalPlot1D
from qfrost_plot_basics import QFrostVType

parser = argparse.ArgumentParser(prog='qfrost_plot',
                                 description='Does plots of QFrost 1D journal files. '
                                             'By default creates PNG isopleths, but also '
                                             'can do PDF isopleths or AVI animated curves')

formats = parser.add_argument_group(title='Output Options (exclusive)')
format_group = formats.add_mutually_exclusive_group()
format_group.add_argument('-f', '--factor',
                          action='store',
                          type=float,
                          metavar='S',
                          help='scale factor for PNG isopleths '
                               '(default: 1.0 = 700px height)',
                          default=1.0)
format_group.add_argument('-s', '--scalable',
                          action='store_true',
                          help='save isopleths to PDF '
                               '(by default will save to PNG)')
format_group.add_argument('-a', '--anim',
                          action='store_true',
                          help='create 480x720 AVI animation')

parser.add_argument('-d', '--depth',
                    action='store',
                    type=float,
                    metavar='D',
                    help='maximum depth in meters (default: 7.95)',
                    default=8.0)

parser.add_argument('FILE',
                    nargs='?',
                    type=argparse.FileType('r'),
                    help='path to data file (default: data.txt)',
                    default='data.txt')

args = parser.parse_args()


def readpart(f, arg_converter):
    result = []
    while True:
        s = f.readline().replace(' ', "").replace('\n', '')
        if s == "":
            return result
        result.append(arg_converter(s))


print("*** Processing journal file '%s' ***" % args.FILE.name)
dates = readpart(args.FILE, datestr2num)
depths = readpart(args.FILE, float)
print(" * Got {0} dates & {1} depths (tot. {2} vals)"
      .format(len(dates), len(depths), len(dates) * len(depths)))
t = readpart(args.FILE, float)
v = readpart(args.FILE, float)
tbf = readpart(args.FILE, float)
args.FILE.close()

journal_plot = JournalPlot1D(dates, depths, tbf, t, v, args.depth)

filename_base = splitext(args.FILE.name)[0]

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

    filename_v = filename_base + '_v' + stuff_extension
    print(' * Plotting V [standalone color map]')
    journal_plot.savePlot2D(filename_v,
                            scale_factor,
                            QFrostVType.thawed_part,
                            QFrostVType.none)

    filename_tv  = filename_base + '_both' + stuff_extension
    print(' * Plotting T&V [T map & V contours]')
    journal_plot.savePlot2D(filename_tv,
                            scale_factor,
                            QFrostVType.thawed_part,
                            QFrostVType.temperature)
