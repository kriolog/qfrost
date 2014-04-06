#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from matplotlib import rc
rc('font', **{'family': 'serif'})
rc('text', usetex=True)
rc('text.latex', unicode=True)
rc('text.latex', preamble=r"\usepackage[utf8]{inputenc}")
rc('text.latex', preamble=r"\usepackage[russian]{babel}")

from scipy import interpolate
from math import floor, ceil
from pylab import datestr2num
from numpy import reshape, fromstring
import numpy
from matplotlib import pyplot as plt
from matplotlib.ticker import FuncFormatter
from matplotlib.ticker import MultipleLocator
import matplotlib.tri as tri
from matplotlib.path import Path
from matplotlib.dates import MonthLocator
from matplotlib.dates import DateFormatter
import matplotlib.colors as col
from progressbar import ProgressBar
from matplotlib.animation import FuncAnimation
import argparse
from matplotlib.patches import PathPatch
from matplotlib.tri import UniformTriRefiner, TriAnalyzer

parser = argparse.ArgumentParser(prog='qfrost_plot',
                                 description='Does plots of QFrost 2D export files.')

formats = parser.add_argument_group(title='Output Options (exclusive)')
format_group = formats.add_mutually_exclusive_group()
format_group.add_argument('-f', '--factor',
                          action='store',
                          type=float,
                          metavar='S',
                          help='scale factor for PNG plots '
                               '(default: 1.0 = 700px height)',
                          default=1.0)
format_group.add_argument('-s', '--scalable',
                          action='store_true',
                          help='save plots to PDF '
                               '(by default will save to PNG)')

parser.add_argument('FILE',
                    nargs='?',
                    type=argparse.FileType('r'),
                    help='path to data file')

args = parser.parse_args()


def readpart(f):
    lines = []
    while True:
        line = f.readline().rstrip()
        if line == "" or line[0].isalpha():
            return numpy.array([x.split('\t') for x in lines], dtype=numpy.float)
        lines.append(line)

def pathes(points):
    result = []
    polygon = []
    for point in points:
        polygon.append(point)
        if len(polygon) > 2 and (point == polygon[0]).all():
            result.append(Path(polygon))
            polygon = []
    return  result

def anyPolygonContainsPoint(polygons, point):
    for polygon in polygons:
        if polygon.contains_point(point):
            return True
    return False


def domainContainsPoint(outerPolygons, innerPolygons, point):
    if not anyPolygonContainsPoint(outerPolygons, point):
        return False
    # FIXME проверять только по полигонам, принаджежащих нашему внешн. полигону)
    return not anyPolygonContainsPoint(innerPolygons, point)

def maskByDomain(triang, outerPolygons, innerPolygons):
    #midsX = fullX[triang.triangles].mean(axis=1)
    #midsY = fullY[triang.triangles].mean(axis=1)
    ntri = triang.triangles.shape[0]
    mask = []
    for i in range(0, ntri):
        triangle = triang.triangles[i]
        pointX = triang.x[triangle].mean()
        pointY = triang.y[triangle].mean()
        point = (pointX, pointY)
        mask.append(not domainContainsPoint(outerPolygons, innerPolygons, point))
    triang.set_mask(mask)

def addThawingMask(triang, v):
    ntri = triang.triangles.shape[0]
    for i in range(0, ntri):
        if triang.mask[i]:
            continue
        num0 = 0
        num1 = 0
        for thawedPart in v[triang.triangles[i]]:
            if thawedPart == 0.0:
                num0 += 1
            else:
                if thawedPart == 1.0:
                    num1 += 1
                else:
                    break
        if num0 == 3 or num1 == 3:
            triang.mask[i] = True


def formatPercent(x, i):
    return str(int(round(x*100.0, 0))) + "\,\%"

percentformatter = FuncFormatter(formatPercent)

print("*** Parsing '%s' ***" % args.FILE.name)
args.FILE.readline()
args.FILE.readline()
x, y, t, v = readpart(args.FILE).T
outerPolygonsPoints = readpart(args.FILE)
outerPolygonsX, outerPolygonsY = outerPolygonsPoints.T
innerPolygonsPoints = readpart(args.FILE)
innerPolygonsX, innerPolygonsY = ([], [])
if innerPolygonsPoints != []:
    innerPolygonsX, innerPolygonsY = innerPolygonsPoints.T
