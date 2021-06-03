import sys
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

sns.set()

assert len(sys.argv) == 3, "You need to provide a measurements file, \
and the compiler optimization level + other optimizations as string.\n \
    Usage: python plot_measurements.txt measurements.csv O0"

df = pd.read_csv(sys.argv[1], skiprows=7, index_col='Region')
fig, ax = plt.subplots(1, figsize=(7,7), dpi=100)
df.plot(kind='bar', rot=45, ax=ax, legend=False)
for p in ax.patches:
    ax.annotate(f"{p.get_height():.2E}",(p.get_x()+p.get_width()/2., p.get_height()),ha='center',va='center',xytext=(0, 10),textcoords='offset points')
plt.title(f'Cycle measurements for\n{sys.argv[2]}')
plt.ylabel("# Cycles")
plt.tight_layout()
plt.savefig('measurement.png', bbox_inches='tight')
