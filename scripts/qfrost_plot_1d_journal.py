#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from sys import stderr
from os.path import splitext
from math import floor, ceil
from pylab import datestr2num
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
                               '(default: 1.0 = 700px height)',
                          default=1.0)
format_group.add_argument('-s', '--scalable',
                          action='store_true',
                          help='save to PDF (instead of PNG)')
format_group.add_argument('-a', '--anim',
                          action='store_true',
                          help='create 480x720 MP4 animation')

parser.add_argument('-d', '--depth',
                    action='store',
                    type=float,
                    metavar='D',
                    help='maximum depth in meters (default: 8.0)',
                    default=8.0)

parser.add_argument('FILE',
                    nargs='*',
                    type=argparse.FileType('r'),
                    help='input file (with QFrost 1D journal)')

args = parser.parse_args()


def readpart(f, arg_converter=float):
    """
    1D массив значений, полученных построчным чтением f вплоть до пустой строки.
    None, если попалась неконвертируемая строка (arg_converter выдал ошибку).

    f: построчно итерируемый объект (например, открытый файл или массив строк)
    arg_converter: конвертер строк в значения элементов (по умолчанию - float).
    """
    result = []
    for line in f:
        token = line.strip()
        if not token:
            break # пустая строка
        val = None
        try:
            val = arg_converter(token)
        except:
            print(' - can not parse data: type convertion failed!\n'
                  ' >> %s' % line.strip('\n'), file=stderr)
            return None
        result.append(val)
    return result


for file in args.FILE:
    print("*** Processing journal file '%s' ***" % file.name)
    dates = readpart(file, datestr2num)
    if dates is None:
        continue
    depths = readpart(file)
    print(" * Got {0} dates & {1} depths (tot. {2} vals)"
        .format(len(dates), len(depths), len(dates)*len(depths)))
    t = readpart(file)
    v = readpart(file)
    tbf = readpart(file)
    file.close()

    journal_plot = JournalPlot1D(dates, depths, tbf, t, v, args.depth)

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
