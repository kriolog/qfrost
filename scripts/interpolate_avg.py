#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np

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

    def condfuncgen(x_min, x_max):
        def thefunc(x):
            return (x >= x_min) & (x <= x_max)
            #return lambda x: np.logical_and(np.core.umath.greater_equal(x, x_min),
            #                                np.core.umath.less_equal(x, x_max))
        return thefunc

    def piecefuncgen(coeffs, x0):
        def thefunc(x):
            result = coeffs[0]
            for i in range(1, len(coeffs)):
                result += coeffs[i] * pow(x - x0, i)
            return result
        return thefunc

    funclist = []
    condfuncs = []

    for i in range(0, len(y)):
        condfuncs.append(condfuncgen(x[i], x[i+1]))
        funclist.append(piecefuncgen(spline_coeffs[i], x[i]))

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

    step = (x[-1]-x[0])/(num-1)

    ax = [x[0] + step * i for i in range(0, num)]

    funclist, condfuncs = spline_avg_piecefunctions(x, y)
    condlist = [func(ax) for func in condfuncs]

    func = funclist[0]
    return ax, np.piecewise(ax, condlist, funclist)
