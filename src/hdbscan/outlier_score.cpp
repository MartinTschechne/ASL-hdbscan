#include <hdbscan/HDBSCAN_star.h>
#include <cstdlib>

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

int CompareOutlierScores(const void* a, const void* b) {
    return CompareTo(*(OutlierScore*)a, *(OutlierScore*)b);
}

void CalculateOutlierScores(
        const Vector* const clusters, double* point_noise_levels, size_t point_noise_levels_length,
        size_t* point_last_clusters, const double* core_distances,
        const std::string& outlier_scores_outputFile, const char delimiter,
        bool infinite_stability, vector* result) {
    
    vector_clear(result);

    //Iterate through each point, calculating its outlier score:
    for(size_t i = 0; i < point_noise_levels_length; ++i) {
        double eps_max = ((Cluster*)clusters->elements[point_last_clusters[i]])->propagated_lowest_child_death_level;
        double eps = point_noise_levels[i];

        double score = 0.0;
        if(eps != 0) {
            score = 1 - (eps_max / eps);
        }

        OutlierScore* new_score = (OutlierScore*)malloc(sizeof(OutlierScore));
        new_score->core_distance = core_distances[i];
        new_score->score = score;
        new_score->id = i;
        vector_push_back(result, (void*)new_score);
    }

    //Sort the outlier scores:
    qsort(result->elements, result->size, sizeof(void*), CompareOutlierScores);

    //Output the outlier scores:
    std::ofstream writer(outlier_scores_outputFile);
    if(infinite_stability) {
        writer << warning_message << "\n";
    }

    for(size_t i = 0; i < result->size; ++i) {
        OutlierScore* score = (OutlierScore*)result->elements[i];
        writer << score->score << delimiter << score->id << "\n";
    }
}
