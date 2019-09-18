#!/usr/bin/env python3

from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
import sys
from PyQt5.QtWidgets import QApplication
from PyQt5.QtWidgets import QMainWindow

from gui_manager import GUIManager

# main
if __name__=="__main__":
    parser = ArgumentParser(description="GUI for graphing "
                                        "network characteristics.",
                            formatter_class=ArgumentDefaultsHelpFormatter)
    parser.add_argument("-o","--output_file", type=str,
                        help="The CSV output file of latency data",
                        default="_output_file.csv")
    args = parser.parse_args()

    # create the "application" and the main window
    application = QApplication(sys.argv)
    main_window = QMainWindow()

    gui_manager = GUIManager(main_window, args.output_file)

    # start the GUI
    gui_manager.w.show()
    sys.exit(application.exec_())

