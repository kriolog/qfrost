#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from PyQt5.QtCore import QObject, pyqtSignal

import numpy
from matplotlib import pyplot as plt
from matplotlib.ticker import MultipleLocator
import matplotlib.tri as tri
from matplotlib.path import Path
import matplotlib.colors as col
from matplotlib.colorbar import Colorbar
from matplotlib.patches import PathPatch
from matplotlib.tri import UniformTriRefiner, TriAnalyzer
import itertools

plt.rc('font', **{'family' : 'Droid Sans'})

def _pathes(points):
    """Набор полилиний (matplotlib.path.Path), полученный из списка точек.

    points: список точек, по которым заданы полилинии. Все полилинии должны быть
            замкнуты (координаты первой и последней точек совпадают), ибо именно
            благодаря этому и идёт выделение отдельных полилиний в общем списке.
    """
    result = []
    polygon = []
    for point in points:
        polygon.append(point)
        if len(polygon) > 2 and (point == polygon[0]).all():
            result.append(Path(polygon))
            polygon = []
    return result


def _any_polygon_contains_point(polygons, point):
    """Содержится ли точка point в каком-либо полигоне из списка polygons.
    """
    for polygon in polygons:
        if polygon.contains_point(point):
            return True
    return False


def _domain_contains_point(outer_polygons, inner_polygons, point):
    """Относится ли точка point к расчётной области.

    Считается, что точка point попадает в расчётную область, если её содержит
    любой внешний полигон (outer_polygons) и не содержат дырки (inner_polygons).
    """
    if not _any_polygon_contains_point(outer_polygons, point):
        return False
    # FIXME проверять только по дыркам, соответствующим нашему внешн. полигону
    return not _any_polygon_contains_point(inner_polygons, point)


def _add_mask_domain(triang, outer_polygons, inner_polygons):
    """Прячет треугольники из triang в соответствие с формой расчётной области.

    Треугольник скрывается, если его центр не попадает внутрь расчётной области,
    заданной внутренними и внешними полигонами (outer_polygons/inner_polygons).
    """
    ntri = triang.triangles.shape[0]
    if triang.mask is None:
        triang.set_mask(numpy.zeros(ntri, dtype=bool))
    for i in range(0, ntri):
        if triang.mask[i]:
            continue
        triangle = triang.triangles[i]
        point = (triang.x[triangle].mean(),
                 triang.y[triangle].mean())
        if not _domain_contains_point(outer_polygons, inner_polygons, point):
            triang.mask[i] = True


def _add_mask_thawing(triang, v):
    """Прячет треугольники из triang, в которых сейчас не идёт фазовый переход.

    Треугольник скрывается, если относительный объём талой фазы (из v) во всех
    его вершинах равны 0 или 1 (т.е. если грунт здесь полностью талый/мёрзлый).
    """
    ntri = triang.triangles.shape[0]
    if triang.mask is None:
        triang.set_mask(numpy.zeros(ntri, dtype=bool))
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


def _add_mask_flat(triang):
    """Прячет некорректные (плоские) треугольники из triang.

    Для поиска таких треугольников используется matplotlib.tri.TriAnalyzer.
    """
    mask = TriAnalyzer(triang).get_flat_tri_mask()
    if triang.mask is None:
        triang.set_mask(mask)
    else:
        for i in range(0, triang.triangles.shape[0]):
            if mask[i]:
                triang.mask[i] = True


def _polygons(points, indexes):
    """Полигоны, полученные разделением points по индексам из indexes."""
    result = []
    cur_polygon = []
    for i in range(len(points)):
        if i in indexes and cur_polygon != []:
            result.append(cur_polygon)
            cur_polygon = []
        cur_polygon.append(points[i])
    if cur_polygon != []:
        result.append(cur_polygon)
    return result


def _segments(poly):
    """Все стороны полигона poly (в виде попарного перечисления точек)."""
    return zip(poly, poly[1:] + [poly[0]])


def _is_clockwise(poly):
    """Ориентация полигона poly - идут ли его точки по часовой стрелке."""
    clockwise = False
    if (sum(x0*y1 - x1*y0 for ((x0, y0), (x1, y1)) in _segments(poly))) < 0:
        clockwise = not clockwise
    return clockwise


