from PyQt5.QtWidgets import QTableView
from PyQt5.QtCore import Qt, pyqtSlot, QAbstractTableModel, QTimer, QVariant, \
                         QModelIndex
from pipe_logger import PipeReader

class NetworkDataTableModel(QAbstractTableModel):

    def __init__(self, output_file, parent=None): 
        super(NetworkDataTableModel, self).__init__()
        self.column_titles = ["Src-Dest", "Subscription", "Size", "Index",
                              "Time Sent ns", "Latency ms"]
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

        # output file
        self.outfile = open(output_file, "w")

    def headerData(self, section, orientation, role=Qt.DisplayRole):
        if role == Qt.DisplayRole and orientation == Qt.Horizontal:
            return self.column_titles[section]
        else:
            return QVariant()

#  // message summary
#  std::stringstream ss;
#  ss << msg->source << "-" << r_ptr->r << "," // src-dest
#     << msg->message_name << ","              // subscription name
#     << msg->message.size() << ","            // size of these records
#     << msg->message_number << ","            // message number
#     << msg->nanoseconds << ","               // time sent in nanoseconds
#     << (_now_nanoseconds() - msg->nanoseconds) / 1000000.0; // delta ms

    @pyqtSlot()
    def update_data(self):
        queue = self.pipe_reader.queue
        new_network_data = dict()
        while not queue.empty():
            line = queue.get()
            self.outfile.write(line+"\n")
            parts = line.split(",")

            # (R1-R2,subscription)
            key = (parts[0],parts[1])
            # [size, message#, latency]
            value = [int(parts[2]),int(parts[3]), int(parts[4]),
                     float(parts[5])]
            new_network_data[key]=value
        self.set_data(new_network_data)
        self.outfile.flush()

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

