#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from PyQt5.QtCore import QObject, Qt, pyqtSignal, pyqtSlot
from PyQt5.QtWidgets import qApp

import numpy
from matplotlib import pyplot as plt
from matplotlib.ticker import FuncFormatter
from matplotlib.ticker import MultipleLocator
import matplotlib.tri as tri
from matplotlib.path import Path
import matplotlib.colors as col
from matplotlib.colorbar import Colorbar
import matplotlib.cm as cm
from matplotlib.patches import PathPatch
from matplotlib.tri import UniformTriRefiner, TriAnalyzer
import itertools

from qfrost_plot_basics import QFrostPlot

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
    __triang = None      # Результат триангуляции по центрам блоков
    __triang_ext = None  # Результат триангуляции по всем точкам (включая края)

    __temperatures = None # Температуры по центрам блоков
    __thawed_parts = None # Отн. объёмы талой фазы по центрам блоков

    __temperatures_ext = None # Температуры по центрам блоков и краям
    __thawed_parts_ext = None # Отн. объёмы талой фазы по центрам блоков и краям

    __cmap_t = None  # Цвета температуры
    __cmap_v = None  # Цвета V_th (для полной карты)
    __cmap_v2 = None # Цвета V_th (для фронтовой карты - прозрачность переменна)

    __levels_t = None # Ключевые уровни для температуры
    __levels_v = None # Ключевые уровни для V_th

    __domain_patch = None # Построение формы расчётной области (PathPatch)

    __fig = None  # Фигура, которую мы рисуем
    __axes = None # Пара осей, внутри которой идут построения

    __colorbar = None    # Цветовая шкала (она всегда одна: или t, или v)

    __contourf_t = None  # Карта температуры (ContourSet)
    __contourf_v = None  # Карта отн. объёма талой фазы (ContourSet)
    __contour_t = None   # Изолинии температуры (ContourSet)
    __contour_v = None   # Изолинии отн. объёма талой фазы (ContourSet)
    __contourf_v2 = None # Фронтовая карта отн. объёма талой фазы (ContourSet)

    __map_shown = True  # Нужно ли показывать цветовую карту
    __map_use_t = True  # Является ли источником данных для карты температура
                        # (если False, то отн. объём талой фазы)

    __iso_shown = True  # Нужно ли показывать изолинии
    __iso_use_t = True  # Является ли источником данных для изолиний температура
                        # (если False, то отн. объём талой фазы)

    __act_shown = False # Нужно ли показывать над основым построением фронтовую
                        # карту отн. объёма талой фазы

    __orig_axes_anchor = None      # Anchor у осей для построения без шкалы
    __orig_axes_subplotspec = None # Subplotspec у осей для построения без шкалы

    stateChanged = pyqtSignal('QString') # Смена статуса (т.е. этапа построения)
    stateCleared = pyqtSignal()          # Очистка статуса - построение готово

    def __init__(self, figure):
        """В конструкторе настраиваются цветовые шкалы и ключевые значения."""
        QObject.__init__(self)

        self.__fig = figure
        self.__axes = figure.add_subplot(111)


        self.__cmap_t = QFrostPlot.ColormapTemperature()
        self.__cmap_v = QFrostPlot.ColormapThawedPart()
        self.__cmap_v2 = QFrostPlot.ColormapClear()

        self.__levels_t = QFrostPlot.LevelsTemperature()
        self.__levels_v = QFrostPlot.LevelsThawedPart()

        # Обе координаты - это метры, так что пусть оси будут равномасштабны
        self.__axes.set_aspect('equal')

        # Показываем сетку
        QFrostPlot.SetupGridFor2D(self.__axes)

        # Автоматически подгоняем размер фигуры под содержимое
        self.__fig.set_tight_layout(True)

        # Запомним параметры осей, пока не добавлена цветовая шкала
        self.__orig_axes_anchor = self.__axes.get_anchor()
        self.__orig_axes_subplotspec = self.__axes.get_subplotspec()


    @pyqtSlot(bool)
    def set_map_visibility(self, visible):
        if self.__map_shown == visible:
            return
        self.__map_shown = visible
        contourf = self.__contourf_t if self.__map_use_t else self.__contourf_v
        self.__set_visibility_contour(contourf, visible)
        if visible:
            self.__colorbar_create()
        else:
            self.__colorbar_remove()
        self.__redraw()


    @pyqtSlot(bool)
    def set_map_uses_t(self, use_t):
        if self.__map_use_t == use_t:
            return
        self.__map_use_t = use_t
        if self.__map_shown:
            self.__set_visibility_contour(self.__contourf_t, use_t)
            self.__set_visibility_contour(self.__contourf_v, not use_t)
            self.__colorbar_create()
            self.__redraw()


    @pyqtSlot(bool)
    def set_iso_visibility(self, visible):
        if self.__iso_shown == visible:
            return
        self.__iso_shown = visible
        contour = self.__contour_t if self.__iso_use_t else self.__contour_v
        self.__set_visibility_contour(contour, visible)
        self.__redraw()


    @pyqtSlot(bool)
    def set_iso_uses_t(self, use_t):
        if self.__iso_use_t == use_t:
            return
        self.__iso_use_t = use_t
        if self.__iso_shown:
            self.__set_visibility_contour(self.__contour_t, use_t)
            self.__set_visibility_contour(self.__contour_v, not use_t)
            self.__redraw()


    @pyqtSlot(bool)
    def set_act_visibility(self, visible):
        if self.__act_shown == visible:
            return
        self.__act_shown = visible
        if self.__contourf_v2 is not None:
            self.__set_visibility_contour(self.__contourf_v2, visible)
            self.__redraw()


    @pyqtSlot()
    def replot_iso(self):
        if self.__contour_t is not None:
            qApp.setOverrideCursor(Qt.WaitCursor)
            for contour_set in [self.__contour_t, self.__contour_v]:
                self.__remove_contour_set(contour_set)
            self.__plot_iso()
            self.__redraw()
            qApp.restoreOverrideCursor()


    def clear(self):
        if self.__domain_patch is not None:
            self.__domain_patch.remove()
            self.__domain_patch = None

        self.__colorbar_remove()

        for contour_set in [self.__contourf_t, self.__contourf_v,
                            self.__contour_t, self.__contour_v,
                            self.__contourf_v2]:
            self.__remove_contour_set(contour_set)

        self.__redraw()


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
        self.__triang = tri.Triangulation(block_centers_x, block_centers_y)

        self.stateChanged.emit('Triangulating (all points)...')
        self.__triang_ext = tri.Triangulation(full_x, full_y)

        self.stateChanged.emit('Updating triangulation mask (detecting outer triangles)...')
        _add_mask_domain(self.__triang, outer_polygons, inner_polygons)

        self.stateChanged.emit('Updating triangulation mask (detecting flat triangles)...')
        _add_mask_flat(self.__triang)
        _add_mask_flat(self.__triang_ext)

        self.stateCleared.emit()


    def plot_data(self,
                  temperatures, thawed_parts,
                  temperatures_bounds, thawed_parts_bounds):
        """Делает построение по указанным данным (сетка должна быть задана)."""
        self.__temperatures = temperatures
        self.__thawed_parts = thawed_parts

        self.__temperatures_ext = numpy.concatenate((temperatures, temperatures_bounds))
        self.__thawed_parts_ext = numpy.concatenate((thawed_parts, thawed_parts_bounds))

        self.__plot_map()

        if self.__map_shown:
            self.__colorbar_create()

        self.__plot_act()
        self.__plot_iso()

        self.stateChanged.emit('Drawing...')
        self.__redraw()

        self.stateCleared.emit()


    def __plot_iso(self):
        self.stateChanged.emit('Plotting temperature contours...')
        self.__contour_t = self.__contour(self.__temperatures, self.__levels_t)
        QFrostPlot.LabelContoursTemperature(self.__contour_t, 8)
        if not (self.__iso_shown and self.__iso_use_t):
            self.__hide_contour(self.__contour_t)

        self.stateChanged.emit('Plotting thawed part contours...')
        self.__contour_v = self.__contour(self.__thawed_parts, self.__levels_v)
        QFrostPlot.LabelContoursThawedPart(self.__contour_v, 8)
        if not (self.__iso_shown and not self.__iso_use_t):
            self.__hide_contour(self.__contour_v)

        self.stateCleared.emit()


    def __plot_map(self):
        # Цветовая карта температуры #
        self.stateChanged.emit('Plotting temperature map...')
        self.__contourf_t = self.__contourf(self.__temperatures_ext,
                                            self.__levels_t,
                                            self.__cmap_t)
        if not (self.__map_shown and self.__map_use_t):
            self.__hide_contour(self.__contourf_t)

        # Цветовая карта отн. объёма талой фазы #
        self.stateChanged.emit('Plotting thawed part map...')
        self.__contourf_v = self.__contourf(self.__thawed_parts_ext,
                                            self.__levels_v,
                                            self.__cmap_v)
        if not (self.__map_shown and not self.__map_use_t):
            self.__hide_contour(self.__contourf_v)

        self.stateCleared.emit()


    def __plot_act(self):
        self.stateChanged.emit('Hatching thawed zone...')
        self.__contourf_v2 = self.__thawed_zone_contourf()
        if not self.__act_shown:
            self.__hide_contour(self.__contourf_v2)
        self.stateCleared.emit()


    def __redraw(self):
        self.__fig.canvas.draw()


    def __set_visibility_contour(self, contour_set, visible):
        if contour_set is not None:
            for collection in contour_set.collections:
                collection.set_visible(visible)
            for caption in contour_set.labelTexts:
                caption.set_visible(visible)


    def __hide_contour(self, contour_set):
        self.__set_visibility_contour(contour_set, False)


    def __remove_contour_set(self, contour_set):
        """Удаляет весь ContourSet contour_set (включая подписи)"""
        if contour_set is None:
            return
        for collection in contour_set.collections:
            collection.remove()
        for caption in contour_set.labelTexts:
            caption.remove()
        del contour_set
        contour_set = None


    def __contourf(self, values, levels, cmap):
        result = tri.TriContourSet(self.__axes,
                                   self.__triang_ext,
                                   values,
                                   levels,
                                   cmap=cmap,
                                   filled=True,
                                   extend='both',
                                   antialiased=False)
        for collection in result.collections:
            collection.set_clip_path(self.__domain_patch)
        return result


    def __thawed_zone_contourf(self):
        result = tri.TriContourSet(self.__axes,
                                   self.__triang_ext,
                                   self.__thawed_parts_ext,
                                   QFrostPlot.LevelsThawedPartHatch(),
                                   filled=True,
                                   hatches=QFrostPlot.ThawedPartHatches(),
                                   colors=QFrostPlot.ColormapClear(),
                                   extend='both',
                                   antialiased=False)
        for collection in result.collections:
            collection.set_clip_path(self.__domain_patch)
        return result


    def __contour(self, values, levels):
        result = tri.TriContourSet(self.__axes,
                                   self.__triang,
                                   values,
                                   levels,
                                   filled=False,
                                   colors=['0.25', '0.5', '0.5', '0.5', '0.5'],
                                   linewidths=[1.0, 0.5, 0.5, 0.5, 0.5])

        for collection in result.collections:
            collection.set_clip_path(self.__domain_patch)
            collection.set_zorder(5)
        return result


    def __colorbar_create(self):
        self.__colorbar_remove()
        if self.__map_use_t:
            if self.__contourf_t is None:
                print('Can not create colorbar without corresponding contourf!')
            else:
                self.__colorbar = QFrostPlot.ColorbarTemperature(self.__contourf_t, self.__fig)
        else:
            if self.__contourf_v is None:
                print('Can not create colorbar without corresponding contourf!')
            else:
                self.__colorbar = QFrostPlot.ColorbarThawedPart(self.__contourf_v, self.__fig)


    def __colorbar_remove(self):
        if self.__colorbar is not None:
            self.__colorbar.remove()
            del self.__colorbar
            self.__colorbar = None
            self.__axes.set_subplotspec(self.__orig_axes_subplotspec)
            self.__axes.update_params()
            self.__axes.set_anchor(self.__orig_axes_anchor)
            self.__axes.set_position(self.__axes.figbox)
