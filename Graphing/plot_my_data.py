import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from pathlib import Path

# Load CSV
script_location = Path(__file__).resolve().parent
df = pd.read_csv(script_location / "test-data.csv")

t = df["Time"].astype(float).to_numpy()

# 3D trajectory 
fig = plt.figure()
ax = fig.add_subplot(projection="3d")

ax.plot(df["Disp_X"], df["Disp_Y"], df["Disp_Z"], label="Trajectory", linewidth=2, alpha=0.6)

acc_mag = np.sqrt(df["Acc_X"]**2 + df["Acc_Y"]**2 + df["Acc_Z"]**2)
sc = ax.scatter(df["Disp_X"], df["Disp_Y"], df["Disp_Z"], c=acc_mag, cmap="plasma")

cbar = fig.colorbar(sc, ax=ax, pad=0.1)
cbar.set_label("Acceleration Magnitude (m/s²)", rotation=270, labelpad=20)

x_range = df["Disp_X"].max() - df["Disp_X"].min()
y_range = df["Disp_Y"].max() - df["Disp_Y"].min()
z_range = df["Disp_Z"].max() - df["Disp_Z"].min()
ax.set_box_aspect((max(x_range,1e-6), max(y_range,1e-6), max(z_range,1e-6)))

ax.set_xlabel("Disp_X")
ax.set_ylabel("Disp_Y")
ax.set_zlabel("Disp_Z")
ax.legend()
ax.view_init(elev=20, azim=-35)

# Disp_Z vs Time
plt.figure()
plt.plot(t, df["Disp_Z"], linewidth=2)
plt.xlabel("Time (s)")
plt.ylabel("Disp_Z (m)")
plt.title("Vertical Position vs Time")

# Velocity 
plt.figure()
plt.plot(t, df["Vel_X"], label="Vel_X")
plt.plot(t, df["Vel_Y"], label="Vel_Y")
plt.plot(t, df["Vel_Z"], label="Vel_Z", linewidth=2)
plt.xlabel("Time (s)")
plt.ylabel("Velocity")
plt.title("Velocity vs Time")
plt.legend()

# Acc magnitude 
plt.figure()
plt.plot(t, acc_mag, linewidth=2)
plt.xlabel("Time (s)")
plt.ylabel("Acceleration magnitude (m/s²)")
plt.title("Acceleration Magnitude vs Time")

# Angles 
plt.figure()
plt.plot(t, df["Ang_R"], label="Roll")
plt.plot(t, df["Ang_P"], label="Pitch")
plt.plot(t, df["Ang_Y"], label="Yaw")
plt.xlabel("Time (s)")
plt.ylabel("Angle")
plt.title("Angles vs Time")
plt.legend()

# Pressure 
plt.figure()
plt.plot(t, df["Pressure"])
plt.xlabel("Time (s)")
plt.ylabel("Pressure (Pa)")
plt.title("Pressure vs Time")

# ---------- Temperature ----------
plt.figure()
plt.plot(t, df["Temp"])
plt.xlabel("Time (s)")
plt.ylabel("Temperature (°C)")
plt.title("Temperature vs Time")

plt.show()
