import os

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


df = pd.DataFrame(columns=np.arange(9))

'''
Parsing could possibly done in a better way
'''
try:

    split_string = " seconds] DATA: "
    columns = {}
    i = 0

    file_handle = open(os.path.join(os.path.dirname(__file__), "../_build/bin/debug_output.txt"), "r")
    file_lines = file_handle.readlines()

    for line in file_lines:

        if split_string in line:
            
            _, line = line.split(split_string)
            line = line.replace('\n', '').split(',')

            if i == 0:
                i += 1
                df.columns = columns = line
            else:
                df = pd.concat([df, pd.DataFrame([line], columns=columns, dtype=float)], ignore_index=True)

    file_handle.close()

except (FileNotFoundError, PermissionError):
    print("Failed to open debug_output.txt")
    exit()


'''
Correct dataframe population check
'''
print(df.head())


'''
Plot trace of particle motion
'''
unique_ids = []

for id in df.particle_id:
    if id not in unique_ids:
        unique_ids.append(id)

fig, axis = plt.subplots(1, 2)

for id in unique_ids:
    axis[0].plot(df.loc[df.particle_id == id].x_pos, df.loc[df.particle_id == id].y_pos)
    axis[1].plot(df.loc[df.particle_id == id].x_momenta, df.loc[df.particle_id == id].y_momenta)
    

axis[0].title.set_text("Position")
axis[1].title.set_text("Momenta")
plt.show()
