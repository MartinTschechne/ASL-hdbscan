import sys
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

sns.set()

assert len(sys.argv) == 4, "You need to provide a measurements file, \
the compiler optimization level and the dimension of the data set.\n \
    Usage: python plot_measurements.txt measurements.csv 3 32"

df = pd.read_csv(sys.argv[1], skiprows=4, index_col='Region')
df.plot(kind='bar', logy=True)
plt.title(f'Cycle measurements for -O{sys.argv[2]}, dim(data)={sys.argv[3]}')
plt.savefig('measurement.png', bbox_inches='tight')
