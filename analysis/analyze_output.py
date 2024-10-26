import os, sys, re
import matplotlib.pyplot as plt


metrics = ("position", "momentum", "orientation", "angular_momentum")

'''
Read the DATA log levels within "logfile" argument and
return a list of dictionaries containing the physics metrics for
each particle
'''
def parse_log_file(logfile):

    pattern = r"^.*DATA:.*particle\[(\d+)\]\.(\w+) = <([\d\D]+), ([\d\D]+), ([\d\D]+)>$"
    data = {metrics[i]:[[]] for i in range(len(metrics))}

    with open(logfile, "r") as file:
        for line in file:
            match = re.match(pattern, line)
            if match:
                id = int(match.group(1))
                vector_type = match.group(2).lower()
                components = (float(match.group(3)), float(match.group(4)), float(match.group(5)))
                
                if len(data[vector_type]) < id+1:
                    for _ in range((id+1)-len(data[vector_type])):
                        data[vector_type].append([])
                
                data[vector_type][id].append(components)
    
    return data

'''
Print the types and length of each component of the
particle data object
'''
def print_metadata(data):
    print(f"Metrics:    {type(data)}({len(data)}), ")
    print(f"Particles:  {type(data[metrics[0]])}({len(data[metrics[0]])}), ")
    print(f"Samples:    {type(data[metrics[0]][0])}({len(data[metrics[0]][0])}), ")
    print(f"Coords:     {type(data[metrics[0]][0][0])}({len(data[metrics[0]][0][0])})")


'''
Print to stdout the physics metrics for each particle.  Each
sample for a particle is printed on the same row.
This is to check the data is read in correctly.
'''
def print_data(data):
    for id in range(len(data[metrics[0]])):
        for sample in range(len(data[metrics[0]][id])):
            print(f"{sample+id*(len(metrics)-1):7}:", end="")
            for m in range(len(metrics)):
                print(f"  {metrics[m]}[{id}]<{data[metrics[m]][id][sample]}>", end="")
            print(f"")

'''
Plot all four physics metrics for each particle in a
seperate figure
'''
def plot_data(data):
    for id in range(len(data)):
        fig = plt.figure(id, figsize=[12,10])
        fig.suptitle(f"Particle {id}")
        for m in range(len(metrics)):
            ax = fig.add_subplot(2, 2, int(m+1), projection="3d")
            x, y, z = enumerate(zip(data[metrics[m]][id]))
            ax.scatter(x, y , z)
            ax.set_title(metrics[m])
    plt.show()

def print_usage():
    print("Usage: python analyze_output.py [log file path]\n")
    exit()

def parse_args():
    if len(sys.argv) != 2:
        print_usage()
    else:
        log_file = os.path.abspath(sys.argv[1])
        if not os.path.isfile(log_file):
            print("[log file path] is invalid")
            print_usage()

    return log_file


'''
Entry point
'''
if __name__ == "__main__":

    data = parse_log_file(parse_args())
    print_data(data);print(f"\n")
    print_metadata(data);print(f"\n")
    plot_data(data)
    
