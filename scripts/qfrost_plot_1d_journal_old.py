#!/usr/bin/env python2
# -*- coding: utf-8 -*-

'''
from matplotlib.backend_bases import register_backend
from matplotlib.backends.backend_pgf import FigureCanvasPgf
register_backend('pdf', FigureCanvasPgf)

from matplotlib import rcParams
pgf_with_custom_preamble = {
    "font.family": "serif",
    "text.usetex": False,
    "text.latex.unicode": True,
    "pgf.rcfonts": False,
    "pgf.texsystem": "pdflatex",
    "pgf.preamble": [
        r"\usepackage[utf8]{inputenc}",
        r"\usepackage[russian]{babel}",
        r"\usepackage[T2A]{fontenc}"
    ]
}
rcParams.update(pgf_with_custom_preamble)

from matplotlib import rc
rc('font', **{'family': 'Liberation Sans'})
rc('text', usetex=False)
'''

from matplotlib import rc
rc('font', **{'family':'serif'})
rc('text', usetex=True)
rc('text.latex', unicode=True)
rc('text.latex', preamble='\usepackage[utf8]{inputenc}')
rc('text.latex', preamble='\usepackage[russian]{babel}')

from math import floor, ceil
from pylab import datestr2num
from numpy import arange, reshape
from matplotlib import pyplot as plt
from matplotlib.ticker import FuncFormatter
from matplotlib.ticker import MultipleLocator
from matplotlib.dates import MonthLocator
from matplotlib.dates import DateFormatter
from subprocess import check_call
from os import mkdir
from os.path import exists
from progressbar import ProgressBar
import argparse

parser = argparse.ArgumentParser(prog='qfrost_plot',
                                 description='Plots stuff from QFrost journal data files. '
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


def formatPercent(x, for_tex=None):
    return str(int(round(x*100.0, 0))) + "%"


def formatPercentTex(x, for_tex=None):
    return str(int(round(x*100.0, 0))) + "\,\%"

percentformatter = FuncFormatter(formatPercent)
percentformattertex = FuncFormatter(formatPercentTex)


def setDepthAxesSettings(axe):
    axe.set_major_locator(MultipleLocator())
    axe.set_minor_locator(MultipleLocator(base=0.2))


def plotstuff(x, y, z, is_thawed_parts_plot, filename, scale_factor, z_cont=[]):
    cmap = plt.cm.jet if not is_thawed_parts_plot else plt.cm.gist_ncar

    V = []
    if is_thawed_parts_plot:
        V = [i/20.0 for i in range(0, 21)]
        V[0] = 1e-15
        V[-1] = 1.0 - V[0]
    else:
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
        colorbar.set_label(u"Температура $T$, $^\circ$C")
    else:
        colorbar = plt.colorbar(ticks=V, format=percentformattertex)
        colorbar.set_label("Относительный объём талой фазы $V_{th}$")

    plt.axis('tight')

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


def createanimation(t, v, tbf, y, dates):
    formatter = DateFormatter("%d.%m")
    pngs_dir = 'anim'
    if not exists(pngs_dir):
        mkdir(pngs_dir)
    bar = ProgressBar(len(t)).start()
    for i in range(len(t)):
        bar.update(i)
        date_text = formatter.format_data(dates[i])
        if i == 0:
            plt.grid(True, ls='-', c='#a0a0a0')

            p_tbf, = plt.plot(tbf, y, 'k--', linewidth=1)
            p_t, = plt.plot(t[i], y, 'k', linewidth=2)

            #plt.ylim(plt.ylim()[::-1])
            plt.xlim(-8, 2)
            plt.xlabel("Температура, $^\circ$C")
            plt.ylabel('Глубина, м')

            ytl1 = plt.axes().get_yticklabels()
            ytl1[0].set_visible(False)
            #ytl1[-1].set_visible(False)

            ax2 = plt.axes().twiny()
            p_v, = ax2.plot(v[i], y, 'b', linewidth=2)
            plt.xlim(0, 1)
            ax2.set_xlabel("Относительный объём талой фазы $V_{th}$")

            setDepthAxesSettings(ax2.yaxis)
            ax2.xaxis.set_major_formatter(percentformatter)

            plt.legend((p_t, p_v, p_tbf),
                       ('$T$', '$V_{th}$', '$T_{bf}$'),
                       'lower left')
            fig = plt.gcf()
            fig.set_size_inches(4.8, 7.2)
            nicetitle = plt.figtext(0.08, 0.96, date_text,
                                    size=20, horizontalalignment='center')
            plt.ylim((max(y), min(y)))
            plt.yticks(arange(floor(min(y)), ceil(max(y)), 1.0))
            plt.tight_layout()
        else:
            p_t.set_xdata(t[i])
            p_v.set_xdata(v[i])
            nicetitle.set_text(date_text)

        filename = pngs_dir + '/' + str('%03d' % i) + '.png'
        plt.savefig(filename, dpi=100)
    bar.finish()
    avifile = 'output.avi'
    command = ('mencoder',
               'mf://%s/*.png' % pngs_dir,
               '-mf', 'type=png:fps=25',
               '-ovc', 'lavc',
               '-lavcopts', 'vcodec=ffvhuff',
               '-nosound',
               '-o', avifile)
    print("about to execute:\n%s" % ' '.join(command))
    check_call(command)
    print("The movie was written to '" + avifile + "'")
    print("You may want to delete anim/*.png now.")
    plt.close()

print("*** Parsing '%s' ***" % args.FILE.name)
dates = readpart(args.FILE, datestr2num)
depths = readpart(args.FILE, float)
t = readpart(args.FILE, float)
v = readpart(args.FILE, float)
tbf = readpart(args.FILE, float)
args.FILE.close()

t = reshape(t, (len(dates), len(depths)))
v = reshape(v, (len(dates), len(depths)))

if args.anim:
    print('*** Creating T animation ***')
    createanimation(t, v, tbf, depths, dates)
else:
    t = t.swapaxes(0, 1)
    v = v.swapaxes(0, 1)
    stuff_extension = '.pdf' if args.scalable else '.png'
    scale_factor = 1.0 if args.scalable else args.factor
    print('*** Plotting T ***')
    plotstuff(dates, depths, t, False,
              'temperatures' + stuff_extension, scale_factor, t)

    print('*** Plotting V ***')
    plotstuff(dates, depths, v, True,
              'thawedparts' + stuff_extension, scale_factor)

    print('*** Plotting V and T ***')
    plotstuff(dates, depths, v, True,
              'both' + stuff_extension, scale_factor, t)
