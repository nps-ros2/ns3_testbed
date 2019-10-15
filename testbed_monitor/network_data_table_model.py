from PyQt5.QtWidgets import QTableView
from PyQt5.QtCore import Qt, pyqtSlot, QAbstractTableModel, QTimer, QVariant, \
                         QModelIndex
from pipe_logger import PipeReader

class NetworkDataTableModel(QAbstractTableModel):

    def __init__(self, _queue, parent=None): 
        super(NetworkDataTableModel, self).__init__()
        self.column_titles = ["Src", "Dest", "Subscription", "Size", "Tx Count",
                              "Rx Count", "Latency ms"]
        self.table_data = dict()
        self.unsorted_keys = list()

#        # sample data
#        self._set_data({("a","b","c"):(1,2,3,4)})

        # queue 
        self.queue = _queue

        # queue management
        timer = QTimer(self)
        timer.timeout.connect(self.update_data)
        timer.start(1000) # once per second

    def headerData(self, section, orientation, role=Qt.DisplayRole):
        if role == Qt.DisplayRole and orientation == Qt.Horizontal:
            return self.column_titles[section]
        else:
            return QVariant()

    # called by 1 second timer
    @pyqtSlot()
    def update_data(self):
        queue = self.queue  # clarity
        tx_data = dict()
        rx_data = dict()
        while not queue.empty():
            line = queue.get()
            l = line.split(",")

            if len(l) == 5:
                # publisher:
                # from, to, subscription, tx count, timestamp
                # key=(from,to,subscription)
                # value=(tx_count, timestamp)
                tx_data[l[0],l[1],l[2]]=l[3],l[4]
            elif len(l) == 7:
                # subscriber:
                # from, to, subscription, tx_count, rx_count, size, timestamp
                # key=(from,to,subscription)
                # value=(tx_count, rx_count, size, timestamp)
                rx_data[l[0],l[1],l[2]]=l[3],l[4],l[5],l[6]
            else:
                print("bad data from pipe: '%s'"%line)

        # build table data from first tx_data and matching rx_data
        # from this timed period
        first_tx_data = dict()

        # get first tx values
        for key, value in tx_data.items():
            if not key in first_tx_data:
                first_tx_data[key] = value

        # create table data
        # key=(from,to,subscription)
        # value=(size, tx_count, rx_count, latency)
        new_table_data = dict()

        for key, value in rx_data.items():
            if key in first_tx_data and not key in new_table_data:

                # add entry if count matches
                if value[0] == first_tx_data[key][0]: # counts match
                    latency=(int(value[3]) - int(first_tx_data[key][1])) \
                                                 / 1000000.0 # delta ms
                    new_table_data[key] = (int(value[2]), int(value[0]),
                                       int(value[1]), latency)
        self._set_data(new_table_data)

    def _set_data(self, new_table_data):

        self.beginResetModel()

        # clear existing values
        zeros = [0]*4
        for key in self.unsorted_keys:
            self.table_data[key] = zeros

        # set new values
        for key, value in new_table_data.items():
            self.table_data[key]=value

        self.endResetModel()

        self.unsorted_keys = list(self.table_data.keys())

    def rowCount(self, parent=QModelIndex()):
        return len(self.table_data)

    def columnCount(self, parent=QModelIndex()):
        return len(self.column_titles)

    def data(self, index, role=Qt.DisplayRole):
        if role == Qt.DisplayRole:
            row = index.row()
            column = index.column()
            key = self.unsorted_keys[row]
            if column < 3:
                return key[column]
            else:
                value = self.table_data[key]
            return value[column-3]
        else:
            return QVariant()

