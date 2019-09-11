from PyQt5.QtWidgets import QTableView
from PyQt5.QtCore import Qt, pyqtSlot, QAbstractTableModel, QTimer, QVariant, \
                         QModelIndex
from pipe_logger import PipeReader

class NetworkDataTableModel(QAbstractTableModel):

    def __init__(self, column_titles, parent=None): 
        super(NetworkDataTableModel, self).__init__()
        self.column_titles = column_titles
        self.network_data = dict()
        self.unsorted_keys = list()

#        # fake data
#        self.set_data({("a","b"):(3,4,5)})

        # queue from pipe reader
        self.pipe_reader = PipeReader()

        # queue management
        timer = QTimer(self)
        timer.timeout.connect(self.update_data)
        timer.start(1000) # once per second

    def headerData(self, section, orientation, role=Qt.DisplayRole):
        if role == Qt.DisplayRole and orientation == Qt.Horizontal:
            return self.column_titles[section]
        else:
            return QVariant()

    @pyqtSlot()
    def update_data(self):
        queue = self.pipe_reader.queue
        new_network_data = dict()
        while not queue.empty():
            parts = queue.get().split(",")
            key = (parts[0],parts[1])
            value = parts[2:]
            new_network_data[key]=value
        self.set_data(new_network_data)

    def set_data(self, network_data):

        self.beginResetModel()

        # clear existing values
        zeros = [0]*(len(self.column_titles)-2)
        for key in self.unsorted_keys:
            self.network_data[key] = zeros

        # set new values
        for key, value in network_data.items():
            self.network_data[key]=value

        self.endResetModel()

        self.unsorted_keys = list(self.network_data.keys())

        # print array of 10 latency values for odometry
        vector = list()
        for i in range(1,11):
            key = ("R%d"%i,"odometry")
            if key in self.unsorted_keys:
                vector.append(float(self.network_data[key][2]))
            else:
                vector.append(0)
        print("%f,%f,%f,%f,%f,%f,%f,%f,%f,%f"%tuple(vector))

    def rowCount(self, parent=QModelIndex()):
        return len(self.network_data)

    def columnCount(self, parent=QModelIndex()):
        return len(self.column_titles)

    def data(self, index, role=Qt.DisplayRole):
        if role == Qt.DisplayRole:
            row = index.row()
#            print("data row: %d, of %d"%(row, len(self.unsorted_keys)))
            column = index.column()
            key = self.unsorted_keys[row]
            if column < 2:
                return key[column]
            else:
                value = self.network_data[key]
#                print("data: %s %s %s"%(row, column, self.network_data[key]))
#                print(type(value[column-2]))
            return value[column-2]
        else:
            return QVariant()

