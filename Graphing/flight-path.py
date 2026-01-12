import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from pathlib import Path

# Getting annoying error, this directly finds the directory
script_location = Path(__file__).resolve().parent
file_path = script_location / "test-data.csv"

df = pd.read_csv(file_path)
fig = plt.figure()
ax = fig.add_subplot(projection='3d')


ax.plot(df["Disp_X"], df["Disp_Y"], df["Disp_Z"], label='Trajectory', linewidth=2, alpha=0.8, color="gray")

df['Acc_Mag'] = np.sqrt(df['Acc_X']**2 + df['Acc_Y']**2 + df['Acc_Z']**2)
sc = ax.scatter(df['Disp_X'], df['Disp_Y'], df['Disp_Z'], c=df['Acc_Mag'], cmap='plasma', alpha=1)

cbar = fig.colorbar(sc, ax=ax, pad=0.1)
cbar.set_label('Acceleration Magnitude (m/s²)', rotation=270, labelpad=20)

# Makes each axis unit the same size
x_range = df["Disp_X"].max() - df["Disp_X"].min()
y_range = df["Disp_Y"].max() - df["Disp_Y"].min()
z_range = df["Disp_Z"].max() - df["Disp_Z"].min()
ax.set_box_aspect((x_range, y_range, z_range))

# Make legend, set axes limits and labels
ax.legend()
ax.set_xlabel('Displacement X', labelpad=20)
ax.set_ylabel('Displacement Y', labelpad=10)
ax.set_zlabel('Displacement Z', labelpad=5)

# Tick label rotations
ax.tick_params(axis='x', rotation=50)
ax.tick_params(axis='y', rotation=-50)

# Customize the view angle so it's easier to see that the scatter points lie
# on the plane y=0
ax.view_init(elev=20., azim=-35, roll=0)

plt.show()