# importing numpy module
import numpy

file1 = open("log_isolation.txt", "r")
lines = file1.readlines()

task_1_lines = []
task_2_lines = []
task_3_lines = []
task_4_lines = []

separated_data = [task_1_lines, task_2_lines, task_3_lines, task_4_lines]

which_task = -1
data_line = False

for line in lines:
    line.strip()
    if(line.startswith("CORE")):
        data_line = True            
        which_task =  int(line[9])
    elif data_line is True:
        separated_data[which_task-1].append(line)
        data_line = False

print("Data sample size per task: " + str(len(task_1_lines)))
print()

for task_count, data in enumerate(separated_data):

    read_times = []
    exec_times = []
    write_times = []
    pass_times = []
    times = [read_times, exec_times, write_times, pass_times]

    for line in data:
        phase_separation = line.split(", ")
        for index, phase in enumerate(phase_separation):
            phase_split = phase.split(" ")
            times[index].append(int(phase_split[1]))
    
    read = []
    exec = []
    write = []
    for i, time in enumerate(read_times):
        read.append(exec_times[i] - read_times[i])
    for i, time in enumerate(exec_times):
        exec.append(write_times[i] - exec_times[i])   
    for i, time in enumerate(write_times):
        write.append(pass_times[i] - write_times[i])

    print("TASK " + str((task_count + 1)))
    print("READ avg: " + str(numpy.average(read)))
    # print(read)
    print("EXECUTE avg: " + str(numpy.average(exec)))
    # print(exec)
    print("WRITE avg: " + str(numpy.average(write)))
    # print(write)
    print()



file1.close()