boundsX, boundsY, boundsT, boundsV = readpart(args.FILE).T
args.FILE.close()

fullX = numpy.concatenate((x, boundsX))
fullY = numpy.concatenate((y, boundsY))
fullT = numpy.concatenate((t, boundsT))
fullV = numpy.concatenate((v, boundsV))

outerPolygons = pathes(outerPolygonsPoints)
assert(len(outerPolygons) == 1)
innerPolygons = pathes(innerPolygonsPoints)

knownTriang = tri.Triangulation(x, y)
maskByDomain(knownTriang, outerPolygons, innerPolygons)
mask = TriAnalyzer(knownTriang).get_flat_tri_mask()
for i in range(0, len(knownTriang.mask)):
    mask[i] |= knownTriang.mask[i]
knownTriang.set_mask(mask)

fullTriang = tri.Triangulation(fullX, fullY)
mask = TriAnalyzer(fullTriang).get_flat_tri_mask()
fullTriang.set_mask(mask)

minX = outerPolygonsX.min()
maxX = outerPolygonsX.max()
minY = outerPolygonsY.min()
maxY = outerPolygonsY.max()

print "Plotting"

plt.figure()
plt.xlim(xmin=minX, xmax=maxX)
plt.ylim(ymin=maxY, ymax=minY)
plt.gca().set_aspect('equal')
#plt.triplot(knownTriang, lw=0.5, color='white')

mainPath = outerPolygons[0]
mainPatch = PathPatch(mainPath, facecolor='none', lw=1)
mainPatch.set_zorder(6)
plt.grid(True, ls='-', c='#e0e0e0')
#[line.set_zorder(15) for line in plt.axes().lines]
plt.gca().add_patch(mainPatch)

#addThawingMask(fullTriang, fullV)

is_thawed_parts_plot = False

cmap = plt.cm.jet if not is_thawed_parts_plot else plt.cm.gist_ncar

if is_thawed_parts_plot:
    V = [i/20.0 for i in range(0, 21)]
    V[0] = 1e-15
    V[-1] = 1.0 - V[0]
else:
    V = [i/2.0 for i in range(-20, 21)]

cs = plt.tricontourf(fullTriang, 
                     fullT if not is_thawed_parts_plot else fullV,
                     V, cmap=cmap, extend='both', antialiased=False) # с антиалиасингом хуже
for collection in cs.collections:
    collection.set_clip_path(mainPatch)

if not is_thawed_parts_plot:
    colorbar = plt.colorbar(ticks=MultipleLocator(base=1.0))
    colorbar.set_label(u"Температура $T$, $^\circ$C")
else:
    colorbar = plt.colorbar(ticks=V, format=percentformatter)
    colorbar.set_label(u"Относительный объём талой фазы $V_{th}$")

#addThawingMask(knownTriang, v)

#plt.tricontour(tri_refi, z_test_refi)
#plt.title('tricontourf, tricontour (refine->mask)')
filename='plot.png'
plt.savefig(filename, dpi=200, bbox_inches='tight')
print('Saved ' + filename + "!")

vals = t if not is_thawed_parts_plot else v

refine_for_contrours = False

if refine_for_contrours:
    refiner = UniformTriRefiner(knownTriang)
    knownTriang, vals = refiner.refine_field(vals, subdiv=2)

cs = plt.tricontour(knownTriang, 
                    vals,
                    V,
                    colors=['0.25', '0.5', '0.5', '0.5', '0.5'],
                    linewidths=[1.0, 0.5, 0.5, 0.5, 0.5])

#plt.clabel(cs,
#           fontsize=4, inline=True, fmt=r'$%1.1f$',
#           use_clabeltext=True)

for collection in cs.collections:
    collection.set_clip_path(mainPatch)
    collection.set_zorder(5)
filename='plot_with_contours.png'
plt.savefig(filename, dpi=200, bbox_inches='tight')
print('Saved ' + filename + "!")

plt.close()
