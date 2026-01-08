import matplotlib.pyplot as plt
import numpy as np

ax = plt.figure().add_subplot(projection='3d')

# Plot a sin curve using the x and y axes.
x = np.random.sample(100) * 100
y = np.random.sample(100) * 40
z = np.random.sample(100) * 50
ax.plot(x, y, z, label='curve in (x, y)')

x_range = x.max() - x.min()
y_range = y.max() - y.min()
z_range = z.max() - z.min()
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