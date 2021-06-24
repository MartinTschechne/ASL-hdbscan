# python plotting/plot_stacked_speedup.py results/opt_level/measurements_runner_bitset_nocalc_avx_unroll_2.txt results/opt_level/measurements_runner_O0.txt results/opt_level/measurements_runner_no_optimization.txt results/opt_level/measurements_runner_full.txt "-O3 bitset_nocalc_avx_unroll_2"

import pandas as pd
import sys
import matplotlib.pyplot as plt
import seaborn as sns

sns.set()

assert len(sys.argv) == 6, "You need to provide a measurements file, \
the measurements file of the unoptimized version and string describing the applied optimizations including compiler optimization level.\n \
    Usage: python full_opt.txt base_opt.txt o3.txt full.txt measurements.csv 'O3'"

opt_df = pd.read_csv(sys.argv[1], skiprows=7, index_col='Region')
base_df = pd.read_csv(sys.argv[2], skiprows=7, index_col='Region')
o3_df = pd.read_csv(sys.argv[3], skiprows=7, index_col='Region')
dist_df = pd.read_csv(sys.argv[4], skiprows=7, index_col='Region')


speedup_df = base_df / opt_df
speedup_o3 = base_df / o3_df
speedup_dist = base_df / dist_df

fig, ax = plt.subplots(1, figsize=(7, 7), dpi=100)

color = sns.color_palette()
speedup_df.plot(kind='bar', rot=45, ax=ax, legend=False, logy=True, color=color[2])
speedup_dist.plot(kind='bar', rot=45, ax=ax, legend=False, color=color[1], logy=True)
speedup_o3.plot(kind='bar', rot=45, ax=ax, legend=False, color=color[0], logy=True)

for p in ax.patches:
    ax.annotate(
        f"{p.get_height():.2f}",
        (p.get_x() + p.get_width() / 2.0, p.get_height()),
        ha='center',
        va='center',
        xytext=(0, 10),
        textcoords='offset points'
        )

ax.legend(labels=["-O3 + Distance + MST", "-O3 + Distance Function", "Compiler Flag -O3"])

plt.title('Speedup for HDBSCAN Regions and Optimizations')  # {sys.argv[5]}')
plt.tight_layout()
plt.savefig('speedup_stacked.pdf', bbox_inches='tight')
# plt.show()
