import argparse
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import linear_sum_assignment
from sklearn.metrics import confusion_matrix, ConfusionMatrixDisplay

def main():
    """Plot clustering results of Java reference and ours."""
    parser = argparse.ArgumentParser(
        add_help=True,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )

    parser.add_argument(
        '--data_path', type=str, required=True, help='Path to data set.')
    parser.add_argument(
        '--java_reference', type=str, required=True, help='Path to partition.csv of Java reference implementation.')
    parser.add_argument(
        '--c_partition', type=str, required=True, help='Path to partition.csv of C implementation.')
    parser.add_argument(
        '--save_path', type=str, help='Path to save figure.')
    cfg = parser.parse_args()

    data = np.loadtxt(cfg.data_path, delimiter=',')
    java = np.loadtxt(cfg.java_reference, delimiter=',')
    ours = np.loadtxt(cfg.c_partition, delimiter=',')

    conf_mat = confusion_matrix(java, ours, normalize='true')
    # take care of permutation invariance of labels
    # find permutation of labels to optimize confusion matrix
    _, col_ind = linear_sum_assignment(-conf_mat)
    conf_mat = conf_mat[:,col_ind]

    if data.shape[1] > 2:
        print("Dimensionality of data points is larger than 2.",
              "Only the first two dimensions will be used to create scatter plots.",end='\n\n')

    fig, axes = plt.subplots(1,3,figsize=(15,5))

    axes[0].scatter(data[:,0], data[:,1], c=java)
    axes[0].set_title("Java reference")

    axes[1].scatter(data[:,0], data[:,1], c=ours)
    axes[1].set_title("Ours")

    disp = ConfusionMatrixDisplay(conf_mat)
    disp.plot(ax=axes[2])
    axes[2].set_title("Confusion matrix")

    plt.tight_layout()
    if cfg.save_path:
        plt.savefig(cfg.save_path,dpi=100)
    else:
        plt.show()

    return 0

if __name__ == '__main__':
    main()
