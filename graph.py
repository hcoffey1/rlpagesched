"""
Student : Hayden Coffey
ECE 517

This file is a bit of a mess, it was modified
to graph whatever I needed at the time.
"""
import matplotlib.pyplot as plt
import os

# Encodings for learning methods
MC_EV = 1
QL = 2

# Encodings for policy types
GREEDY = 0
E_GREEDY = 1
UNIFORM = 2

#directory = "./resTime/"


class DataRecord:
    def __init__(self, d, a, e, n, m, r, pi):
        self.d = int(d)
        self.a = float(a)
        self.e = float(e)
        self.n = int(n)
        self.m = int(m)
        self.r_policy = int(r)
        self.pi_policy = int(pi)

    data = []
    time = None


dataRecords = []

#for file in os.listdir(directory):
#splitFile = (file.split('_'))A
file1 = "medium_2q_10000.log"
file2 = "medium_2q_0.5d_10000.log"
file3 = "medium_2q_0.9d_1hd_10000.log"
file4 = "medium_2q_0.5d_1hd_10000.log"
file5 = "timeOut"
history_val = 3622753
#targetDir = directory + file

#tmp = DataRecord(splitFile[0], splitFile[1], splitFile[2], splitFile[3],
#                    splitFile[4], splitFile[5], splitFile[6])
tmp_data1 = []
with open(file1) as f:
    for line in f:
        sp = (line.split())
        if sp[0] == "STATISTICS":
            continue
        if sp[0] == "Time":
            tmp_data1.append(int(sp[1])/(1.0*history_val))

tmp_data2 = []
with open(file2) as f:
    for line in f:
        sp = (line.split())
        if sp[0] == "STATISTICS":
            continue
        if sp[0] == "Time":
            tmp_data2.append(int(sp[1])/(1.0*history_val))

tmp_data3 = []
with open(file3) as f:
    for line in f:
        sp = (line.split())
        if sp[0] == "STATISTICS":
            continue
        if sp[0] == "Time":
            tmp_data3.append(int(sp[1])/(1.0*history_val))

tmp_data4 = []
with open(file4) as f:
    for line in f:
        sp = (line.split())
        if sp[0] == "STATISTICS":
            continue
        if sp[0] == "Time":
            tmp_data4.append(int(sp[1])/(1.0*history_val))

tmp_data5 = []
with open(file5) as f:
    for line in f:
        sp = (line.split())
        minute = int((sp[1].split('m')[0]))
        second = int((sp[1].split('m')[1].split('.')[0]))
        tmp_data5.append(minute*60 + second)

fig, ax1 = plt.subplots()

#import matplotlib.pyplot as plt
# line 1 points
indx1 = 2
indx2 = 1


title = "Relative Performance vs Episode Count"
#title = "Return vs Epochs, RP:{}, ({} Episodes)".format(r_code,n)

xlabel = "Episode Count"
ylabel = "Performance Relative to History Scheduler"

# plotting the line 1 points
plt.plot(tmp_data1, label="lr=0.1, discount=0.9, history_depth=2")
# plotting the line 2 points
plt.plot(tmp_data2, label="lr=0.1, discount=0.5, history_depth=2")

plt.xlabel(xlabel)
# Set the y axis label of the current axis.
plt.ylabel(ylabel)
# Set a title of the current axes.
plt.title(title)
# show a legend on the plot
plt.legend()
# Display a figure.
plt.show()

plt.plot(tmp_data3, label="lr=0.1, discount=0.9, history_depth=1")
plt.xlabel(xlabel)
# Set the y axis label of the current axis.
plt.ylabel(ylabel)
# Set a title of the current axes.
plt.title(title)
# show a legend on the plot
plt.legend()

plt.show()

xlabel = "Number of Trained Pages"
ylabel = "Time to Execute 1000 Epochs (s)"

plt.plot(tmp_data5)
plt.xlabel(xlabel)
# Set the y axis label of the current axis.
plt.ylabel(ylabel)
# Set a title of the current axes.
plt.title(title)

plt.show()