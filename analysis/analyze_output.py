import os
import re


def parse_logfile_for_physics_data(filepath):

    pattern = r"^.*DATA:.*]\.(\w+) = <([\d\D]+), ([\d\D]+), ([\d\D]+)>$"
    physics_data = {
        "position":[],
        "momentum": [],
        "orientation": [],
        "angular_momentum": []
    }

    with open(filepath, "r") as file:
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


if __name__ == "__main__":
    filepath = os.path.join(os.path.dirname(__file__), "../_build/bin/Debug/debug_output.txt")
    physics_data = parse_logfile_for_physics_data(filepath)
    for key, value in physics_data.items():
        print(f'{key}: {value}')

'''
Plot trace of particle motion
'''
# unique_ids = []

# for id in df.particle_id:
#     if id not in unique_ids:
#         unique_ids.append(id)

# fig, axis = plt.subplots(1, 2)

# for id in unique_ids:
#     axis[0].plot(df.loc[df.particle_id == id].x_pos, df.loc[df.particle_id == id].y_pos)
#     axis[1].plot(df.loc[df.particle_id == id].x_momenta, df.loc[df.particle_id == id].y_momenta)
    

# axis[0].title.set_text("Position")
# axis[1].title.set_text("Momentum")
# plt.show()
