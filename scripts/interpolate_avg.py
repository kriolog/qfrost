#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
import datetime as dt
from calendar import isleap

def spline_avg_coeffs(x, y):
    """Коэффициенты сплайна периодической функции, заданной средними значениями.

    Пример применения: получение значений температуры по дням из среднемесячных.

    Параметры:
    x: N+1 значений аргумента, задающих N интервалов. Должен быть возрастающим!
    y: N средних значений функции для каждого из выделенных интервалов.

    То есть из N средних значений функции {y_i} на интервалах {[x_i .. x_{i+1}]}
    получаем коэффициенты квадратичного сплайна f_i - N наборов {a_i, b_i, c_i},
    задащих его внутри каждого интервала как f_i(x) = a_i + b_i * x + c_i * x^2.
    Для этого решается система уравнений, составленная из накладываемых условий.

    Основные условия (корректность и соответствие средних):
    1) Непрерывность - f_i(x_i) = f_{i+1}(x_i).
    2) Гладкость первого порядка - f'_i(x_i) = f'_{i+1}(x_i).
    3) Известные средние - ∫ [x_{i-1} .. x_i] f_i(x)dx = y_i * (x_1 - x_{i-1}).

    Дополнительные условия (делают сплайн периодическим):
    1) Непрерывность на стыке периодов - f_0(x_0) = f_N(x_{N+1}).
    2) Гладкость первого порядка на стыке периодов - f'_0(x_0) = f'_N(x_{N+1}).

    Возвращается массив коэффициентов искомого сплайна: N троек {a_i, b_i, c_i}.
    """
    n = len(y)

    if x.ndim != 1 or y.ndim != 1:
        raise ValueError("x and y should both be 1-D arrays")

    if len(x) != n + 1:
        raise ValueError("x should have 1 more element than y")

    a = np.zeros((3*n, 3*n))
    b = np.zeros(3*n)

    for i in range(0, n):
        h = x[i+1] - x[i]

        if isinstance(h, dt.timedelta):
            h = h.total_seconds()

        if h <= 0:
            raise ValueError("x should be sorted in ascending order")

        j = 3*i

        a[j, j] = 1
        a[j, j+1] = h
        a[j, j+2] = h*h

        #a[j+1, j] = 0 (уже)
        a[j+1, j+1] = 1
        a[j+1, j+2] = 2*h

        a[j+2, j] = 1
        a[j+2, j+1] = h/2
        a[j+2, j+2] = h*h/3

    for i in range(0, n-1):
        j = 3*i
        a[j, j+3] = -1
        a[j+1, j+4] = -1

    a[3*n-3, 0] = -1
    a[3*n-2, 1] = -1

    for i in range(0, n):
        j = 3*i
        b[j+2] = y[i]

    result_flat = np.linalg.solve(a, b)
    return np.reshape(result_flat, (n,3))


def spline_avg_piecefunctions(x, y):
    """Сегменты сплайна периодической функции, заданной средними значениями.

    Параметры:
    x: N+1 значений аргумента, задающих N интервалов. Должен быть возрастающим!
    y: N средних значений функции для каждого из выделенных интервалов.

    Сплайн составлен из N сегментов для каждого интервала из {[x_i .. x_{i+1}]}.

    Возвращается два массива функций по N элементов, по паре на каждый интервал:
    1) Квадратичные функции, опредяющие сегменты сплайна на заданных интервалах.
    2) Функции принадлежности к этим интервалам (для получения булевых массивов,
       используемых в качестве аргумента `condlist` для функции `np.piecewise`).
    """
    spline_coeffs = spline_avg_coeffs(x, y)

    funclist = []
    condfuncs = []

    for i in range(0, len(y)):
        condfuncs.append(_condfuncgen(x[i], x[i+1]))
        funclist.append(_piecefuncgen(spline_coeffs[i], x[i]))

    return funclist, condfuncs


def interp_avg(x, y, num): 
    """Координаты сплайна периодической функции, заданной средними значениями.

    Параметры:
    x: N+1 значений аргумента, задающих N интервалов. Должен быть возрастающим!
    y: N средних значений функции для каждого из выделенных интервалов.
    num: требуемое количество точек интерполяции.

    Возвращаются X и Y координаты сплайна - 2 массива по `num` элементов каждый,
    причём элементы первого (значения X) равномерно возрастают от x[0] до x[N].
    """
    if not isinstance(num, int) or num <= 0:
        raise ValueError("num must be positive integer")

    step = (x[-1]-x[0])/num
    ax = [x[0]] + [x[0] + step * i for i in range(1, num-1)] + [x[-1]]

    funclist, condfuncs = spline_avg_piecefunctions(x, y)
    condlist = [func(ax) for func in condfuncs]

    func = funclist[0]
    return ax, np.piecewise(ax, condlist, funclist)


