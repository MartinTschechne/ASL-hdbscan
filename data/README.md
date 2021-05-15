### Usage of `make_data.py`
* Create simple 2D data set with two moons and two blobs:
```
python make_data.py --out_name example_data_set --n_points 100
```

* Create higher dimensional data set:
```
python make_data.py --out_name data_set --n_features 10 --n_points 1000
```

* Create data set with higher number of clusters:
```
python make_data.py --out_name data_set --n_features 10 --n_points 1000 --n_centers 10
```

* Create data set with more noise:
```
python make_data.py --out_name data_set --n_features 10 --n_points 1000 --noise_level 1.5
```

Command line arguments:
* `--out_name`: Name of the `csv`-file or output path, e.g. `--out_name test` or `--out_name ../test.csv`.
* `--n_points`: Number of points in the dataset.Equally distributed among clusters. Default: 100
* `--n_features`: Dimensionality of the points. Default: 2
* `--n_centers`: Number of cluster centers. Default: 3
* `--noise_level`: Standard deviation of each cluster. Default 1.0
* `--seed`: Random seed for reproducibility. Default: 42

To quickly inspect the points and cluster labels produced by the reference implementation run
```
python ../hdbscan_reference.py --data_path test.csv
```
Works also for higher dimensional data, but will only use the first two dimensions of the points fo scatter plots!
