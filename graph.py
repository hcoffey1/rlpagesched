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
    #lines = f.readlines()
    #for x in lines[0].split():
    #    tmp_data.append(float(x))

    #tmp.time = float(lines[1])
#print(tmp_data)
#tmp.data = tmp_data.copy()
#dataRecords.append(tmp)

#timeData_1_0 = {}
#timeData_1_1 = {}
#timeData_2_0 = {}
#timeData_2_1 = {}
#for data in dataRecords:
#    if data.r_policy == 0 and data.m == MC_EV:
#        timeData_1_0[data.n] = (data.time)
#
#    if data.r_policy == 0 and data.m == QL:
#        timeData_2_0[data.n] = (data.time)
#
#    if data.r_policy == 1 and data.m == MC_EV:
#        timeData_1_1[data.n] = (data.time)
#
#    if data.r_policy == 1 and data.m == QL:
#        timeData_2_1[data.n] = (data.time)
#
#keys = sorted(timeData_1_1.keys(), reverse=False)
#x1 = []
#for key in keys:
#    x1.append(timeData_1_1[key])
#
#keys = sorted(timeData_2_1.keys(), reverse=False)
#x2 = []
#for key in keys:
#    x2.append(timeData_2_1[key])

fig, ax1 = plt.subplots()

#import matplotlib.pyplot as plt
# line 1 points
indx1 = 2
indx2 = 1

#if dataRecords[indx1].m == QL:
#    method_str1 = "Q-Learning"
#elif dataRecords[indx1].m == MC_EV:
#    method_str1 = "Every-Visit MC"
#
#if dataRecords[indx2].m == QL:
#    method_str2 = "Q-Learning"
#elif dataRecords[indx2].m == MC_EV:
#    method_str2 = "Every-Visit MC"

#r_code = dataRecords[indx1].r_policy
#n = dataRecords[indx1].n

title = "Relative Performance vs Episode Count"
#title = "Return vs Epochs, RP:{}, ({} Episodes)".format(r_code,n)

xlabel = "Episode Count"
ylabel = "Performance Relative to History Scheduler"

#xlabel = "Epochs (Averaged by groups of {})".format(n/100)
#ylabel = "Summed Return"

#x1 = list(timeData_1_0.values())
#x2 = list(timeData_2_0.values())
#x1 = dataRecords[indx1].data
#x2 = dataRecords[indx2].data

# plotting the line 1 points
plt.plot(tmp_data1, label="lr=0.1, discount=0.9, history_depth=2")
# line 2 points
#x2 = [10,20,30]
#y2 = [40,10,30]
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