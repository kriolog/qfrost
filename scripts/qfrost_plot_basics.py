#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from matplotlib import rc
rc('font', **{'family' : 'Droid Sans'})

from matplotlib.ticker import FuncFormatter
from matplotlib.ticker import MultipleLocator
from matplotlib.dates import MonthLocator
from matplotlib.dates import DateFormatter
import matplotlib.colors as Colors
from matplotlib.colorbar import Colorbar

from enum import Enum, unique


@unique
class QFrostVType(Enum):
    """
    Перечень типов графиков - то есть параметров, которые можно построить.
    """
    none        = 0 # Отсутствующее построение
    temperature = 1 # Построение температуры
    thawed_part = 2 # Построение отн. объём талой фазы


class QFrostPlot():
    """
    Класс для подготовки к построению. Поможет настроить оси, шкалы, сетки и пр.
    """

    #============================== Шкалы (цвет) ==============================#
    @staticmethod
    def ColormapTemperature():
        """Шкала температуры."""
        ctdictT = {'red': [(0.0,  0.0, 0.0),
                           (0.45, 0.0, 0.0),
                           (0.5,  1.0, 1.0),
                           (0.55, 1.0, 1.0),
                           (1.0,  0.5, 0.5)],

                'green':  [(0.0,  0.0, 0.0),
                           (0.25, 0.0, 0.0),
                           (0.45, 1.0, 1.0),
                           (0.55, 1.0, 1.0),
                           (0.75, 0.0, 0.0),
                           (1.0,  0.0, 0.0)],

                'blue':   [(0.0,  0.5, 0.5),
                           (0.45, 1.0, 1.0),
                           (0.5,  1.0, 1.0),
                           (0.55, 0.0, 0.0),
                           (1.0,  0.0, 0.0)]}

        return Colors.LinearSegmentedColormap('QFrostT', ctdictT)


    @staticmethod
    def ColormapThawedPart():
        """Шкала отн. объёма талой фазы (для полной карты)."""
        startcolor = (0.0, 100.0/255.0, 0.0)
        endcolor = (0.0, 1.0, 0.0)
        return Colors.LinearSegmentedColormap.from_list('QFrostV',
                                                        [startcolor,
                                                         endcolor])


    @staticmethod
    def ColormapFront():
        """Шкала отн. объёма талой фазы (для выделения фронта)."""
        startcolor = (0.0, 0.3, 0.0, 0.0)
        midcolor = (0.0, 0.3, 0.0, 1.0)
        endcolor = startcolor
        return Colors.LinearSegmentedColormap.from_list('QFrostV2',
                                                        [startcolor,
                                                         startcolor,
                                                         startcolor,
                                                         midcolor,
                                                         endcolor,
                                                         endcolor,
                                                         endcolor])

    @staticmethod
    def Colormap(vtype):
        """Шкала для T или Vbf (исходя из vtype)."""
        if not isinstance(vtype, QFrostVType):
            raise TypeError("Argument 'vtype' must be QFrostVType (not %s)."
                            % type(vtype).__name__)
        if vtype is QFrostVType.thawed_part:
            return QFrostPlot.ColormapThawedPart()
        elif vtype is QFrostVType.temperature:
            return QFrostPlot.ColormapTemperature()
        else:
            raise ValueError("Argument 'vtype' can not be 'none'.")
    #==========================================================================#


    #========= Ключевые значения (уровни изолиний и/или смены цветов) =========#
    @staticmethod
    def LevelsTemperature():
        """Ключевые значения температуры (для изолиний и межцветовых границ)."""
        return [i/2.0 for i in range(-20, 21)]


    @staticmethod
    def LevelsThawedPart():
        """Ключевые значения Vth (для изолиний и межцветовых границ)."""
        result = [i/20.0 for i in range(0, 21)]
        result[0] = 1e-15
        result[-1] = 1.0 - result[0]
        return result


    @staticmethod
    def Levels(vtype):
        """Ключевые значения для T или Vbf (исходя из vtype)."""
        if not isinstance(vtype, QFrostVType):
            raise TypeError("Argument 'vtype' must be QFrostVType (not %s)."
                            % type(vtype).__name__)
        if vtype is QFrostVType.thawed_part:
            return QFrostPlot.LevelsThawedPart()
        elif vtype is QFrostVType.temperature:
            return QFrostPlot.LevelsTemperature()
        else:
            raise ValueError("Argument 'vtype' can not be 'none'.")
    #==========================================================================#


    #================================ Локаторы ================================#
    @staticmethod
    def LocatorBasicTemperature():
        """Локатор изолиний температуры (соответствует LevelsTemperature())."""
        return MultipleLocator(base=0.5)


    @staticmethod
    def LocatorColorbarTemperature():
        """Локатор зарубок на шкале температуры."""
        return MultipleLocator(base=1.0)


    @staticmethod
    def LocatorBasicThawedPart():
        """Локатор изолиний Vth (соответствует LevelsThawedPart())."""
        return MultipleLocator(base=0.05)


    @staticmethod
    def LocatorColorbarThawedPart():
        """Локатор зарубок на шкале отн. объёма талой фазы."""
        return MultipleLocator(base=0.1)
    #==========================================================================#


    #======================= Шкалы (создание столбиков) =======================#
    @staticmethod
    def ColorbarTemperature(csetf, fig):
        """Создаёт и возвращает столбик шкалы T для csetf в фигуре fig."""
        result = fig.colorbar(csetf,
                              ticks=QFrostPlot.LocatorColorbarTemperature(),
                              use_gridspec=True)
        result.set_label('Температура $T$, °C')
        return result


    @staticmethod
    def ColorbarThawedPart(csetf, fig):
        """Создаёт и возвращает столбик шкалы Vth для csetf в фигуре fig."""
        result = fig.colorbar(csetf,
                              ticks=QFrostPlot.LocatorColorbarThawedPart(),
                              format=QFrostPlot.PercentFormatter(),
                              use_gridspec=True)
        result.set_label('Относительный объём талой фазы $V_{th}$')
        return result


    @staticmethod
    def Colorbar(vtype, csetf, fig):
        """Столбик шкалы для T или Vbf (исходя из vtype)."""
        if not isinstance(vtype, QFrostVType):
            raise TypeError("Argument 'vtype' must be QFrostVType (not %s)."
                            % type(vtype).__name__)
        if vtype is QFrostVType.thawed_part:
            return QFrostPlot.ColorbarThawedPart(csetf, fig)
        elif vtype is QFrostVType.temperature:
            return QFrostPlot.ColorbarTemperature(csetf, fig)
        else:
            raise ValueError("Argument 'vtype' can not be 'none'.")
    #==========================================================================#


    #==================== Оси (подпись, лимит, тики и пр.) ====================#
    @staticmethod
    def SetupAxisMonth(axis):
        """
        Настраивает ось месяцев - подпись и осн. тики (локатор и форматтер).
        Возвращает добавленную к оси подпись.
        """
        axis.set_major_locator(MonthLocator())
        axis.set_major_formatter(DateFormatter('%m'))
        return axis.set_label_text('Месяц')


    @staticmethod
    def SetupAxisDepth(axis):
        """
        Настраивает ось глубины - подпись, осн.+доп. тики (локаторы) и инверсия.
        Возвращает добавленную к оси подпись.
        """
        axis.set_minor_locator(MultipleLocator(base=0.2))
        axis.set_major_locator(MultipleLocator())
        if axis.axis_name is 'x':
            axis.axes.invert_xaxis()
        else:
            axis.axes.invert_yaxis()
        return axis.set_label_text('Глубина, м')


    @staticmethod
    def SetupAxisTemperature(axis):
        """
        Настраивает ось температуры - подпись [и (пока что) ничего, помимо неё].
        Возвращает добавленную к оси подпись.
        """
        return axis.set_label_text('Температура, °C')


    @staticmethod
    def SetupAxisThawedPart(axis):
        """
        Настраивает ось Vth - цветная подпись, осн. тики (локатор+цвет), лимиты.
        Возвращает добавленную подпись.
        """
        AXIS_COLOR = 'blue'  # Цвет для подписи и для тиков
        axis.set_major_formatter(QFrostPlot.PercentFormatter())
        if axis.axis_name is 'x':
            axis.axes.set_xlim(0.0, 1.0)
        else:
            axis.axes.set_ylim(0.0, 1.0)
        [ticklabel.set_color(AXIS_COLOR) for ticklabel in axis.get_ticklabels()]
        return axis.set_label_text('Относительный объём талой фазы $V_{th}$',
                                   color=AXIS_COLOR)
    #==========================================================================#


    #================================= Сетка ==================================#
    @staticmethod
    def SetupGridFor1D(axes):
        """Показывает сетку внутри axes (вариант для обычных графиков)."""
        axes.set_axisbelow(True) # сетка под графиками
        axes.grid(True, ls='-', c='#a0a0a0')


    @staticmethod
    def SetupGridFor2D(axes):
        """Показывает сетку внутри axes (вариант для двухмерных построений)."""
        axes.grid(True, ls='-', c='#e0e0e0')
    #==========================================================================#


    #============================ Подписи изолиний ============================#
    @staticmethod
    def _LabelContours(cset, fontsize, fmt):
        """Подписывает изолинии.
        cset: изолинии для подписей - ContourSet. Не могут быть [уже] подписаны!
        fontsize: размер шрифта для создаваемых подписей - int.
        fmt: формат для подписей - словарь, строка вида '%1.2f' или Formatter.
        """
        if cset.labelTexts:
            raise ValueError('Will not add labels to already labeled contours.')
        cset.clabel(fontsize=fontsize, fmt=fmt)


    @staticmethod
    def LabelContoursTemperature(cset, fontsize=6):
        """Подписывает изолинии T шрифтом данного размера."""
        QFrostPlot._LabelContours(cset, fontsize, '%1.1f')


    @staticmethod
    def LabelContoursThawedPart(cset, fontsize=6):
        """Подписывает изолинии Vth шрифтом данного размера."""
        QFrostPlot._LabelContours(cset, fontsize, QFrostPlot.PercentFormatter())


    @staticmethod
    def LabelContours(vtype, cset, fontsize=6):
        """Подписывает изолинии T или Vbf (по vtype) шрифтом данного размера."""
        if not isinstance(vtype, QFrostVType):
            raise TypeError("Argument 'vtype' must be QFrostVType (not %s)."
                            % type(vtype).__name__)
        if vtype is QFrostVType.thawed_part:
            QFrostPlot.LabelContoursThawedPart(cset, fontsize)
        elif vtype is QFrostVType.temperature:
            QFrostPlot.LabelContoursTemperature(cset, fontsize)
        else:
            raise ValueError("Argument 'vtype' can not be 'none'.")
    #==========================================================================#


    #================================= Прочее =================================#
    @staticmethod
    def PercentFormatter():
        """Форматтер для показа дробных чисел в виде процента (со значком %)."""
        def _format_percent(x, i):
            return str(int(round(x * 100.0, 0))) + '%'
        return FuncFormatter(_format_percent)


    @staticmethod
    def ContourBasicColors():
        """Набор цветов для изолиний (в пару к ContourBasicLineWidths())."""
        return ['0.25', '0.5', '0.5', '0.5', '0.5']


    @staticmethod
    def ContourBasicLineWidths():
        """Набор значений толщины изолиний (в пару к ContourBasicColors())."""
        return [1.0, 0.5, 0.5, 0.5, 0.5]
    #==========================================================================#
