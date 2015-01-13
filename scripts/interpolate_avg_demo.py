#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import copy
import numpy as np
import datetime as dt
from warnings import warn
from calendar import isleap
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib.ticker as mticker
from interpolate_avg import spline_avg_coeffs, interp_avg, InterpMonthly

def _polynom_str(x0, coeffs):
    """Понятное человеку текстовое представление многочлена N степени.

    Возвращается строка вида a+b*(x-x0)+c*(x-x0)^2, в которой параметры a, b и c
    заменены на соответственные элементы `coeffs`, а x0 заменены значением `x0`.
    """
    polynom_power = len(coeffs) - 1
    if polynom_power < 1:
        raise ValueError('polynom power is too small (min 1)' % polynom_power)
    result = '{:f}'.format(coeffs[0])
    x_str = ''
    if isinstance(x0, dt.date):
        x_str = 'secs({}, x)'.format(x0.isoformat()).replace('-','_').replace('T00:00:00', '')
    else:
        x_str = 'x' if x0 == 0.0 else '(x{:+})'.format(-x0)
    polynom_token = '{{{}:+}}*' + x_str
    for i in range(1, polynom_power+1):
        result += ('{:+f}*'.format(coeffs[i]) + x_str).replace('-', ' - ').replace('+', ' + ')
        if i > 1:
            result += '^{}'.format(i)
    return result


def _next_range(arr):
    """Возвращает массив, полученный прибавлением к `arr` значения `arr[-1]`."""
    result = copy.copy(arr)
    delta = arr[-1] - arr[0]
    if isinstance(arr[0], dt.date):
        year_delta = dt.timedelta(days=366 if isleap(arr[0].year) else 365)
        if abs((delta - year_delta).seconds) < 5:
            delta = year_delta
    return [d + delta for d in arr]


def _step_func(x, y):
    """Возвращает пару массивов, задающих ступенчатую функцию для её построения.

    Параметры:
    x: N+1 значений аргумента, задающих N интервалов. Должен быть возрастающим!
    y: N значений функции для каждого из выделенных интервалов.
    """
    sx = []
    sy = []

    for i in range(0, len(y)):
        sx.append(x[i])
        sy.append(y[i])

        sx.append(x[i+1])
        sy.append(y[i])

    sx.append(x[-1])
    sy.append(y[0])

    return sx, sy


def _roman(input):
    """Convert an integer to Roman numerals."""
    if not isinstance(input, int):
        raise TypeError("expected integer, got %s" % type(input))
    if not 0 < input < 4000:
        raise ValueError("argument must be between 1 and 3999")
    ints = (1000, 900,  500, 400, 100,  90, 50,  40, 10,  9,   5,  4,   1)
    nums = ('M',  'CM', 'D', 'CD','C', 'XC','L','XL','X','IX','V','IV','I')
    result = ""
    for i in range(len(ints)):
        count = int(input / ints[i])
        result += nums[i] * count
        input -= ints[i] * count
    return result


def _setup_monthly_axis(axis):
    """Настройка засечек и подписей `axis` для выделения ежемесячных интервалов.

    Результат: помесячное разбиение оси с указанием номера каждого месяца в виде
    римских цифр, расположенных по центру каждого интервала. Эту функцию следует
    использовать после проведения построений или задания масштаба, поскольку она
    должна убрать серединные засечки, появляющиеся после центрирования подписей.
    """
    axis.set_major_locator(mdates.MonthLocator())
    axis.set_minor_locator(mdates.MonthLocator(bymonthday=15))

    def month_roman(x, pos):
        return _roman(mdates.num2date(x).month)

    axis.set_major_formatter(mticker.NullFormatter())
    axis.set_minor_formatter(mticker.FuncFormatter(month_roman))

    minor_ticks = axis.get_minor_ticks()
    if not minor_ticks:
        warn('no minor ticks: check that plotting/scaling is done before call')
    else:
        for tic in minor_ticks:
            tic.tick1On = tic.tick2On = False


def main():
    y = np.array([-20.7, -21.7, -19.5,
                  -12.9, -5.2, 1.9,
                  7.4, 6.9, 3.4,
                  -3.9, -12.5, -18.1])

    use_monthly_class = True

    ax = []
    ay = []
    sx = []
    sy = []
    ax2 = []
    ay2 = []
    sx2 = []
    sy2 = []

    if use_monthly_class:
        interp_monthly = InterpMonthly(y)

        ax, ay = interp_monthly.interp(24, True)
        sx, sy = interp_monthly.step_func(True)

        ax2, ay2 = interp_monthly.interp(550, False)
        sx2, sy2 = interp_monthly.step_func(False)
    else:
        x = 30 * np.arange(0, len(y)+1, dtype=int)
        #x = np.array([dt.date(2014, month, 1) for month in range(1, 13)] + [dt.date(2015, 1, 1)])

        ax, ay = interp_avg(x, y, 1000)
        sx, sy = _step_func(x, y)

        ax2, ay2 = _next_range(ax), ay
        sx2, sy2 = _next_range(sx), sy

    ax3, ay3 = _next_range(ax2), ay2
    sx3, sy3 = _next_range(sx2), sy2

    fig, axes = plt.subplots()

    extra_color = '0.3'
    extra_ls = ':'
    extra_steps_ls = '--'

    axes.plot(ax, ay, color=extra_color, ls=extra_ls)
    axes.plot(sx, sy, antialiased=False, color=extra_color, ls=extra_steps_ls)

    axes.plot(ax2, ay2)
    axes.plot(sx2, sy2, antialiased=False)

    axes.plot(ax3, ay3, color=extra_color, ls=extra_ls)
    axes.plot(sx3, sy3, antialiased=False, color=extra_color, ls=extra_steps_ls)

    if isinstance(ax[0], dt.date):
        _setup_monthly_axis(axes.xaxis)

    axes.grid()

    plt.show()


if __name__ == '__main__':
    main()
