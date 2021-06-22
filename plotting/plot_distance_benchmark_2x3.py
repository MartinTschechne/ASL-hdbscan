import argparse
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

sns.set()

def main():
    """Plotting script for distance function benchmarking."""

    parser = argparse.ArgumentParser(
        add_help=True,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )

    parser.add_argument(
        '--csv_path', type=str, required=True, help='Path to csv file.')
    parser.add_argument(
        '--save_path', type=str, help='Path to output directory.')

    cfg = parser.parse_args()

    benchmark_name = cfg.csv_path.split('/')[-1].rstrip(".csv")
    measurements_df = pd.read_csv(cfg.csv_path)

    opt_names = measurements_df.NAME.unique().tolist()
    opt_names.remove("Baseline")
    opt_names.remove("Blocking Optimized")

    base_df = measurements_df[measurements_df.NAME=="Baseline"]
    base_df = base_df.pivot("NUM_POINTS","DIM","CYC")

    scale = 4
    fig, axes = plt.subplots(2,3,figsize=(4*scale,2.5*scale))
    if len(opt_names) == 1: # make subplots array 2d, even if we only have a single optimization
        axes = [axes]
    for on, ax in zip(opt_names, axes.ravel()):
        opt_df = measurements_df[measurements_df.NAME==on]
        opt_df = opt_df.pivot("NUM_POINTS","DIM","CYC")
        # sns.heatmap(opt_df,ax=axes[i][0],annot=True,fmt="d",linewidth=.5)
        # axes[i][0].set_title(f"{on} Cycles per point")
        speedup = base_df/opt_df
        sns.heatmap(speedup,ax=ax,annot=True,fmt=".2f",linewidth=.5,vmin=0.75,vmax=10.25)
        ax.set_title(f"{on} Speed Up")

    plt.tight_layout()
    if cfg.save_path:
        plt.savefig(f"{cfg.save_path}/{benchmark_name}.pdf",dpi=100)
    else:
        plt.show()

    return 0

if __name__ == '__main__':
    main()
