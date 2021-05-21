#include <hdbscan/HDBSCAN_star.h>

int CompareTo(const OutlierScore& a, const OutlierScore& b) {
    if(a.score > b.score) {
        return 1;
    }
    if(a.score < b.score) {
        return -1;
    }

    if(a.core_distance > b.core_distance) {
        return 1;
    }
    if(a.core_distance < b.core_distance) {
        return -1;
    }

    return a.id - b.id;
}

void CalculateOutlierScores(
        const std::vector<Cluster*>& clusters, double* point_noise_levels, size_t point_noise_levels_length,
        size_t* point_last_clusters, const double* core_distances,
        const std::string& outlier_scores_outputFile, const char delimiter,
        bool infinite_stability, std::vector<OutlierScore>& result) {

    result.clear();
    result.reserve(point_noise_levels_length);

    //Iterate through each point, calculating its outlier score:
    for(size_t i = 0; i < point_noise_levels_length; ++i) {
        double eps_max = clusters[point_last_clusters[i]]->propagated_lowest_child_death_level;
        double eps = point_noise_levels[i];

        double score = 0.0;
        if(eps != 0) {
            score = 1 - (eps_max / eps);
        }

        result.push_back(OutlierScore(score, core_distances[i], i));
    }

    //Sort the outlier scores:
    std::sort(result.begin(), result.end(), [](const OutlierScore& a, const OutlierScore& b)->bool { return a.score < b.score; });

    //Output the outlier scores:
    std::ofstream writer(outlier_scores_outputFile);
    if(infinite_stability) {
        writer << warning_message << "\n";
    }

    for(const OutlierScore& score : result) {
        writer << score.score << delimiter << score.id << "\n";
    }
}
