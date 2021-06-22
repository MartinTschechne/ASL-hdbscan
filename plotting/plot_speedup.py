import sys
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

sns.set()

assert len(sys.argv) == 4, "You need to provide a measurements file, \
the measurements file of the unoptimized version and string describing the applied optimizations including compiler optimization level.\n \
    Usage: python plot_measurements.txt measurements.csv O3"

opt_df = pd.read_csv(sys.argv[1], skiprows=7, index_col='Region')
base_df = pd.read_csv(sys.argv[2], skiprows=7, index_col='Region')
speedup_df = base_df/opt_df

fig, ax = plt.subplots(1, figsize=(7,7), dpi=100)
speedup_df.plot(kind='bar',rot=45, ax=ax, legend=False)
for p in ax.patches:
    ax.annotate(f"{p.get_height():.2f}",(p.get_x()+p.get_width()/2., p.get_height()),ha='center',va='center',xytext=(0, 10),textcoords='offset points')
plt.title(f'Speedup for {sys.argv[3]}')
plt.tight_layout()
# plt.savefig('speedup.pdf', bbox_inches='tight')
plt.show()
