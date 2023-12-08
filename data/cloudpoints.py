import numpy as np
from scipy.stats import qmc

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import proj3d

rng = np.random.default_rng()
engine = qmc.PoissonDisk(d=3, radius=0.2, seed=138, hypersphere='surface')
sample = engine.fill_space()

print(len(sample))
print(min(sample[:, 0]), max(sample[:, 0]))
print(min(sample[:, 1]), max(sample[:, 1]))
print(min(sample[:, 2]), max(sample[:, 2]))

print(sample)

fig = plt.figure(figsize=(12,7))
ax = fig.add_subplot(projection='3d')
img = ax.scatter(sample[:, 0], sample[:, 1], sample[:, 2])
fig.colorbar(img)

ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')

plt.show()