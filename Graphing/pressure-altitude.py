import matplotlib.pyplot as plt
import pandas as pd
from pathlib import Path

# Getting annoying error, this directly finds the directory
script_location = Path(__file__).resolve().parent
file_path = script_location / "test-data.csv"

df = pd.read_csv(file_path)
fig = plt.figure()
ax = fig.add_subplot(projection='3d')


ax.plot(df["Time"], df["Pressure"], df["Disp_Z"], label="Pressure", linewidth=2, alpha=1, color="Blue")

# Make legend, set axes limits and labels
ax.legend()
ax.set_xlabel('Time')
ax.set_ylabel('Pressure')
ax.set_zlabel('Displacement Z')

plt.show()