def _fix_polygon_orientation(poly, clockwise):
    """Если ориентация poly не соответствует clockwise, переворачивает его."""
    if _is_clockwise(poly) != clockwise:
        poly.reverse()


def _domain_path(outer_polygons_points, inner_polygons_points,
                 outer_polygons_indexes, inner_polygons_indexes):
    """ Полная траектория для расчётной области (matplotlib.path.Path).

    outer_polygons_points: точки всех внешних полигонов.
    inner_polygons_points: точки всех внутренних полигонов.
    outer_polygons_indexes: индексы первых точек из outer_polygons_points.
    inner_polygons_indexes: индексы первых точек из inner_polygons_points.

    Списки точек разбиваются на отдельные полигоны (по элементам, номер которых
    указан в соответствующем списке индексов). Далее ориентация каждого полигона
    корректируется (внешние полигоны - по часовой стрелке, внутренние - против)
    и все они объединяются в возвращаемую тректорию (где для вершин, начинающих
    каждый полигон, используется код MOVETO вместо LINETO).
    """
    if outer_polygons_points == []:
        return Path()

    outer_polygons = _polygons(outer_polygons_points, outer_polygons_indexes)
    for poly in outer_polygons:
        _fix_polygon_orientation(poly, True)

    inner_polygons = _polygons(inner_polygons_points, inner_polygons_indexes)
    for poly in inner_polygons:
        _fix_polygon_orientation(poly, False)

    points = list(itertools.chain.from_iterable(outer_polygons))
    if inner_polygons != []:
        numpy.append((points, list(itertools.chain.from_iterable(inner_polygons))))

    codes = numpy.ones(len(points), dtype=Path.code_type) * Path.LINETO
    codes[outer_polygons_indexes] = Path.MOVETO
    codes[inner_polygons_indexes + len(outer_polygons_points)] = Path.MOVETO
    return Path(points, codes)


