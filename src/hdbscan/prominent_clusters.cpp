#include <hdbscan/HDBSCAN_star.h>

void FindProminentClusters(const std::vector<Cluster*>& clusters,
        const std::string& hierarchy_file, const std::string& flat_output_file,
        const char delimiter, size_t num_points, bool infinite_stability,
        std::vector<size_t>& result) {

    //Take the list of propagated clusters from the root cluster:
    const Vector* solution = clusters[1]->propagated_descendants;

    std::ifstream reader(hierarchy_file);
    size_t* flat_partioning{new size_t[num_points]{0}};

    //Store all the file offsets at which to find the birth points for the flat clustering:
    std::map<size_t, std::vector<size_t>*> significant_file_offsets;
    for(size_t i = 0; i < solution->size; ++i) {
        const Cluster* cluster = (Cluster*)vector_get(solution, i);
        std::vector<size_t>* cluster_list = nullptr;
        auto entry = significant_file_offsets.find(cluster->file_offset);
        if(entry == significant_file_offsets.end()) {
            cluster_list = new std::vector<size_t>;
            significant_file_offsets.insert({cluster->file_offset, cluster_list});
        } else {
            cluster_list = entry->second;
        }

        cluster_list->push_back(cluster->label);
    }

    //Go through the hierarchy file, setting labels for the flat clustering:
    for(auto it = significant_file_offsets.begin(); it != significant_file_offsets.end(); ) {
        std::vector<size_t>* cluster_list = it->second;
        size_t offset = it->first;

        reader.seekg(offset, std::ios::beg);
        std::string line;
        std::getline(reader, line);

        std::stringstream stream(line);
        std::string part;
        size_t i = 0;
        while(std::getline(stream, part, delimiter)) {
            if(i == 0) {
                ++i;
                continue;
            }
            size_t label = std::stoi(part);
            if(std::find(cluster_list->begin(), cluster_list->end(), label) != cluster_list->end()) {
                flat_partioning[i-1] = label;
            }

            ++i;
        }
        it = significant_file_offsets.erase(it);
    }

    //Output the flat clustering result:
    std::ofstream writer(flat_output_file);
    if(infinite_stability) {
        writer << warning_message << "\n";
    }

    for(size_t i = 0; i < num_points-1; ++i) {
        if(std::to_string(flat_partioning[i]).size() > 1) {
            size_t v = flat_partioning[i];
            std::cout << flat_partioning[i] << std::endl;
        }
        writer << flat_partioning[i] << delimiter;
    }

    writer << flat_partioning[num_points-1] << "\n";
    delete[] flat_partioning;
}