#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from matplotlib import rc
rc('font', **{'family' : 'Droid Sans'})

from os.path import splitext
from math import floor, ceil
from pylab import datestr2num
from numpy import arange, reshape
from matplotlib import pyplot as plt
from matplotlib.ticker import FuncFormatter
from matplotlib.ticker import MultipleLocator
from matplotlib.dates import MonthLocator
from matplotlib.dates import DateFormatter
import matplotlib.colors as col
from progressbar import ProgressBar
from matplotlib.animation import FuncAnimation
import argparse

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


def formatPercent(x, i):
    return str(int(round(x*100.0, 0))) + "\%"

percentformatter = FuncFormatter(formatPercent)


def setDepthAxesSettings(axe):
    axe.set_major_locator(MultipleLocator())
    axe.set_minor_locator(MultipleLocator(base=0.2))


def plotstuff(x, y, z, is_thawed_parts_plot, filename,
              scale_factor, max_depth, z_cont=[]):

    cmap = plt.cm.jet if not is_thawed_parts_plot else plt.cm.gist_ncar
    """
    if is_thawed_parts_plot:
        cmap = plt.cm.gist_ncar
    else:
        cmap = col.ListedColormap(["#00007F",
                                   "#0000C0",
                                   "#0000FF",
                                   "#007FFF",
                                   "#00FFFF",
                                   "#FFFF00",
                                   "#FF7F00",
                                   "#FF0000",
                                   "#C00000",
                                   "#7F0000"])
    """

    if is_thawed_parts_plot:
        V = [i/20.0 for i in range(0, 21)]
        V[0] = 1e-15
        V[-1] = 1.0 - V[0]
    else:
        #V = [-20, -10, -5, -2, 0, 2, 5, 10, 20]
        V = [i/2.0 for i in range(-20, 21)]

    plt.ylabel('Глубина, м')
    plt.xlabel('Месяц')

    plt.ylim(plt.ylim()[::-1])

    setDepthAxesSettings(plt.axes().yaxis)
    plt.axes().xaxis.set_major_locator(MonthLocator())
    plt.axes().xaxis.set_major_formatter(DateFormatter('%m'))

    plt.grid(True, ls='-', c='#a0a0a0')

    print("Plotting contourf...")
    plt.contourf(x, y, z, V, cmap=cmap, extend='both')

    if not is_thawed_parts_plot:
        colorbar = plt.colorbar(ticks=MultipleLocator(base=1.0))
        #colorbar = plt.colorbar(ticks=[-20, -10, -5, -2, 0, 2, 5, 10, 20])
        colorbar.set_label(u"Температура $T$, $^\circ$C")
    else:
        colorbar = plt.colorbar(ticks=V, format=percentformatter)
        colorbar.set_label("Относительный объём талой фазы $V_{th}$")

    plt.axis('tight')
    plt.ylim(ymin=min(max_depth, plt.ylim()[0]))

    if len(z_cont):
        print('Plotting contour...')
        cset = plt.contour(x, y, z_cont,
                           locator=MultipleLocator(base=0.5),
                           linewidths=0.5, colors='k', linestyles='solid')
        plt.clabel(cset,
                   fontsize=6, inline=True, fmt=r'$%1.1f$',
                   use_clabeltext=True)

    fig = plt.gcf()
    fig.set_size_inches(8, 6)
    #plt.tight_layout()

    print('Saving ' + filename + "...")
    plt.savefig(filename, dpi=130.28*scale_factor, bbox_inches='tight')
    print('Saved ' + filename + "!")

    plt.close()


def createanimation(t, v, tbf, y, dates, max_depth, filename):
    formatter = DateFormatter("%d.%m")
    plt.axes().set_axisbelow(True)  # сетка позади графиков
    plt.grid(True, ls=':', c='#a0a0a0')

    p_v, = plt.plot([], [], 'b', linewidth=2)
    plt.xlim(0, 1)
    v_ax_label = plt.xlabel("Относительный объём талой фазы $V_{th}$",
                            color="blue")

    plt.ylabel('Глубина, м')

    v_ax = plt.gca()
    [i.set_color("blue") for i in v_ax.get_xticklabels()]

    setDepthAxesSettings(v_ax.yaxis)
    v_ax.xaxis.set_major_formatter(percentformatter)

    t_ax = plt.axes().twiny()
    p_tbf, = t_ax.plot(tbf, y, 'k--', linewidth=1)
    p_t, = t_ax.plot([], [], 'k', linewidth=3)

    plt.xlim(-8, 2)
    plt.xlabel("Температура, $^\circ$C")

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

t = reshape(t, (len(dates), len(depths)))
v = reshape(v, (len(dates), len(depths)))

out_basename = splitext(args.FILE.name)[0]

if args.anim:
    print('*** Creating T animation ***')
    createanimation(t, v, tbf, depths, dates, args.depth, out_basename + '.mp4')
else:
    t = t.swapaxes(0, 1)
    v = v.swapaxes(0, 1)

    stuff_extension = '.pdf' if args.scalable else '.png'
    scale_factor = 1.0 if args.scalable else args.factor
    print('*** Plotting T ***')
    plotstuff(dates, depths, t, False,
              out_basename + '_t' + stuff_extension, scale_factor, args.depth, t)

    print('*** Plotting V ***')
    plotstuff(dates, depths, v, True,
              out_basename + '_v' + stuff_extension, scale_factor, args.depth)

    print('*** Plotting V and T ***')
    plotstuff(dates, depths, v, True,
              out_basename + '_tv' + stuff_extension, scale_factor, args.depth, t)
