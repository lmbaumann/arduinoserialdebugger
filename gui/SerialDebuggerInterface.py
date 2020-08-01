# SerialDebuggerInterface.py - Small GUI to view logged variables and skip over breakpoints.
# Created by Luca Baumann, 2020

import sys
import re
from datetime import datetime

import numpy as np
from serial import Serial
from serial.tools import list_ports
from PyQt5.QtWidgets import QApplication, QMainWindow, QComboBox, QPushButton, QStatusBar, QScrollArea, QWidget, QHBoxLayout, QVBoxLayout, QLabel, QFrame, QCheckBox, QFileDialog, QErrorMessage
from PyQt5 import uic
from PyQt5.QtCore import QTimer
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg
from matplotlib.figure import Figure


class QVLine(QFrame):

    def __init__(self):
        super().__init__()
        self.setFrameShape(QFrame.VLine)


class QHLine(QFrame):

    def __init__(self):
        super().__init__()
        self.setFrameShape(QFrame.HLine)


class VariableView(QFrame):
    """Class to store and show a logged variable"""

    REPLOT_RATE = 200           # update rate for plots in ms
    PLOT_LENGTH = 2000          # maximum numer of points to plot
    PLOT_ONLY_LATEST = False    # only plot latest values

    def __init__(self, name="default"):
        super().__init__()
        self._pause = False
        self._variable_name = "default"
        self._variable_values = []
        self._variable_min = None
        self._variable_max = None
        self._btn_pause_resume = QPushButton()
        layout = QHBoxLayout()
        self.lbl_name = QLabel()
        self.lbl_value = QLabel()
        self.figure = Figure()
        self.canvas = FigureCanvasQTAgg(self.figure)
        self.canvas.maximumHeight = 100
        self.canvas.maximumWidth = 200
        self.ax = None
        layout.addWidget(self.lbl_name)
        layout.addWidget(QVLine())
        layout.addWidget(self.lbl_value)
        layout.addWidget(QVLine())
        layout.addWidget(self.canvas)
        layout.addStretch()
        layout.addWidget(self._create_buttons())
        self.setLayout(layout)
        self.set_variable_name(name)
        # use timer to replot data
        self.timer = QTimer()
        self.timer.timeout.connect(self.plot)
        self.timer.start(VariableView.REPLOT_RATE)

    def _create_buttons(self):
        frame = QFrame()
        layout = QVBoxLayout()
        layout.addStretch()
        frame.setLayout(layout)
        btn = QPushButton("Reset")
        btn.clicked.connect(self.reset)
        layout.addWidget(btn)
        self._btn_pause_resume = QPushButton("Pause")
        self._btn_pause_resume.clicked.connect(self._pause_resume)
        layout.addWidget(self._btn_pause_resume)
        btn = QPushButton("Export...")
        btn.clicked.connect(self.export)
        layout.addWidget(btn)
        layout.addStretch()
        return frame

    def _pause_resume(self):
        self._pause = not self._pause
        if self._pause:
            self._btn_pause_resume.setText("Resume")
        else:
            self._btn_pause_resume.setText("Pause")

    def _export(self, filepath):
        with open(filepath, "w") as f:
            f.write("value\n")
            for line in self._variable_values:
                if type(line) == list:
                    ret = ""
                    for val in line:
                        ret += str(val)+","
                    f.write(ret[:-1]+"\n")
                else:
                    f.write(str(line)+"\n")

    def export(self):
        """Export all values recorded to this point."""
        filepath, _ = QFileDialog.getSaveFileName(self, "Export values", datetime.now().strftime("%Y-%m-%d_%H-%M-%S") + "_" + self._variable_name + ".csv")
        if len(filepath) > 0:
            try:
                self._export(filepath)
            except Exception as e:
                QErrorMessage().showMessage(str(e))

    def set_variable_name(self, name):
        """Set the shown name of the logged variable."""
        self._variable_name = name
        self.lbl_name.setText(name)

    def add_variable_value(self, value):
        """Add a value to be logged."""
        if self._pause:
            return
        self._variable_values.append(value)
        if type(value) == int or type(value) == float:
            if self._variable_min == None:
                self._variable_min = value
            else:
                if self._variable_min > value:
                    self._variable_min = value
            if self._variable_max == None:
                self._variable_max = value
            else:
                if self._variable_max < value:
                    self._variable_max = value
            self.lbl_value.setText("min: " + str(self._variable_min) + "\ncurrent: " + str(value) + "\nmax: " + str(self._variable_max))
        else:
            self.lbl_value.setText(str(value))

    def reset(self):
        """Delete all values."""
        self._variable_values = []
        self.lbl_value.setText("")

    def plot(self):
        """Plot variable values in canvas"""
        if (len(self._variable_values) != 0) and (type(self._variable_values[0]) in [int, float]):
            if not self.ax:
                self.ax = self.figure.add_subplot(111)
            self.ax.clear()
            data = np.array(self._variable_values)
            # Decide on plotting type
            if len(data) > VariableView.PLOT_LENGTH:
                if VariableView.PLOT_ONLY_LATEST:
                    data = data[-VariableView.PLOT_LENGTH:]
                else:
                    # Optimization to limit amount of data to be plotted
                    data = data[np.linspace(0, len(data)-1, VariableView.PLOT_LENGTH, dtype=np.int)]
            self.ax.plot(data)
            self.canvas.show()
            self.canvas.draw()
        else:
            self.canvas.hide()
        self.timer.start(VariableView.REPLOT_RATE)


