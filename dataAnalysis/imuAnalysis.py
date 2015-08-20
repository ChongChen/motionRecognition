import numpy as np
import matplotlib.pyplot as plt

dir = '../data/'
fileName = dir + 'still_normal.txt'
"""fileName = dir + 'still_leftDown.txt'

fileName = dir + 'still_rightDown.txt'

fileName = dir + 'still_frontDown.txt'
fileName = dir + 'still_leftDownLean.txt'
fileName = dir + 'still_rightDownLean.txt'
fileName = dir + 'still_frontDownLean.txt'
fileName = dir + 'motion_rightUpRotation3.txt'
fileName = dir + 'motion_leftUpRotation3.txt'
fileName = dir + 'motion_leftRightRotation3.txt'
fileName = dir + 'motion_frontBackRotation3.txt'
fileName = dir + 'motion_xMove.txt'
fileName = dir + 'motion_zMove.txt'
"""

fin = open(fileName)

lines = fin.readlines()

res = []
for str in lines:
	tmp = str.split()
	tmp = map(eval, tmp)
	res.append(tmp)

res_array = np.array(res)

f, ax = plt.subplots(3)

ax[0].plot(res_array[10:110, 0], 'b', label='acc-x')
ax[0].plot(res_array[10:110, 1], 'g', label='acc-y')
ax[0].plot(res_array[10:110, 2], 'r', label='acc-z')
ax[0].legend()

ax[1].plot(res_array[10:110, 3], 'b', label='gyr-x')
ax[1].plot(res_array[10:110, 4], 'g', label='gyr-y')
ax[1].plot(res_array[10:110, 5], 'r', label='gyr-z')
ax[1].legend()

ax[2].plot(res_array[10:110 ,6], 'b', label='mag-x')
ax[2].plot(res_array[10:110, 7], 'g', label='mag-y')
ax[2].plot(res_array[10:110, 8], 'r', label='mag-z')
ax[2].legend()
plt.xlabel('time')
plt.show()
