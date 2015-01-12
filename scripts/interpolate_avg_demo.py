#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt
from interpolate_avg import spline_avg_coeffs, interp_avg


def polynom_str(x_first, coeffs):
    polynom_power = len(coeffs) - 1
    if polynom_power < 1:
        raise ValueError('polynom power is too small (min 1)' % polynom_power)
    result = '{:1.5f}'.format(coeffs[0])
    x_str = 'x' if x_first == 0.0 else '(x{:+})'.format(-x_first)
    polynom_token = '{{{}:+}}*' + x_str
    for i in range(1, polynom_power+1):
        result += ('{:+1.5f}*'.format(coeffs[i]) + x_str).replace('-', ' - ').replace('+', ' + ')
        if i > 1:
            result += '^{}'.format(i)
    return result


def main():
    y = np.array([-20.7, -21.7, -19.5, -12.9, -5.2, 1.9,
                  7.4, 6.9, 3.4, -3.9, -12.5, -18.1])
    x = 30 * np.arange(0, len(y)+1, dtype=int)

    result = spline_avg_coeffs(x, y)

    sx = []
    sy = []

    print('x_i\tx_i+1\ty_i\tf_i(x)')
    for i in range(0, len(y)):
        polynom = polynom_str(x[i], result[i])
        print('{}\t{}\t{}\t{}'.format(x[i], x[i+1], y[i], polynom))

        sx.append(x[i])
        sy.append(y[i])

        sx.append(x[i+1])
        sy.append(y[i])

    sx.append(x[-1])
    sy.append(y[0])

    ax, ay = interp_avg(x, y, 1000)

    plt.plot(ax, ay)
    plt.plot(ax + x[-1], ay, color='gray', ls=':')
    plt.plot(sx, sy, antialiased=False)
    plt.plot(sx + x[-1], sy, antialiased=False, color='gray', ls='--')
    plt.show()

if __name__ == '__main__':
    main()
