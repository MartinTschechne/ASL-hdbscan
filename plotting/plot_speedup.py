import sys
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

sns.set()

assert len(sys.argv) == 5, "You need to provide a measurements file, \
the measurements file of the unoptimized version, the compiler optimization level and the dimension of the data set.\n \
    Usage: python plot_measurements.txt measurements.csv 3 32"

opt_df = pd.read_csv(sys.argv[1], skiprows=4, index_col='Region')
base_df = pd.read_csv(sys.argv[2], skiprows=4, index_col='Region')
speedup_df = base_df/opt_df
speedup_df.plot(kind='bar')
plt.title(f'Speedup for -O{sys.argv[3]}, dim(data)={sys.argv[4]}\nOverall speedup={float(base_df.sum()/opt_df.sum()):.3f}')
plt.savefig('speedup.png', bbox_inches='tight')
