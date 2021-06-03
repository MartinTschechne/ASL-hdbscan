import argparse
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

sns.set()

def main():
    """Plotting script for final comparison with reference implementation."""

    parser = argparse.ArgumentParser(
        add_help=True,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )

    parser.add_argument(
        '--python_reference', type=str, required=True, help='Path python reference output.')
    parser.add_argument(
        '--ours', type=str, required=True, help='Path to our best benchmark measurements_runner_*.txt output.')
    parser.add_argument(
        '--cpu_freq', type=float, required=True, help='CPU frequency in GHz, e.g. 2.8')

    cfg = parser.parse_args()

    # read reference measurement output
    with open(cfg.python_reference,'r') as f:
        ref = f.read()
    f.close()

    ref = float(ref.split('\n')[-2].split(' ')[-2])/1000 # in seconds

    # read our fastest measurement
    df = pd.read_csv(cfg.ours, skiprows=7, index_col='Region')
    ours = int(df.loc['total'])/(cfg.cpu_freq*10**9) # convert to secods

    # optional for report: include java reference
    res = pd.DataFrame(data=[ref, ours],index=['python hdbscan','Our best'],columns=['time [sec]'])

    fig, ax = plt.subplots(1,figsize=(5,5),dpi=100)
    res.plot(kind='bar', ax=ax, rot=0, legend=False)
    for p in ax.patches:
        ax.annotate(f"{p.get_height():.2f}",(p.get_x()+p.get_width()/2., p.get_height()),ha='center',va='center',xytext=(0, 10),textcoords='offset points')
    plt.title(f"Speedup = {ref/ours :.2f} @ {cfg.cpu_freq} GHz")
    plt.ylabel("Execution time [sec]")
    plt.tight_layout()
    plt.savefig("comparison.png",bbox_inches='tight')
    return 0

if __name__ == '__main__':
    main()
