#!/usr/bin/env python3
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
import itertools

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


def readpart(f, dtype=numpy.float):
    lines = []
    while True:
        line = f.readline().rstrip()
        if line == "" or line[0].isalpha():
            return numpy.array([x.split('\t') for x in lines], dtype=dtype)
        lines.append(line)


def pathes(points):
    result = []
    polygon = []
    for point in points:
        polygon.append(point)
        if len(polygon) > 2 and (point == polygon[0]).all():
            result.append(Path(polygon))
            polygon = []
    return result


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
    ntri = triang.triangles.shape[0]
    mask = []
    for i in range(0, ntri):
        triangle = triang.triangles[i]
        pointX = triang.x[triangle].mean()
        pointY = triang.y[triangle].mean()
        point = (pointX, pointY)
        mask.append(not domainContainsPoint(outerPolygons,
                                            innerPolygons,
                                            point))
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


def polygons(points, startPoints):
    result = []
    curPoly = []
    for i in range(len(points)):
        if i in startPoints and curPoly != []:
            result.append(curPoly)
            curPoly = []
        curPoly.append(points[i])
    if curPoly != []:
        result.append(curPoly)
    return result


def segments(poly):
    """A sequence of (x,y) numeric coordinates pairs """
    return zip(poly, poly[1:] + [poly[0]])


def isClockwise(poly):
    clockwise = False
    if (sum(x0*y1 - x1*y0 for ((x0, y0), (x1, y1)) in segments(poly))) < 0:
        clockwise = not clockwise
    return clockwise


def fixPointsOrder(points, mustBeClockwise):
    if isClockwise(points) != mustBeClockwise:
        points.reverse()


def fullPath(outerPolygonsPoints, innerPolygonsPoints,
             outerPolygonsStartPoints, innerPolygonsStartPoints):
    if outerPolygonsPoints == []:
        return Path()

    outerPolygons = polygons(outerPolygonsPoints, outerPolygonsStartPoints)
    for poly in outerPolygons:
        fixPointsOrder(poly, True)

    innerPolygons = polygons(innerPolygonsPoints, innerPolygonsStartPoints)
    for poly in innerPolygons:
        fixPointsOrder(poly, False)

    allPoints = list(itertools.chain.from_iterable(outerPolygons))
    if innerPolygons != []:
        allPoints = numpy.concatenate((allPoints, list(itertools.chain.from_iterable(innerPolygons))))

    codes = numpy.ones(len(allPoints), dtype=Path.code_type) * Path.LINETO
    codes[outerPolygonsStartPoints] = Path.MOVETO
    codes[innerPolygonsStartPoints + len(outerPolygonsPoints)] = Path.MOVETO
    return Path(allPoints, codes)


print("*** Parsing '%s' ***" % args.FILE.name)
args.FILE.readline()
args.FILE.readline()
x, y, t, v = readpart(args.FILE).T
outerPolygonsPoints = readpart(args.FILE)
outerPolygonsX, outerPolygonsY = outerPolygonsPoints.T
outerPolygonsStartPoints = readpart(args.FILE, numpy.int)
innerPolygonsPoints = readpart(args.FILE)
innerPolygonsX, innerPolygonsY = ([], [])
innerPolygonsStartPoints = readpart(args.FILE, numpy.int)
if innerPolygonsPoints != []:
    innerPolygonsX, innerPolygonsY = innerPolygonsPoints.T
boundsX, boundsY, boundsT, boundsV = readpart(args.FILE).T
args.FILE.close()

outerPolygons = pathes(outerPolygonsPoints)
innerPolygons = pathes(innerPolygonsPoints)

minX = outerPolygonsX.min()
maxX = outerPolygonsX.max()
minY = outerPolygonsY.min()
maxY = outerPolygonsY.max()

# TODO удалять попадающие за пределы minX..maxX minY..maxY точки (clabel фейлит)

fullX = numpy.concatenate((x, boundsX))
fullY = numpy.concatenate((y, boundsY))
fullT = numpy.concatenate((t, boundsT))
fullV = numpy.concatenate((v, boundsV))

print("Creating main path")
mainPath = fullPath(outerPolygonsPoints, innerPolygonsPoints,
                    outerPolygonsStartPoints, innerPolygonsStartPoints)

print("Triangulating known points")
knownTriang = tri.Triangulation(x, y)

print("Triangulating all points")
fullTriang = tri.Triangulation(fullX, fullY)