class ContourPlot(QObject):
    __full_triang = None
    __known_triang = None

    __cmapT = None
    __cmapV = None

    __Vt = None
    __Vv = None

    __domain_patch = None

    __fig = None
    __axes = None

    __colorbar = None

    __contourf_t = None
    __contour_t = None

    __show_contourf_t = True
    __show_contour_t = True
    __show_clabel_t = True

    stateChanged = pyqtSignal('QString')
    stateCleared = pyqtSignal()

    def __init__(self, figure):
        """В конструкторе настраиваются цветовые шкалы и ключевые значения."""
        QObject.__init__(self)

        self.__fig = figure
        self.__axes = figure.add_subplot(111)

        # Цвета шкалы температуры
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
        self.__cmapT = col.LinearSegmentedColormap('QFrostT', ctdictT)

        # Цвета шкалы относительного объёма талой фазы
        startcolor = (0.0, 0.3, 0.0, 0.0)
        midcolor = (0.0, 0.3, 0.0, 1.0)
        endcolor = startcolor
        self.__cmapV = col.LinearSegmentedColormap.from_list('QFrostV',
                                                           [startcolor,
                                                            startcolor,
                                                            startcolor,
                                                            midcolor,
                                                            endcolor,
                                                            endcolor,
                                                            endcolor])

        # Ключевые значения температуры
        self.__Vt = [i/2.0 for i in range(-20, 21)]

        # Ключевые значения относительного объёма талой фазы
        self.__Vv = [i/20.0 for i in range(0, 21)]
        self.__Vv[0] = 1e-15
        self.__Vv[-1] = 1.0 - self.__Vv[0]

        self.__axes.set_aspect('equal')
        self.__axes.grid(True, ls='-', c='#e0e0e0')


    def clear(self):
        if self.__domain_patch is not None:
            self.__domain_patch.remove()
            self.__domain_patch = None

        if self.__contourf_t is not None:
            for collection in self.__contourf_t.collections:
                collection.remove()
            self.__contourf_t = None

        if self.__contour_t is not None:
            for _ in range(0, len(self.__contour_t.labelCValues)):
                self.__contour_t.pop_label()

            for collection in self.__contour_t.collections:
                collection.remove()
            self.__contour_t = None

        self.__fig.canvas.draw()


    def set_mesh(self,
                 block_centers_x, block_centers_y,
                 outer_polygons_points, outer_polygons_indexes,
                 inner_polygons_points, inner_polygons_indexes,
                 bounds_x, bounds_y):
        """Рассчитывает и подготавливает триангуляцию для указанной сетки."""
        self.stateChanged.emit('Initial mesh processing...')
        outer_polygons_x, outer_polygons_y = outer_polygons_points.T
        inner_polygons_x, inner_polygons_y = ([], [])
        if inner_polygons_points != []:
            inner_polygons_x, inner_polygons_y = inner_polygons_points.T

        outer_polygons = _pathes(outer_polygons_points)
        inner_polygons = _pathes(inner_polygons_points)

        min_x = outer_polygons_x.min()
        max_x = outer_polygons_x.max()
        min_y = outer_polygons_y.min()
        max_y = outer_polygons_y.max()
        self.__axes.set_xlim(min_x, max_x)
        self.__axes.set_ylim(max_y, min_y)
        # TODO удалять точки вне min_x..max_x; min_y..max_y (clabel фейлит)

        full_x = numpy.concatenate((block_centers_x, bounds_x))
        full_y = numpy.concatenate((block_centers_y, bounds_y))

        domain_path = _domain_path(outer_polygons_points, inner_polygons_points,
                                   outer_polygons_indexes, inner_polygons_indexes)
        self.__domain_patch = PathPatch(domain_path, facecolor='none', lw=1)
        self.__domain_patch.set_zorder(7)
        self.__axes.add_patch(self.__domain_patch)

        self.stateChanged.emit('Triangulating (known points)...')
        self.__known_triang = tri.Triangulation(block_centers_x, block_centers_y)

        self.stateChanged.emit('Triangulating (all points)...')
        self.__full_triang = tri.Triangulation(full_x, full_y)

        self.stateChanged.emit('Updating triangulation mask (detecting outer triangles)...')
        _add_mask_domain(self.__known_triang, outer_polygons, inner_polygons)

        self.stateChanged.emit('Updating triangulation mask (detecting flat triangles)...')
        _add_mask_flat(self.__known_triang)
        _add_mask_flat(self.__full_triang)

        self.stateCleared.emit()


    def plot_data(self,
                  temperatures, thawed_parts,
                  temperatures_bounds, thawed_parts_bounds):
        """Делает построение по указанным данным (сетка должна быть задана)."""
        temperatures_full = numpy.concatenate((temperatures, temperatures_bounds))
        thawed_parts_full = numpy.concatenate((thawed_parts, thawed_parts_bounds))

        self.stateChanged.emit('Plotting temperatures map...')
        self.__contourf_t = tri.TriContourSet(self.__axes,
                                              self.__full_triang, temperatures_full,
                                              self.__Vt, cmap=self.__cmapT,
                                              filled=True,
                                              extend='both',
                                              antialiased=False)
        # антиалиасинг красив, если цвета сильно меняются, иначе всё портят щели

        for collection in self.__contourf_t.collections:
            collection.set_clip_path(self.__domain_patch)

        # Добавляем шкалу температур (если ещё не сделали этого)
        if self.__colorbar is None:
            self.__colorbar = self.__axes.figure.colorbar(self.__contourf_t,
                                                          ticks=MultipleLocator(base=1.0))
            self.__colorbar.set_label(u"Температура $T$, °C")

        self.stateChanged.emit('Plotting temperatures contours...')
        self.__contour_t = tri.TriContourSet(self.__axes,
                                             self.__known_triang, temperatures,
                                             self.__Vt,
                                             filled=False,
                                             colors=['0.25', '0.5', '0.5', '0.5', '0.5'],
                                             linewidths=[1.0, 0.5, 0.5, 0.5, 0.5])

        self.__contour_t.clabel(fontsize=8, fmt=r'$%1.1f$')

        for collection in self.__contour_t.collections:
            collection.set_clip_path(self.__domain_patch)
            collection.set_zorder(5)

        self.stateChanged.emit('Drawing...')
        self.__fig.canvas.draw()

        self.stateCleared.emit()
