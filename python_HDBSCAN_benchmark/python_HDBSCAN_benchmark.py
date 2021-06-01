import click
import hdbscan
import numpy as np
import time
from sklearn.metrics.cluster import adjusted_rand_score

class MetricType(click.ParamType):
    name = 'metric'

    def convert(self, value, param, ctx):
        if value.strip().lower() in hdbscan.dist_metrics.METRIC_MAPPING:
            return value
        else:
            self.fail('"{}" is not a valid distance metric.'.format(value),
                param, ctx)


@click.command()
@click.option(
    '-d', '--data', type=click.File(), prompt='Data file',
    help='Input data: [points x features] (CSV)')
@click.option(
    '-l', '--labels', type=click.File(), prompt='Labels file',
    help='Ground truth labels: [labels x 1] (CSV)')
@click.option(
    '-m', '--metric', type=MetricType(), default='euclidean',
    prompt='Distance metric', help='Distance metric as available in sklearn')


def python_benchmark(data, labels, metric):
    """Run and time HDBSCAN from scikit-learn-contrib on given dataset."""

    X = np.loadtxt(data, delimiter=',')
    y_true = np.loadtxt(labels, delimiter=',')
    print(
        'HDBSCAN on {0:d} points with {1:d} features, using metric "{2}".'.format( X.shape[0], X.shape[1], metric))

    clusterer = hdbscan.HDBSCAN(metric=metric)
    tic = time.perf_counter()
    y_pred = clusterer.fit_predict(X)
    toc = time.perf_counter()

    rand_score = adjusted_rand_score(y_pred, y_true)
    time_ms = (toc - tic) * 1e3

    print('Rand Index: {0:.2f}'.format(rand_score))
    print('Time taken: {0:.1f} ms'.format(time_ms))

if __name__ == '__main__':
    python_benchmark()