print("Masking known triangles by domain")
maskByDomain(knownTriang, outerPolygons, innerPolygons)

print("Masking by TriAnalyzer")
mask = TriAnalyzer(knownTriang).get_flat_tri_mask()
for i in range(0, len(knownTriang.mask)):
    mask[i] |= knownTriang.mask[i]
knownTriang.set_mask(mask)

mask = TriAnalyzer(fullTriang).get_flat_tri_mask()
fullTriang.set_mask(mask)

print("Plotting")

plt.figure()
plt.xlim(xmin=minX, xmax=maxX)
plt.ylim(ymin=maxY, ymax=minY)
plt.gca().set_aspect('equal')
#plt.triplot(knownTriang, lw=0.5, color='white')

mainPatch = PathPatch(mainPath, facecolor='none', lw=1)
mainPatch.set_zorder(7)
plt.gca().add_patch(mainPatch)

plt.grid(True, ls='-', c='#e0e0e0')
#[line.set_zorder(15) for line in plt.axes().lines]

#addThawingMask(fullTriang, fullV)

ctdictT = {'red':   [(0.0,  0.0, 0.0),
                     (0.45, 0.0, 0.0),
                     (0.5,  1.0, 1.0),
                     (0.55, 1.0, 1.0),
                     (1.0,  0.5, 0.5)],

           'green': [(0.0,  0.0, 0.0),
                     (0.25, 0.0, 0.0),
                     (0.45, 1.0, 1.0),
                     (0.55, 1.0, 1.0),
                     (0.75, 0.0, 0.0),
                     (1.0,  0.0, 0.0)],

           'blue':  [(0.0,  0.5, 0.5),
                     (0.45, 1.0, 1.0),
                     (0.5,  1.0, 1.0),
                     (0.55, 0.0, 0.0),
                     (1.0,  0.0, 0.0)]}

cmapT = col.LinearSegmentedColormap('QFrostT', ctdictT)

startcolor = (0.0, 0.3, 0.0, 0.0)
midcolor = (0.0, 0.3, 0.0, 1.0)
endcolor = startcolor
cmapV = col.LinearSegmentedColormap.from_list('QFrostV', 
                                              [startcolor, startcolor, startcolor,
                                               midcolor,
                                               endcolor, endcolor, endcolor])


Vv = [i/20.0 for i in range(0, 21)]
Vv[0] = 1e-15
Vv[-1] = 1.0 - Vv[0]

Vt = [i/2.0 for i in range(-20, 21)]

print("tricontourf T")
cs = plt.tricontourf(fullTriang, fullT,
                     Vt, cmap=cmapT, extend='both',
                     antialiased=False) # антиалиасинг красив, если цвета сильно
                                        # меняются, иначе всё портят щели
for collection in cs.collections:
    collection.set_clip_path(mainPatch)

colorbar = plt.colorbar(ticks=MultipleLocator(base=1.0))
colorbar.set_label(u"Температура $T$, $^\circ$C")

print("tricontourf Vth")
cs = plt.tricontourf(knownTriang, v,
                     Vv, cmap=cmapV, extend='both', antialiased=True)
for collection in cs.collections:
    collection.set_clip_path(mainPatch)
    collection.set_zorder(6)

#addThawingMask(knownTriang, v)

#plt.tricontour(tri_refi, z_test_refi)
#plt.title('tricontourf, tricontour (refine->mask)')
print('Saving')
filename = args.FILE.name + '.pdf'
plt.savefig(filename, dpi=200, bbox_inches='tight')
print('Saved ' + filename + "!")

refine_for_contrours = False

if refine_for_contrours:
    refiner = UniformTriRefiner(knownTriang)
    knownTriang, vals = refiner.refine_field(vals, subdiv=2)

print("tricontour T")
cs = plt.tricontour(knownTriang,
                    t,
                    Vt,
                    colors=['0.25', '0.5', '0.5', '0.5', '0.5'],
                    linewidths=[1.0, 0.5, 0.5, 0.5, 0.5])

plt.clabel(cs,
           fontsize=6, inline=True, fmt=r'$%1.1f$',
           use_clabeltext=True)

for collection in cs.collections:
    collection.set_clip_path(mainPatch)
    collection.set_zorder(5)

print('Saving')
filename = args.FILE.name + '.with_contours.pdf'
plt.savefig(filename, dpi=200, bbox_inches='tight')
print('Saved ' + filename + "!")

plt.close()
