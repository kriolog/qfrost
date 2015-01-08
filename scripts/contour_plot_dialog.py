#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import numpy

from PyQt5.QtCore import pyqtSlot

from PyQt5 import QtCore
from PyQt5 import QtWidgets
from PyQt5 import QtGui

import matplotlib
matplotlib.use('qt5agg')

from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure

from contour_plot import ContourPlot

def _readpart(f, dtype=numpy.float, sep='\t'):
    """Многомерный массив (numpy.array), полученный из части текстового файла.

    Считывает содержимое файла f до пустой или начинающейся с буквы строки.
    После этого полученные строки разбиваются на поля по разделителю sep, каждое
    поле конвертируется в тип dtype и уходит в возвращаемый массив.

    Размерность возвращаемого массива равна числу полей, выделенных в считанных
    строках после разбиения по разделителю. А его размер равен количеству строк.
    """
    lines = []
    while True:
        line = f.readline().rstrip()
        if line == "" or line[0].isalpha():
            return numpy.array([x.split(sep) for x in lines], dtype=dtype)
        lines.append(line)


class AreaPlotCanvas(FigureCanvas):
    def __init__(self, parent=None):
        fig = Figure(dpi=100)

        FigureCanvas.__init__(self, fig)
        self.setParent(parent)

        FigureCanvas.setSizePolicy(self,
                                   QtWidgets.QSizePolicy.Expanding,
                                   QtWidgets.QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)