class App(QMainWindow):

    BAUDRATES = ["9600", "19200", "38400", "57600", "74880", "115200", "230400", "250000", "500000", "1000000", "2000000"]
    SERIAL_UPDATE_RATE = 100

    def __init__(self):
        super().__init__()
        self.ports = []
        self.serial = None
        self.line_buffer = ""
        self.tracked_variables = {}
        self.on_breakpoint = False
        uic.loadUi('SerialDebuggerInterface.ui', self)
        self.statusbar = self.findChild(QStatusBar, "statusbar")
        self.maincontainer = self.findChild(QScrollArea, "maincontainer")
        self.layout_main = QVBoxLayout()
        self.maincontainer.setLayout(self.layout_main)
        self.cb_port = self.findChild(QComboBox, "cb_port")
        self.cb_port.currentIndexChanged.connect(self.create_serial)
        self.cb_baud = self.findChild(QComboBox, "cb_baud")
        self.cb_baud.currentIndexChanged.connect(self.create_serial)
        self.cb_baud.addItems(App.BAUDRATES)
        self.cb_skip = self.findChild(QCheckBox, "cb_skip")
        self.cb_skip.stateChanged.connect(self.skip_breakpoint)
        self.btn_reset = self.findChild(QPushButton, "btn_reset")
        self.btn_reset.clicked.connect(self.reset)
        self.btn_next = self.findChild(QPushButton, "btn_next")
        self.btn_next.clicked.connect(self._btn_next_click)
        self.lbl_breakpoint = self.findChild(QLabel, "lbl_breakpoint")
        self.list_ports()
        self.show()
        # use timer for serial monitoring
        self.timer = QTimer()
        self.timer.timeout.connect(self.check_serial)
        self.timer.start(App.SERIAL_UPDATE_RATE)

    def _btn_next_click(self):
        self.skip_breakpoint()

    def create_serial(self):
        baud = int(self.cb_baud.itemText(self.cb_baud.currentIndex()))
        if len(self.ports) != 0:
            port = self.cb_port.itemText(self.cb_port.currentIndex())
            if self.serial: 
                self.serial.close()
            self.serial = Serial(port, baud)

    def list_ports(self):
        """Check available ports and add it to the combobox."""
        self.ports = list_ports.comports()
        self.cb_port.clear()
        self.cb_port.addItems([val.device for val in self.ports])

    def update_variable(self, name, value):
        """Register a variable by tracking it in the internal dictionary and adding a UI element for it."""
        if name in self.tracked_variables:
            self.tracked_variables[name].add_variable_value(value)
        else:
            var = VariableView(name)
            self.tracked_variables[name] = var
            self.layout_main.addWidget(var)
            var.show()

    def skip_breakpoint(self):
        """Send ok message to skip current breakpoint."""
        if not self.serial: 
            self.statusbar.showMessage("Serial not initialized")
        else:
            self.serial.write(b"ok")
            self.on_breakpoint = False

    def check_serial(self):
        """Check serial port for new data."""
        def evaluate_list(data):
            str2 = re.sub(r"[^\d_,.]", "", data)
            ret = []
            for x in str2.split(","):
                if x != "":
                    if "." in x:
                        ret.append(float(x))
                    else:
                        ret.append(int(x))
                else:
                    ret.append(0)
            return ret if len(ret)>1 else ret[0]
            
        if not self.serial: 
            self.statusbar.showMessage("Serial not initialized")
        else:
            self.statusbar.showMessage("Serial open")
            if self.serial.is_open and self.serial.in_waiting != 0:
                lines = self.serial.read(self.serial.in_waiting).split(b"\r\n")
                try:
                    lines = [line.decode("utf-8") for line in lines]
                    # use buffer to store unfinisched reads
                    lines[0] = self.line_buffer + lines[0]
                    if lines[-1] != "":
                        self.line_buffer = lines.pop()
                    else:
                        self.line_buffer = ""
                except Exception as e:
                    return
                for line in lines:
                    parts = line.split(" ")
                    if parts[0] == "log":
                        if parts[1] == "breakpoint":
                            if self.cb_skip.isChecked():
                                self.skip_breakpoint()
                            else:
                                self.on_breakpoint = True
                                self.lbl_breakpoint.setText("Breakpoint " + str(parts[2]))
                        elif parts[1] == "variable":
                            name = parts[2]
                            try:
                                val = evaluate_list(parts[3])
                            except Exception as e:
                                print(e)
                                print("---", line)
                                print("---", parts)
                                return
                            self.update_variable(name, val)
                        else:
                            print("Unknown log encountered: " + line)
                    elif parts[0] == "":
                        pass
                    else:
                        print("Unknown message encountered: " + line)
            else:
                self.statusbar.showMessage("Serial open, no messages")
        self.timer.start(App.SERIAL_UPDATE_RATE)

    def reset(self):
        """Reset all variable trackers."""
        for _, var in self.tracked_variables.items():
            # kill it. kill it with fire
            var.hide()
            self.layout_main.removeWidget(var)
            del var
        self.tracked_variables = {}
        self.maincontainer.update()
        self.layout_main.update()


if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = App()
    sys.exit(app.exec_())
