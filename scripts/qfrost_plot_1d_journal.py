#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from os.path import splitext
from math import floor, ceil
from pylab import datestr2num
from numpy import arange, reshape
from matplotlib import pyplot as plt
from matplotlib.animation import FuncAnimation
from progressbar import ProgressBar
from matplotlib.dates import DateFormatter
import argparse

from qfrost_plot_basics import QFrostPlot

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


def plotstuff(x, y, z, is_thawed_parts_plot, filename,
              scale_factor, max_depth, z_cont=[]):
    print("Setting up plot...")
    cmap = QFrostPlot.Colormap(is_thawed_parts_plot)
    contourf_levels = QFrostPlot.Levels(is_thawed_parts_plot)

    QFrostPlot.SetupGridFor2D(plt.axes())

    QFrostPlot.SetupAxisMonth(plt.axes().xaxis)
    QFrostPlot.SetupAxisDepth(plt.axes().yaxis)

    print("Plotting contourf...")
    csetf = plt.contourf(x, y, z, contourf_levels, cmap=cmap, extend='both')

    colorbar = QFrostPlot.Colorbar(csetf, plt.gcf(), is_thawed_parts_plot)

    plt.axis('tight')
    plt.ylim(ymin=min(max_depth, plt.ylim()[0]))

    if len(z_cont):
        print('Plotting contour...')
        contour_locator = QFrostPlot.LocatorBasicTemperature()
        cset = plt.contour(x, y, z_cont,
                           locator=contour_locator,
                           colors=QFrostPlot.ContourBasicColors(),
                           linewidths=QFrostPlot.ContourBasicLineWidths())
        QFrostPlot.LabelContourTemperatures(cset)

    fig = plt.gcf()
    fig.set_size_inches(8, 6)
    #plt.tight_layout()

    print('Saving ' + filename + "...")
    plt.savefig(filename, dpi=130.28*scale_factor, bbox_inches='tight')
    print('Saved ' + filename + "!")

    plt.close()


def createanimation(t, v, tbf, y, dates, max_depth, filename, tmin, tmax):
    QFrostPlot.SetupGridFor1D(plt.axes())

    p_v, = plt.plot([], [], 'b', linewidth=2)

    QFrostPlot.SetupAxisThawedPart(plt.axes().xaxis)
    QFrostPlot.SetupAxisDepth(plt.axes().yaxis)

    t_ax = plt.axes().twiny()
    p_tbf, = t_ax.plot(tbf, y, 'k--', linewidth=1)
    p_t, = t_ax.plot([], [], 'k', linewidth=3)

    QFrostPlot.SetupAxisTemperature(t_ax.xaxis)
    plt.xlim(tmin, tmax)

    plt.legend((p_t, p_tbf, p_v),
               ('$T$', '$T_{bf}$', '$V_{th}$'),
               loc='lower left')
    fig = plt.gcf()
    fig.set_size_inches(4.8, 7.2)
    plt.ylim((max(y), min(y)))
    plt.ylim(ymin=min(max_depth, plt.ylim()[0]))
    #plt.yticks(arange(floor(min(y)), ceil(max(y)), 1.0))

    nicetitle = plt.figtext(0.08, 0.96, '',
                            size=20, horizontalalignment='center')

    plt.tight_layout()

    bar = ProgressBar(len(t)).start()

    def init():
        p_t.set_data([], [])
        p_v.set_data([], [])
        return p_t, p_v

    formatter = DateFormatter("%d.%m")
    def animate(i):
        if i == 0:
            p_t.set_ydata(y)
            p_v.set_ydata(y)
        p_t.set_xdata(t[i])
        p_v.set_xdata(v[i])
        nicetitle.set_text(formatter.format_data(dates[i]))
        bar.update(i)
        return p_t, p_v

    ani = FuncAnimation(fig, animate, frames=len(t),
                        interval=40,  blit=True, init_func=init)

    #plt.show()
    ani.save(filename, extra_args=['-vcodec', 'libx264'],
             # при битрейте 3000 нет артефактов для 480x720, при 6500 - 600x900
             # но можно выставлять с запасом, ибо размер получается куда меньше
             bitrate=10000, dpi=125)
    plt.close()


print("*** Parsing '%s' ***" % args.FILE.name)
dates = readpart(args.FILE, datestr2num)
depths = readpart(args.FILE, float)
print("Got {0} dates and {1} depths".format(len(dates), len(depths)))
t = readpart(args.FILE, float)
v = readpart(args.FILE, float)
tbf = readpart(args.FILE, float)
args.FILE.close()

if len(t) != len(v) or len(tbf) != len(depths) or len(t) != len(dates) * len(depths):
    raise ValueError('bad input: {0} dates, {1} depths, {2} tbf, {3} t & {4} v'
                     .format(len(dates), len(depths), len(tbf), len(t), len(v)))

tmin = min(t)
tmax = max(t)

t = reshape(t, (len(dates), len(depths)))
v = reshape(v, (len(dates), len(depths)))

filename_base = splitext(args.FILE.name)[0]

if args.anim:
    filename_anim = filename_base + '.mp4'
    print('*** Creating T animation (%s) ***' % filename_anim)
    createanimation(t, v, tbf, depths, dates, args.depth,
                    filename_anim, tmin, tmax)
else:
    t = t.swapaxes(0, 1)
    v = v.swapaxes(0, 1)

    stuff_extension = '.pdf' if args.scalable else '.png'
    scale_factor = 1.0 if args.scalable else args.factor

    filename_t = filename_base + '_t' + stuff_extension
    print('*** Plotting T (%s) ***' % filename_t)
    plotstuff(dates, depths, t, False,
              filename_t, scale_factor, args.depth, t)

    filename_v = filename_base + '_v' + stuff_extension
    print('*** Plotting V (%s) ***' % filename_v)
    plotstuff(dates, depths, v, True,
              filename_v, scale_factor, args.depth)

    filename_vt  = filename_base + '_vt' + stuff_extension
    print('*** Plotting V & T (%s) ***' % filename_vt)
    plotstuff(dates, depths, v, True,
              filename_vt, scale_factor, args.depth, t)
