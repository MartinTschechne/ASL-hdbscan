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

    base_df = measurements_df[measurements_df.NAME=="Baseline"]
    base_df = base_df.pivot("NUM_POINTS","DIM","CYC")


    fig, axes = plt.subplots(len(opt_names),2,figsize=(10,5*len(opt_names)))
    if len(opt_names) == 1: # make subplots array 2d, even if we only have a single optimization
        axes = [axes]
    fig.suptitle(f"{benchmark_name.split('_')[-1]}")
    for i, on in enumerate(opt_names):
        opt_df = measurements_df[measurements_df.NAME==on]
        opt_df = opt_df.pivot("NUM_POINTS","DIM","CYC")
        sns.heatmap(opt_df,ax=axes[i][0],annot=True,fmt="d",linewidth=.5)
        axes[i][0].set_title(f"{on} Cycles per point")
        speedup = base_df/opt_df
        sns.heatmap(speedup,ax=axes[i][1],annot=True,fmt=".2f",linewidth=.5)
        axes[i][1].set_title(f"{on} Speed Up")

    plt.tight_layout()
    if cfg.save_path:
        plt.savefig(f"{cfg.save_path}/{benchmark_name}.png",dpi=100)
    else:
        plt.show()

    return 0

if __name__ == '__main__':
    main()