class AreaPlotDialog(QtWidgets.QMainWindow):
    __plot = None
    __canvas = None
    __filename = ""

    __STATUSBAR_MESSAGE_TIMEOUT = 2000

    def __init__(self):
        QtWidgets.QMainWindow.__init__(self)

        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)

        self.__canvas = AreaPlotCanvas(self)
        self.setCentralWidget(self.__canvas)

        self.__plot = ContourPlot(self.__canvas.figure)
        self.__plot.stateChanged.connect(self.statusBar().showMessage)
        self.__plot.stateCleared.connect(self.statusBar().clearMessage)

        open_action = QtWidgets.QAction(QtGui.QIcon.fromTheme('document-open'),
                                        'Open File...', self)
        open_action.setShortcut(QtGui.QKeySequence.Open)
        open_action.setStatusTip('Open file with 2D plot data from QFrost')
        open_action.triggered.connect(self.open_data)

        save_action = QtWidgets.QAction(QtGui.QIcon.fromTheme('document-save-as'),
                                        'Save Image...', self)
        save_action.setShortcut(QtGui.QKeySequence.Save)
        save_action.setStatusTip('Save plot to image file')
        save_action.triggered.connect(self.save_pdf)

        exit_action = QtWidgets.QAction(QtGui.QIcon.fromTheme('document-close'),
                                        'Close', self)
        exit_action.setShortcut(QtCore.Qt.Key_Exit)
        exit_action.setStatusTip('Close this dialog')
        exit_action.triggered.connect(self.close)

        uses_t_act_text = 'Temperature'
        uses_v_act_text = 'Thawed part'

        # Цветовая карта
        map_separator_act = QtWidgets.QAction('Color Map', self)
        map_separator_act.setSeparator(True)

        map_visibility_act = VisibilityAction('Show Color Map', self)
        map_visibility_act.triggered.connect(self.__plot.set_map_visibility)
        map_visibility_act.setShortcut(QtGui.QKeySequence("Ctrl+M"))

        map_uses_act_group = QtWidgets.QActionGroup(self)
        map_uses_act_group.setExclusive(True)
        map_visibility_act.triggered.connect(map_uses_act_group.setEnabled)

        map_uses_t_act = QtWidgets.QAction(uses_t_act_text, map_uses_act_group)
        map_uses_t_act.setCheckable(True)
        map_uses_t_act.setChecked(True)
        map_uses_t_act.toggled.connect(self.__plot.set_map_uses_t)

        map_uses_v_act = QtWidgets.QAction(uses_v_act_text, map_uses_act_group)
        map_uses_v_act.setCheckable(True)

        # Изолинии
        iso_separator_act = QtWidgets.QAction('Contours', self)
        iso_separator_act.setSeparator(True)

        iso_visibility_act = VisibilityAction('Show Contours', self)
        iso_visibility_act.triggered.connect(self.__plot.set_iso_visibility)
        iso_visibility_act.setShortcut(QtGui.QKeySequence("Ctrl+I"))

        iso_uses_act_group = QtWidgets.QActionGroup(self)
        iso_uses_act_group.setExclusive(True)
        iso_visibility_act.triggered.connect(iso_uses_act_group.setEnabled)

        iso_uses_t_act = QtWidgets.QAction(uses_t_act_text, iso_uses_act_group)
        iso_uses_t_act.setCheckable(True)
        iso_uses_t_act.setChecked(True)
        iso_uses_t_act.toggled.connect(self.__plot.set_iso_uses_t)

        iso_uses_v_act = QtWidgets.QAction(uses_v_act_text, iso_uses_act_group)
        iso_uses_v_act.setCheckable(True)

        iso_replot_act = QtWidgets.QAction('Replot Contours', self)
        iso_replot_act.setStatusTip('Replot contours - use to polish labels after resizing image')
        iso_replot_act.triggered.connect(self.__plot.replot_iso)

        # Дополнительнып элементы построения
        act_visibility_act = VisibilityAction('Mark Phase Transitions', self)
        act_visibility_act.setChecked(False)
        act_visibility_act.triggered.connect(self.__plot.set_act_visibility)

        menubar = self.menuBar()
        file = menubar.addMenu('&File')
        file.addAction(open_action)
        file.addAction(save_action)
        file.addSeparator()
        file.addAction(exit_action)

        view = menubar.addMenu('&View')
        view.addAction(map_separator_act)
        view.addAction(map_visibility_act)
        view.addActions(map_uses_act_group.actions())
        view.addAction(iso_separator_act)
        view.addAction(iso_visibility_act)
        view.addActions(iso_uses_act_group.actions())
        view.addAction(iso_replot_act)
        view.addSeparator()
        view.addAction(act_visibility_act)

        self.__updateTitle()

        self.statusBar().showMessage('Welcome to QFrost 2D Plot!',
                                     self.__STATUSBAR_MESSAGE_TIMEOUT)


    def __updateTitle(self):
        filename = self.__filename if self.__filename else '[no data]'
        self.setWindowTitle(filename + ' — QFrost 2D Plot')


    def __showWarning(self, title, main_text, status_text, exception = None):
        self.statusBar().showMessage(status_text,
                                     self.__STATUSBAR_MESSAGE_TIMEOUT)
        messagebox_text = main_text
        if exception and hasattr(exception, 'strerror'):
            messagebox_text += '\n\n' + exception.strerror + '.'
        QtWidgets.QMessageBox.warning(self, title, messagebox_text)


    def open_data(self):
        filename, _ = QtWidgets.QFileDialog.getOpenFileName(self,
                                                            'Open QFrost 2D Data',
                                                             self.__filename,
                                                            'Text Files (*.txt)')

        if not filename:
            return False

        self.__filename = filename
        self.__plot.clear()
        self.__updateTitle()

        with open(filename, 'r') as f:
            try:
                filename_quote = self.locale().quoteString(filename)
                self.statusBar().showMessage('Reading {0}...'.format(filename_quote))
                QtWidgets.qApp.setOverrideCursor(QtCore.Qt.WaitCursor)

                f.readline()
                f.readline()
                x, y, t, v = _readpart(f).T
                outer_polygons_points = _readpart(f)
                outer_polygons_indexes = _readpart(f, numpy.int)
                inner_polygons_points = _readpart(f)
                inner_polygons_indexes = _readpart(f, numpy.int)
                bounds_x, bounds_y, bounds_t, bounds_v = _readpart(f).T
                f.close()
            except Exception as e:
                QtWidgets.qApp.restoreOverrideCursor()
                self.__showWarning('Open Failed',
                                   'Cannot open {0}.'.format(filename_quote),
                                   'Open failed!', e)
                return False
            else:
                try:
                    self.__plot.set_mesh(x, y,
                                         outer_polygons_points,
                                         outer_polygons_indexes,
                                         inner_polygons_points,
                                         inner_polygons_indexes,
                                         bounds_x, bounds_y)
                    self.__plot.plot_data(t, v, bounds_t, bounds_v)
                except Exception as e:
                    QtWidgets.qApp.restoreOverrideCursor()
                    self.__showWarning('Open Failed',
                                       'Cannot parse {0}.'.format(filename_quote),
                                       'Open failed!', e)
                    return False
                else:
                    QtWidgets.qApp.restoreOverrideCursor()
                    self.statusBar().showMessage('File loaded and plotted',
                                                 self.__STATUSBAR_MESSAGE_TIMEOUT)
                    return True


    def save_pdf(self):
        filename, _ = QtWidgets.QFileDialog.getSaveFileName(self,
                                                            'Save PDF',
                                                            self.__filename + '.pdf',
                                                            'PDF File (*.pdf)')

        if not filename:
            return False

        filename_quote = self.locale().quoteString(filename)

        QtWidgets.qApp.setOverrideCursor(QtCore.Qt.WaitCursor)
        self.statusBar().showMessage('Saving {0}...'.format(filename_quote))

        try:
            self.__canvas.figure.savefig(filename, bbox_inches='tight')
        except Exception as e:
            QtWidgets.qApp.restoreOverrideCursor()
            self.__showWarning('Save Failed',
                               'Cannot save {0}.'.format(filename_quote),
                               'Save failed!', e)
            return False
        else:
            QtWidgets.qApp.restoreOverrideCursor()
            self.statusBar().showMessage('File saved',
                                         self.__STATUSBAR_MESSAGE_TIMEOUT)
            return True


class VisibilityAction(QtWidgets.QAction):
    def __init__(self, text, parent=None, checked=True):
        QtWidgets.QAction.__init__(self, text, parent)
        self.setCheckable(True)
        self.setChecked(checked)
        self.triggered.connect(self.__update_icon)
        self.__update_icon(self.isChecked())


    @pyqtSlot(bool)
    def __update_icon(self, visible):
        icon_name = "layer-visible-on" if visible else "layer-visible-off"
        self.setIcon(QtGui.QIcon.fromTheme(icon_name))


def main():
    app = QtWidgets.QApplication(sys.argv)

    ex = AreaPlotDialog()
    ex.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
