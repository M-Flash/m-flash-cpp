import numpy as np
n = 12
a = np.zeros( [n,n])
a[0,4] = 1
a[0,7] = 1
a[0,8] = 1
a[1,7] = 1
a[2,7] = 1
a[3,5] = 1
a[3,6] = 1
a[4,0] = 1
a[4,5] = 1
a[4,7] = 1
a[5,3] = 1
a[5,4] = 1
a[5,6] = 1
a[6,3] = 1
a[6,5] = 1
a[7,0] = 1
a[7,1] = 1
a[7,2] = 1
a[7,4] = 1
a[8,0] = 1
a[8,9] = 1
a[8,10] = 1
a[8,11] = 1
a[9,8] = 1
a[10,8] = 1
a[11,8] = 1


at = a.transpose()
out_degree = np.ones([n,1])
out_degree = np.matmul(at,out_degree)
out_degree = 1/out_degree

destination = np.ones([n,1]) /n
for i in range(1,4):
 destination *= out_degree  
 destination = np.matmul(a,destination) * 0.85 + (0.15/n)

