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


class QFrostPlot():
    # Функция для представления дробных чисел в виде процента (со значком %)
    @staticmethod
    def PercentFormatter():
        def _format_percent(x, i):
            return str(int(round(x * 100.0, 0))) + '%'
        return FuncFormatter(_format_percent)


    # Шкала температуры
    @staticmethod
    def ColormapTemperature():
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


    # Шкала отн. объёма талой фазы (для полной карты)
    @staticmethod
    def ColormapThawedPart():
            startcolor = (0.0, 100.0/255.0, 0.0)
            endcolor = (0.0, 1.0, 0.0)
            return Colors.LinearSegmentedColormap.from_list('QFrostV',
                                                            [startcolor,
                                                             endcolor])


    # Шкала отн. объёма талой фазы (для выделения фронта)
    @staticmethod
    def ColormapFront():
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


    # Ключевые значения температуры (для изолиний и границ между цветами)
    @staticmethod
    def LevelsTemperature():
        return [i/2.0 for i in range(-20, 21)]


    # Ключевые значения Vth (для изолиний и границ между цветами)
    @staticmethod
    def LevelsThawedPart():
        result = [i/20.0 for i in range(0, 21)]
        result[0] = 1e-15
        result[-1] = 1.0 - result[0]
        return result


    # Локатор для ключевых значений температуры (соотв. LevelsTemperature())
    @staticmethod
    def LocatorBasicTemperature():
        return MultipleLocator(base=0.5)


    # Локатор для значений на шкале температуры
    @staticmethod
    def LocatorColorbarTemperature():
        return MultipleLocator(base=1.0)


    # Локатор для ключевых значений Vth (соотв LevelsThawedPart())
    @staticmethod
    def LocatorBasicThawedPart():
        return MultipleLocator(base=0.05)


    # Локатор для значений на шкале Vth
    @staticmethod
    def LocatorColorbarThawedPart():
        return MultipleLocator(base=0.1)


    # Создаёт в fig и возвращает столбик шкалы для построения температуры csetf
    @staticmethod
    def ColorbarTemperature(csetf, fig):
        result = fig.colorbar(csetf,
                              ticks=QFrostPlot.LocatorColorbarTemperature(),
                              use_gridspec=True)
        result.set_label('Температура $T$, °C')
        return result


    # Создаёт в fig и возвращает столбик шкалы для построения V_th csetf
    @staticmethod
    def ColorbarThawedPart(csetf, fig):
        result = fig.colorbar(csetf,
                              ticks=QFrostPlot.LocatorColorbarThawedPart(),
                              format=QFrostPlot.PercentFormatter(),
                              use_gridspec=True)
        result.set_label('Относительный объём талой фазы $V_{th}$')
        return result


    # Базовые цвета изолиний (использовать с ContourBasicLineWidths)
    @staticmethod
    def ContourBasicColors():
        return ['0.25', '0.5', '0.5', '0.5', '0.5']


    # Базовые значения толщины изолиний (использовать с ContourBasicColors)
    @staticmethod
    def ContourBasicLineWidths():
        return [1.0, 0.5, 0.5, 0.5, 0.5]


    # Настраивает ось помесячную ось времени. Возвращает созданную подпись оси.
    @staticmethod
    def SetupAxisMonth(axis):
        axis.set_major_locator(MonthLocator())
        axis.set_major_formatter(DateFormatter('%m'))
        return axis.set_label_text('Месяц')


    # Настраивает ось глубины. Возвращает созданную подпись оси.
    @staticmethod
    def SetupAxisDepth(axis):
        axis.set_minor_locator(MultipleLocator(base=0.2))
        axis.set_major_locator(MultipleLocator())
        if axis.axis_name is 'x':
            axis.axes.invert_xaxis()
        else:
            axis.axes.invert_yaxis()
        return axis.set_label_text('Глубина, м')


    # Настраивает ось температуры. Возвращает созданную подпись оси.
    @staticmethod
    def SetupAxisTemperature(axis):
        return axis.set_label_text('Температура, °C')


    # Настраивает ось отн. объёма талой фазы. Возвращает созданную подпись оси.
    @staticmethod
    def SetupAxisThawedPart(axis):
        AXIS_COLOR = 'blue'
        axis.set_major_formatter(QFrostPlot.PercentFormatter())
        if axis.axis_name is 'x':
            axis.axes.set_xlim(0.0, 1.0)
        else:
            axis.axes.set_ylim(0.0, 1.0)
        [ticklabel.set_color(AXIS_COLOR) for ticklabel in axis.get_ticklabels()]
        return axis.set_label_text('Относительный объём талой фазы $V_{th}$',
                                   color=AXIS_COLOR)


    # Включает у axis сетку (использовать с обычными графиками)
    @staticmethod
    def SetupGridFor1D(axes):
        axes.set_axisbelow(True) # сетка позади графиков
        axes.grid(True, ls='-', c='#a0a0a0')


    # Включает у axis сетку (использовать с изолиниями и цветокартами)
    @staticmethod
    def SetupGridFor2D(axes):
        axes.grid(True, ls='-', c='#e0e0e0')


    # Создаёт подписи для изолиний температуры cset
    @staticmethod
    def LabelContourTemperatures(cset):
        cset.clabel(fontsize=6, fmt='%1.1f')


    # Создаёт подписи для изолиний отн. объёма талой фазы cset
    @staticmethod
    def LabelContourThawedPart(cset):
        cset.clabel(fontsize=6, fmt=QFrostPlot.PercentFormatter())


    # ColormapThawedPart() или ColormapTemperature() (по is_thawed_parts_plot)
    @staticmethod
    def Colormap(is_thawed_parts_plot):
        if is_thawed_parts_plot:
            return QFrostPlot.ColormapThawedPart();
        else:
            return QFrostPlot.ColormapTemperature();


    # LevelsThawedPart() или LevelsTemperature() (по is_thawed_parts_plot)
    @staticmethod
    def Levels(is_thawed_parts_plot):
        if is_thawed_parts_plot:
            return QFrostPlot.LevelsThawedPart();
        else:
            return QFrostPlot.LevelsTemperature();


    # ColorbarThawedPart() или ColorbarTemperature() (по is_thawed_parts_plot)
    @staticmethod
    def Colorbar(csetf, fig, is_thawed_parts_plot):
        if is_thawed_parts_plot:
            return QFrostPlot.ColorbarThawedPart(csetf, fig);
        else:
            return QFrostPlot.ColorbarTemperature(csetf, fig);
