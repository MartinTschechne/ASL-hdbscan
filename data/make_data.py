import argparse
import numpy as np
import sklearn.datasets as data

def main():
    """Creates data set of four clusters, 2 blobs and 2 moons."""

    parser = argparse.ArgumentParser(
        add_help=True,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )

    parser.add_argument(
        '--out_name', type=str, required=True, help='Name of dataset without .csv ending.')
    parser.add_argument(
        '--n_points', type=int, default = 100, help='Number of total datapoints.')
    parser.add_argument(
        '--noise_level', type=float, default = 1.0, help='Level of noise.')
    parser.add_argument(
        '--seed', type=int, default = 42, help='Random seed.')

    cfg = parser.parse_args()

    moons, _ = data.make_moons(n_samples=int(cfg.n_points/2),
        noise=0.05*cfg.noise_level, random_state=cfg.seed)

    blobs, _ = data.make_blobs(n_samples=int(cfg.n_points/2),
        centers=[(-0.75,2.25), (1.0, 2.0)], cluster_std=0.25*cfg.noise_level,
        random_state=cfg.seed)

    data_set = np.vstack([moons, blobs])

    if cfg.out_name[-4:] == ".csv":
        save_path = cfg.out_name
    else:
        save_path = cfg.out_name + ".csv"

    np.savetxt(save_path ,data_set, delimiter=',',fmt="%.8f")
    return 0

if __name__ == '__main__':
    main()
