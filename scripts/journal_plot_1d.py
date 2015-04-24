from sys import stderr
from datetime import date
from os.path import splitext

from pylab import datestr2num
from numpy import reshape, asarray, ndim

from matplotlib import pyplot as plt
from matplotlib.dates import DateFormatter
from matplotlib.animation import FuncAnimation

from progressbar import ProgressBar, ETA, Percentage, Bar

from qfrost_plot_basics import QFrostPlot, QFrostVType


class JournalPlot1D():
    """Построитель для журналов 1D задач (изоплеты или анимированная кривая)."""

    __cmap_t = QFrostPlot.ColormapTemperature()
    __cmap_v = QFrostPlot.ColormapThawedPart()
    __cmap_v2 = QFrostPlot.ColormapFront()

    __levels_t = QFrostPlot.LevelsTemperature()
    __levels_v = QFrostPlot.LevelsThawedPart()

    __dates = []                    # N значений даты
    __depths = []                   # M значений глубины
    __transition_temperatures = []  # M значений T_bf

    __temperatures_by_day = [[]]    # N*M значений T (2D массив по датам)
    __thawed_parts_by_day = [[]]    # N*M значений V_th (2D массив по датам)

    __temperatures_by_z = [[]]      # M*N значений T (2D массив по глубинам)
    __thawed_parts_by_z = [[]]      # M*N значений V_th (2D массив по глубинам)

    __temperature_min = -10.0;      # Минимальное значение __temperatures
    __temperature_max = 5.0;        # Максимальное значение __temperatures

    __depth_min = 0.0;              # Минимальное значение __depths
    __depth_max = 8.0;              # Максимальное значение __depths


    def __init__(self, dates, depths,
                 transition_temperatures, temperatures, thawed_parts,
                 max_depth = float('inf')):
        """Стандартный конструктор - по комплекту одномерных (!) списков.

        dates: N возрастающих значений даты, для которых сохранены значения.
        depths: M возрастающих значений глубины, для которых сохранены значения.
        transition_temperatures: M значений температуры фазового перехода.
        temperatures: N*M значений температуры - M по дню 1, M по дню 2, ...
        thawed_parts: N*M значений отн. объёма талой фазы.
        max_depth: максимальное значение глубины (по умолчанию ограничения нет).
        """
        for arg_val, arg_name in ((dates, 'dates'),
                                  (depths, 'depths'),
                                  (transition_temperatures, 'transition_temperatures'),
                                  (temperatures, 'temperatures'),
                                  (thawed_parts, 'thawed_parts')):
            arg_type_name = type(arg_val).__name__;
            if not hasattr(arg_val, "__len__") or arg_type_name is 'string':
                raise ValueError('Parameters must be arrays, but {0} is {1}.'
                                 .format(arg_name, arg_type_name))
            if ndim(arg_val) != 1:
                raise ValueError('Parameters must be 1-D, but {0} is {1}-D.'
                                 .format(arg_name, ndim(arg_val)))

        num_dates = len(dates)
        num_depths = len(depths)
        num_values = num_dates * num_depths

        if len(transition_temperatures) != num_depths:
            raise ValueError('Expected {0} transition temperatures, but got {1}.'
                             .format(num_depths, len(transition_temperatures)))

        if len(temperatures) != num_values:
            raise ValueError('Expected {0} temperatures, but got {1}.'
                             .format(num_values, len(temperatures)))

        if len(thawed_parts) != num_values:
            raise ValueError('Expected {0} thawed parts, but got {1}.'
                             .format(num_values, len(thawed_parts)))

        self.__dates = asarray(dates, dtype=date)
        self.__depths = asarray(depths, dtype=float)

        self.__transition_temperatures = asarray(transition_temperatures, dtype=float)

        main_shape = (num_dates, num_depths)
        self.__temperatures_by_day = reshape(asarray(temperatures, dtype=float), main_shape)
        self.__thawed_parts_by_day = reshape(asarray(thawed_parts, dtype=float), main_shape)

        self.__temperatures_by_z = self.__temperatures_by_day.swapaxes(0, 1)
        self.__thawed_parts_by_z = self.__thawed_parts_by_day.swapaxes(0, 1)

        self.__temperature_min = min(temperatures)
        self.__temperature_max = max(temperatures)

        self.__depth_min = min(depths)
        self.__depth_max = min(max(depths), max_depth)


    @classmethod
    def from_file(cls, f, max_depth = float('inf')):
        """Считывает журнал. Если это удалось, возвращает JournalPlot1D по нему.
        Но если его загрузить не вышло (неверный формат ввода), возвращает None.

        f: построчно итерируемый объект (открытый файл, массив строк и пр.).
        max_depth: макс. значение глубины (по умолчанию ограничения нет).
        """

        global _line_number
        _line_number = 0

        def readpart(f, arg_converter=float):
            """
            1D массив значений, полученных чтением строк из f вплоть до пустой.
            None, если попалась неконвертируемая строка (ошибка arg_converter).

            f: построчно итерируемый объект (открытый файл, массив строк и пр.).
            arg_converter: конвертер строк в эл-ты массива (по умолчанию float).
            """
            result = []
            try:
                for line in f:
                    global _line_number
                    _line_number += 1
                    token = line.strip()
                    if not token:
                        break # пустая строка
                    val = None
                    try:
                        val = arg_converter(token)
                    except ValueError:
                        _errprint('LOAD FAILED. Cannot get value from line {0}:\n'
                                  ' >>> {1}'.format(_line_number, line.strip('\n')))
                        return None
                    else:
                        result.append(val)
                return result
            except ValueError:
                _errprint('LOAD FAILED. Cannot iterate through lines (binary input?).')
                return None

        dates = readpart(f, datestr2num)
        if dates is None: return None

        depths = readpart(f)
        if depths is None: return None

        t = readpart(f)
        if t is None: return None

        v = readpart(f)
        if v is None: return None

        tbf = readpart(f)
        if tbf is None: return None

        _subprint('Got {0} dates & {1} depths (=> {2} vals)'
                   .format(len(dates), len(depths), len(dates)*len(depths)))

        try:
            # Если журнал некорректен, конструктор выкинет ValueError
            return cls(dates, depths, tbf, t, v)
        except ValueError as e:
            _errprint('LOAD FAILED. ' + str(e))
            return None


    @staticmethod
    def __file_extension(filename):
        """Расширение файла с именем filename (без точки; в нижнем регистре)."""
        result = splitext(filename)[1].lower()
        return result[1:] if result.startswith('.') else result


    def __values_by_z(self, vtype):
        """Массив значений, соответствующий vtype."""
        if vtype is QFrostVType.thawed_part:
            return self.__thawed_parts_by_z
        elif vtype is QFrostVType.temperature:
            return self.__temperatures_by_z
        else:
            raise ValueError("Argument 'vtype' can not be 'none'.")


    def savePlot2D(self, filename, scale_factor,
                   map_type, iso_type):
        """Сохранение выбранных изоплет и/или цветокарты в заданный файл.

        filename: имя файла (должно иметь подходящее расширение).
        scale_factor: масштабный коэффициент - для выбора dpi при сохранении.
        map_type: величина, строящаяся с помощью цветокарты (QFrostVType).
        iso_type: величина, строящаяся с помощью изолиний (QFrostVType).
        """
        if not isinstance(map_type, QFrostVType):
            raise ValueError("Argument 'map_type' must be QFrostVType (not %s)."
                             % type(map_type).__name__)

        if not isinstance(iso_type, QFrostVType):
            raise ValueError("Argument 'iso_type' must be QFrostVType (not %s)."
                             % type(iso_type).__name__)

        QFrostPlot.SetupGridFor2D(plt.axes())

        QFrostPlot.SetupAxisMonth(plt.axes().xaxis)
        QFrostPlot.SetupAxisDepth(plt.axes().yaxis)

        need_map = map_type is not QFrostVType.none
        need_iso = iso_type is not QFrostVType.none

        fig = plt.gcf()

        if need_map:
            _subsubprint('%s color map' % map_type.name)

            contourf_cmap = QFrostPlot.Colormap(map_type)
            contourf_levels = QFrostPlot.Levels(map_type)
            contourf_cset = plt.contourf(self.__dates,
                                         self.__depths,
                                         self.__values_by_z(map_type),
                                         contourf_levels,
                                         cmap=contourf_cmap,
                                         extend='both')
            if iso_type != map_type:
                colorbar = QFrostPlot.Colorbar(map_type, contourf_cset, fig)


        if need_iso:
            _subsubprint('%s isopleths' % iso_type.name)
            contour_locator = QFrostPlot.LocatorBasicTemperature()
            contour_cset = plt.contour(self.__dates,
                                       self.__depths,
                                       self.__values_by_z(iso_type),
                                       locator=contour_locator,
                                       colors=QFrostPlot.ContourBasicColors(),
                                       linewidths=QFrostPlot.ContourBasicLineWidths())
            QFrostPlot.LabelContours(iso_type, contour_cset)


        plt.axis('tight')
        plt.ylim(ymin=self.__depth_max, ymax=self.__depth_min)

        fig.set_size_inches(8, 6)
        #plt.tight_layout()

        _subsubprint("saving '%s'..." % filename)
        plt.savefig(filename, dpi=130.28*scale_factor, bbox_inches='tight')

        plt.close()


    def saveAnimation(self, filename):
        """Сохранение анимированных графиков (всех - T, Tbf и Vth) в видео-файл.

        filename: имя сохраняемого файла (расширение должно быть '.mp4'!)
        """
        file_ext = self.__file_extension(filename)
        if file_ext != 'mp4':
            raise ValueError('Bad extension (%s) for out video.' % file_ext)

        axes_t = plt.axes()
        axes_v = plt.axes().twiny()

        QFrostPlot.SetupGridFor1D(axes_t)

        QFrostPlot.SetupAxisTemperature(axes_t.xaxis)
        QFrostPlot.SetupAxisThawedPart(axes_v.xaxis)
        QFrostPlot.SetupAxisDepth(axes_t.yaxis)

        axes_t.set_xlim(self.__temperature_min, self.__temperature_max)

        p_tbf, = axes_t.plot(self.__transition_temperatures, self.__depths, 'k--', linewidth=1)
        p_v, = axes_v.plot([], [], 'b', linewidth=2)
        p_t, = axes_t.plot([], [], 'k', linewidth=3)

        plt.legend((p_t, p_tbf, p_v),
                   ('$T$', '$T_{bf}$', '$V_{th}$'),
                   loc='lower left')

        fig = plt.gcf()
        fig.set_size_inches(4.8, 7.2)
        plt.ylim(self.__depth_max, self.__depth_min)

        frame_title = plt.figtext(0.08, 0.96, '',
                                  size=20, horizontalalignment='center')

        plt.tight_layout()

        num_frames = len(self.__dates)
        pbar_widgets = [' - saving %s: ' % filename,
                        Percentage(), ' ',
                        Bar(), ' ',
                        ETA()]
        progress_bar = ProgressBar(num_frames, widgets=pbar_widgets)

        def init():
            progress_bar.start()
            p_t.set_ydata(self.__depths)
            p_v.set_ydata(self.__depths)
            return p_t, p_v

        title_formatter = DateFormatter("%d.%m")
        def animate(i):
            p_t.set_xdata(self.__temperatures_by_day[i])
            p_v.set_xdata(self.__thawed_parts_by_day[i])
            frame_title.set_text(title_formatter.format_data(self.__dates[i]))
            progress_bar.update(i + 1)
            return p_t, p_v

        ani = FuncAnimation(fig, animate, frames=num_frames,
                            interval=40,  blit=True, init_func=init)

        ani.save(filename, extra_args=['-vcodec', 'libx264'], bitrate=-1, dpi=125)
        progress_bar.finish()

        plt.close()


def _errprint(text):
    print(' ! ' + text, file=stderr)

def _subprint(text):
    print(' * ' + text)

def _subsubprint(text):
    print('  - ' + text)
