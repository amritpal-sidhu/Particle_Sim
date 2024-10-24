import os
import re


def parse_logfile_for_physics_data(logfile):

    pattern = r"^.*DATA:.*]\.(\w+) = <([\d\D]+), ([\d\D]+), ([\d\D]+)>$"
    physics_data = {
        "position":[],
        "momentum": [],
        "orientation": [],
        "angular_momentum": []
    }

    with open(logfile, "r") as file:
        for line in file:
            match = re.match(pattern, line)
            if match:
                vector_type = match.group(1).lower()
                components = (float(match.group(2)), float(match.group(3)), float(match.group(4)))

                if vector_type == "position":
                    physics_data["position"].append(components)
                elif vector_type == "momentum":
                    physics_data["momentum"].append(components)
                elif vector_type == "orientation":
                    physics_data["orientation"].append(components)
                elif vector_type == "angular_momentum":
                    physics_data["angular_momentum"].append(components)
                    
    return physics_data

def print_physics_data(physics_data):
    for key, value in physics_data.items():
        for v in value:
            print(f'{key}: {v}')


'''
Entry point
'''
if __name__ == "__main__":
    logfile = os.path.join(os.path.dirname(__file__), "../_build/bin/Debug/debug_output.txt")
    print_physics_data(parse_logfile_for_physics_data(logfile))

'''
Plot trace of particle motion
'''
