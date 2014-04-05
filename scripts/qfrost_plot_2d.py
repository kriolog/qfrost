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

x = numpy.concatenate((x, boundsX))
y = numpy.concatenate((y, boundsY))
t = numpy.concatenate((t, boundsT))
v = numpy.concatenate((v, boundsV))

outerPolygons = pathes(outerPolygonsPoints)
assert(len(outerPolygons) == 1)
innerPolygons = pathes(innerPolygonsPoints)

#additionalX = numpy.concatenate((innerPolygonsX, outerPolygonsX))
#additionalY = numpy.concatenate((innerPolygonsY, outerPolygonsY))
#fullX = numpy.concatenate((x, additionalX))
#fullY = numpy.concatenate((y, additionalY))

#triang = tri.Triangulation(fullX, fullY)
#maskByDomain(triang, outerPolygons, innerPolygons)

knownTriang = tri.Triangulation(x, y)
#maskByDomain(knownTriang, outerPolygons, innerPolygons)

minX = outerPolygonsX.min()
maxX = outerPolygonsX.max()
minY = outerPolygonsY.min()
maxY = outerPolygonsY.max()

#print "Refining"
#refiner = tri.UniformTriRefiner(knownTriang)
#tri_refi, t_refi = refiner.refine_field(t, subdiv=2)

#print "Masking"
#maskByDomain(tri_refi, outerPolygons, innerPolygons)

print "Plotting"

plt.figure()
plt.xlim(xmin=minX, xmax=maxX)
plt.ylim(ymin=maxY, ymax=minY)
plt.gca().set_aspect('equal')
V = [i/2.0 for i in range(-20, 21)]
#plt.triplot(knownTriang, lw=0.3, color='white')

mainPath = outerPolygons[0]
mainPatch = PathPatch(mainPath, facecolor='none', lw=2)
mainPatch.set_zorder(10)
plt.gca().add_patch(mainPatch)

cs = plt.tricontourf(knownTriang, t, V, cmap=plt.cm.jet, extend='both')
for collection in cs.collections:
    collection.set_clip_path(mainPatch)

colorbar = plt.colorbar(ticks=MultipleLocator(base=1.0))
colorbar.set_label(u"Температура $T$, $^\circ$C")


#cs = plt.tricontour(knownTriang, t, V,
#               colors=['0.25', '0.5', '0.5', '0.5', '0.5'],
#               linewidths=[1.0, 0.5, 0.5, 0.5, 0.5])

#for collection in cs.collections:
#    collection.set_clip_path(mainPatch)

#plt.clabel(cs,
#           fontsize=6, inline=True, fmt=r'$%1.1f$',
#           use_clabeltext=True)
#cs = plt.contourf(data)


#plt.tricontour(tri_refi, z_test_refi)
#plt.title('tricontourf, tricontour (refine->mask)')
filename='plot.png'
plt.savefig(filename, dpi=200, bbox_inches='tight')
print('Saved ' + filename + "!")
plt.close()

'''
t = t.swapaxes(0, 1)
v = v.swapaxes(0, 1)
stuff_extension = '.pdf' if args.scalable else '.png'
scale_factor = 1.0 if args.scalable else args.factor
print('*** Plotting T ***')
plotstuff(dates, depths, t, False,
          'temperatures' + stuff_extension, scale_factor, args.depth, t)
'''