"""Интерполяционный сплайн периодической функции, определяемой среднемесячными.

Вычисляет коэффициенты для 2 сплайнов (для високосных и невисокосных лет), после
чего возможна интерполяция по любой точке на временной оси, ведь сплайн является
периодическим (значение и 1 переменная в его первой и последней точке совпадают)
и, поскольку он задан по среднемесячным значениям, не изменяется от года к году.
"""
class InterpMonthly:
    __year_comm = 2013
    __year_leap = 2012

    __x_comm = []
    __x_leap = []

    __coeffs_comm = []
    __coeffs_leap = []

    __piece_funcs_comm = []
    __piece_funcs_leap = []

    __y = []

    def __init__(self, y):
        """При инициализации вычисляет пару сплайнов: високосный и невисокосный.
           Для этого используется `y` - массив (из 12) среднемесячных значений.
        """
        if len(y) != 12:
            raise ValueError('monthly averages array should contain 12 elements, not %d' % len(y))

        assert(isleap(self.__year_leap))
        assert(not isleap(self.__year_comm))

        self.__y = y

        self.__x_comm = self.__x_arr(False)
        self.__x_leap = self.__x_arr(True)

        self.__coeffs_comm = spline_avg_coeffs(self.__x_comm, y)
        self.__coeffs_leap = spline_avg_coeffs(self.__x_leap, y)

        for i in range(0, len(y)):
            self.__piece_funcs_comm.append(_piecefuncgen(self.__coeffs_comm[i],
                                                         self.__x_comm[i]))
            self.__piece_funcs_leap.append(_piecefuncgen(self.__coeffs_leap[i],
                                                         self.__x_leap[i]))


    def val(self, x):
        """Возвращает значение сплайна для даты (или даты-времени) `x`."""
        if not isinstance(x, dt.date):
            raise TypeError("expected date instance, got %s" % type(input))

        is_leap = isleap(x.year)

        year = self.__year_leap if is_leap else self.__year_comm

        funcs = self.__piece_funcs_leap if is_leap else self.__piece_funcs_comm
        func = funcs[x.month - 1]

        return func(x.replace(year=year))


    def interp(self, steps_per_day, is_leap):
        if not isinstance(steps_per_day, int) or steps_per_day <= 0:
            raise ValueError("steps_per_day must be positive integer")

        days_num = 366 if is_leap else 365

        num = steps_per_day * days_num
        x = self.__x_leap if is_leap else self.__x_comm

        step = (x[-1]-x[0])/num

        # FIXME лучше разбивать посуточно по steps_per_day (погрешность меньше)
        x0 = dt.datetime.combine(x[0], dt.time())
        xN = dt.datetime.combine(x[-1], dt.time())

        ax = [x0] + [x0 + step * i for i in range(1, num)]
        ay = []

        year = x[0].year

        funcs = self.__piece_funcs_leap if is_leap else self.__piece_funcs_comm
        func_it = iter(funcs)
        func = next(func_it)

        cur_month = 1
        for x in ax:
            if (x.month != cur_month):
                if (x.month > cur_month):
                    func = next(func_it)
                else:
                    # сменился год (видимо, дошли до xN) - перезапустим итерацию
                    func_it = iter(funcs)
                    func = next(func_it)
                cur_month = x.month
            ay.append(func(x.replace(year=year)))

        return ax, ay


    def step_func(self, is_leap):
        x = self.__x_leap if is_leap else self.__x_comm

        sx = []
        sy = []

        for i in range(0, len(self.__y)):
            sx.append(x[i])
            sy.append(self.__y[i])

            sx.append(x[i+1])
            sy.append(self.__y[i])

        sx.append(x[-1])
        sy.append(self.__y[0])

        return sx, sy


    def __x_arr(self, is_leap):
        """13 дат, разбивающих год на 12 участков, соответствующих месяцам."""
        year = 2012 if is_leap else 2013
        return np.array([dt.date(year, month, 1) for month in range(1, 13)]
                        + [dt.date(year + 1, 1, 1)])


"""Генератор булевых функций принадлежности к интервалу [`x_min` ; `x_max`]."""
def _condfuncgen(x_min, x_max):
    def thefunc(x):
        if hasattr(x, '__iter__'):
            return [thefunc(v) for v in x]
        else:
            return (x >= x_min) & (x <= x_max)

    return thefunc


"""Генератор функции для N-мерного многочлена (типа a + b*(x-x0) + c*(x- x0)^2),
коэффициенты которого задаются массивом `coeffs`, а вместо x0 используется `x0`.
"""
def _piecefuncgen(coeffs, x0):
    def thefunc(x):
        if hasattr(x, '__iter__'):
            return [thefunc(v) for v in x]
        else:
            result = coeffs[0]
            x0_fixed = x0
            if isinstance(x, dt.datetime) and not isinstance(x0, dt.datetime):
                x0_fixed = dt.datetime.combine(x0, dt.time())
            h = x - x0_fixed
            if isinstance(h, dt.timedelta):
                h = h.total_seconds()
            for i in range(1, len(coeffs)):
                result += coeffs[i] * pow(h, i)
            return result

    return thefunc
