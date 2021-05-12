import argparse
import time
import numpy as np
import matplotlib.pyplot as plt
from sklearn.metrics import pairwise
import hdbscan

METRICS = ["euclidean", "manhattan", "infinity", "cosine", "pearson" ]

def main():
    """Visualizes HDBSCAN* clustering for different distance metrics."""
    parser = argparse.ArgumentParser(
        add_help=True,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )

    parser.add_argument(
        '--data_path', type=str, required=True, default=False, help='Path to data set.')
    parser.add_argument(
        '--save_path', type=str, help='Path to save figure.')
    parser.add_argument(
        '--min_cluster_size', type=int, default=5, help='Minimum size of clusters.')
    parser.add_argument(
        '--algorithm', type=str, default='best', help='Algorithm for clustering.',
        choices=['best', 'generic', 'prims_kdtree','prims_balltree','boruvka_kdtree','boruvka_balltree'])
    parser.add_argument(
        '--cluster_selection_method', type=str, default='eom', help='Method to select clusters from condensed tree.',
        choices=['eom', 'leaf'])
    cfg = parser.parse_args()

    data = np.loadtxt(cfg.data_path,delimiter=',')
    print(f"Data shape: {data.shape}",end='\n\n')

    if data.shape[1] > 2:
        print("Dimensionality of data points is larger than 2.",
              "Only the first two dimensions will be used to create scatter plots.",end='\n\n')

    fig, axes = plt.subplots(1,5,figsize=(20,4))

    for ax, metric in zip(np.ravel(axes), METRICS):
        print(f"Start metric: {metric}")
        if metric == "cosine":
            clusterer = hdbscan.HDBSCAN(metric='precomputed',
                algorithm = cfg.algorithm,
                cluster_selection_method = cfg.cluster_selection_method,
                min_cluster_size = cfg.min_cluster_size,
                match_reference_implementation = True,
                core_dist_n_jobs = 1)
            t0 = time.process_time()
            distance_matrix = pairwise.cosine_distances(data)
            clusterer.fit(distance_matrix)
            dt = time.process_time() - t0
        elif metric == "pearson":
            clusterer = hdbscan.HDBSCAN(metric='precomputed',
                algorithm = cfg.algorithm,
                cluster_selection_method = cfg.cluster_selection_method,
                min_cluster_size = cfg.min_cluster_size,
                match_reference_implementation = True,
                core_dist_n_jobs = 1)
            t0 = time.process_time()
            distance_matrix = 1. - np.corrcoef(data)
            clusterer.fit(distance_matrix)
            dt = time.process_time() - t0
        else:
            clusterer = hdbscan.HDBSCAN(metric=metric,
                algorithm = cfg.algorithm,
                cluster_selection_method = cfg.cluster_selection_method,
                min_cluster_size = cfg.min_cluster_size,
                match_reference_implementation = True,
                core_dist_n_jobs = 1)
            t0 = time.process_time()
            clusterer.fit(data)
            dt = time.process_time() - t0

        print(f"Execution time: {(dt*1000):.3f} ms",end='\n\n')

        # add 0.1 and clip at 1 to make noise data points visible
        alphas = np.clip(clusterer.probabilities_+0.1,a_min=0.0,a_max=1.0)
        # this will only plot the first two dimensions, regardless of the data dimension
        ax.scatter(data[:,0],data[:,1],c=clusterer.labels_, alpha = alphas)
        ax.set_title(metric)

    plt.tight_layout()
    if cfg.save_path:
        plt.savefig(cfg.save_path,dpi=100)
    else:
        plt.show()

    return 0

if __name__ == '__main__':
    main()
