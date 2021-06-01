### Usage of `python_HDBSCAN_benchmark.py`

This script runs and times [HDBSCAN from scikit-learn-contrib](https://github.com/scikit-learn-contrib/hdbscan),
the most widely used implementation of [HDBSCAN](http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=8215642).

* Basic usage:
```
python python_HDBSCAN_benchmark.py --data data_set --labels label_set --metric distance_metric
```
where `data_set` and `label_set` are CSV files with one point per row and `distance_metric` is one of sklearn's metrics defaulting to `euclidean`.

* Accuracy score:
The similarity between the given ground truth and the obtained clustering is measured using the [Rand Index](https://scikit-learn.org/stable/modules/generated/sklearn.metrics.adjusted_rand_score.html), where 0.0 indicates a random labeling and 1.0 indicates an identical clustering (up to a permutation).

* Timing:
Time is measured in milliseconds based on `time.perf_counter` which in turn measures the CPU counter which is monotonically increased at a frequency related to the CPU's hardware clock.
