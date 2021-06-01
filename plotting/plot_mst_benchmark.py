from absl import flags, app
import os 
import subprocess
import shutil
from tqdm import tqdm
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

sns.set()
sns.set_style('darkgrid')

output_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "mst_timings")
root_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../build")
timings_dir = os.path.join(root_dir, "mst_timings")

common_args = "-hierarchy_file=output/hierarchy.csv -tree_file=output/tree.csv -visualization_file=output/results.vis -partition_file=output/partition.csv -outlier_score_file=output/outlier_scores.csv"

flags.DEFINE_bool("disable_nobitset", False, "Whether or not to disable the no bitset functions, which consume about 2x memory")
flags.DEFINE_bool("recompute", True, "Whether or not to recompute the timings")
flags.DEFINE_integer("num_runs", 1, "How often to compute the timings")
flags.DEFINE_string("points_file", None, "Path of the csv file of the points")
flags.DEFINE_integer("num_points", None, "Number of points in the file")
flags.DEFINE_integer("num_dimension", None, "Number of dimensions")
flags.DEFINE_string("output_name", "results", "Name of the output files")
flags.DEFINE_string("title_heatmap", None, "Title to add to the heatmap")
flags.DEFINE_string("title_bar", None, "Title to add to the bar plot")

FLAGS = flags.FLAGS

files_base = ["base", "bitset_unroll", "bitset_nocalc", "bitset_nocalc_unroll4", "bitset_nocalc_avx", "bitset_nocalc_avx_unroll2", "bitset_nocalc_avx_unroll4", "nobitset", "nobitset_nocalc", "nobitset_nocalc_avx"]

def compile():
    os.makedirs(root_dir, exist_ok=True)
    subprocess.call("cmake ..".split(), cwd=root_dir)
    subprocess.call("make -j14".split(), cwd=root_dir)

def parse_output(file):
    with open(file, 'r') as f:
        lines = f.readlines()
    
    assert(len(lines) in [3,4])
    if len(lines) == 3:
        return int(lines[1].split(",")[1])
    if len(lines) == 4:
        return int(lines[1].split(",")[1]) + int(lines[2].split(",")[1])

def perform_run():
    assert(FLAGS.points_file is not None)
    assert(FLAGS.num_dimension is not None)
    assert(FLAGS.num_points is not None)

    args = f"-points_file {FLAGS.points_file} -num_points {FLAGS.num_points} -num_dimension {FLAGS.num_dimension} -{'' if FLAGS.disable_nobitset else 'no'}disable_nobitset {common_args}"
    cmd = f"./mst_benchmark {args}"
    subprocess.run(cmd.split(), cwd=root_dir)

    files = [f"{f}.csv" for f in files_base]
    if FLAGS.disable_nobitset:
        files = files[:-3]
    
    return [parse_output(os.path.join(timings_dir, f)) for f in files]

def recompute(result_file):
    compile()
    shutil.rmtree(timings_dir, ignore_errors=True)

    timings = [perform_run() for i in tqdm(range(FLAGS.num_runs))]

    header = files_base
    if FLAGS.disable_nobitset:
        header = header[:-3]
    
    avg = np.average(np.array(timings), 0)
    avg = [str(a) for a in avg]

    with open(result_file, 'w') as f:
        f.write(f"#RUN,{','.join(header)}\n")
        
        def _write_row(j, t):
            tp = [str(l) for l in t]
            return f"{str(j)},{','.join(tp)}"
        
        rows = [_write_row(j, t) for j,t in enumerate(timings)]
        f.write("\n".join(rows))
        f.write("\n")
        f.write(f"AVG,{','.join(avg)}")

def plot(result_csv):
    measurements_df = pd.read_csv(result_csv, index_col=0)
    fig, ax = plt.subplots(1, figsize=(10,10), dpi=100)
    sns.set_style('darkgrid')

    measurements_df.loc['AVG'].plot(kind='bar', ax=ax)
    for p in ax.patches:
        ax.annotate(f"{p.get_height():.2E}",(p.get_x()+p.get_width()/2., p.get_height()),ha='center',va='center',xytext=(0, 10),textcoords='offset points')
    plt.ylabel("# Cycles")
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.title(FLAGS.title_bar)
    plt.savefig(os.path.join(output_dir, f"{FLAGS.output_name}_bar.png"))

    avg = measurements_df.loc['AVG'].values
    avg = np.reshape(avg, (avg.shape[0], 1))
    mat = np.repeat(avg, avg.shape[0], axis=1)
    mat /= np.transpose(mat)

    opt_names = files_base
    if FLAGS.disable_nobitset:
        opt_names = opt_names[:-3]
    fig, axes = plt.subplots(1,figsize=(12,12), dpi=100)
    sns.heatmap(mat,annot=True,fmt=".2f",linewidth=.5, ax=axes, xticklabels=opt_names, yticklabels=opt_names)
    plt.yticks(rotation=0)
    plt.xticks(rotation=45, ha='right')
    plt.tight_layout()
    plt.title(FLAGS.title_heatmap)
    plt.subplots_adjust(top=0.95)
    plt.savefig(os.path.join(output_dir, f"{FLAGS.output_name}_heatmap.png"))  

def main(argv):
    result_file = os.path.join(output_dir, f'{FLAGS.output_name}.csv')
    os.makedirs(output_dir, exist_ok=True)
    if FLAGS.recompute:
        recompute(result_file)
    plot(result_file)

if __name__ == "__main__":
    app.run(main)
