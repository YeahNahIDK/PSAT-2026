import matplotlib.pyplot as plt
import pandas as pd
from pathlib import Path

# Getting annoying error, this directly finds the directory
script_location = Path(__file__).resolve().parent
file_path = script_location / "test-data.csv"

df = pd.read_csv(file_path)
ax = plt.figure().add_subplot(projection='3d')


ax.plot(df["Disp_X"], df["Disp_Y"], df["Disp_Z"], label='Trajectory')

# Makes each axis unit the same size
x_range = df["Disp_X"].max() - df["Disp_X"].min()
y_range = df["Disp_Y"].max() - df["Disp_Y"].min()
z_range = df["Disp_Z"].max() - df["Disp_Z"].min()
ax.set_box_aspect((x_range, y_range, z_range))

# Make legend, set axes limits and labels
ax.legend()
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')

# Customize the view angle so it's easier to see that the scatter points lie
# on the plane y=0
ax.view_init(elev=20., azim=-35, roll=0)

plt.show()