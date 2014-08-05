#!/usr/bin/env python2

import sys
import numpy as np
import matplotlib.pyplot as plt


def convert_c(name):
    cbresult = np.fromfile(name, dtype=np.uint64)
    side = np.sqrt(len(cbresult))
    cbresult = cbresult.reshape((side, side))
    print np.max(cbresult)
    visual_max = np.percentile(cbresult.flatten(), 99.9)
    print visual_max
    plt.imsave("%s.png" % name[:-4], cbresult, cmap='binary_r', vmin=1, vmax=visual_max)

    
if len(sys.argv) == 1:
    convert_c('pic.bin')
else:
    map(convert_c, sys.argv[1:])