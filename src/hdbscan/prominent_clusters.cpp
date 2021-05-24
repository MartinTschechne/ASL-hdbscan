#include <hdbscan/HDBSCAN_star.h>
#include <common/map.h>

void FindProminentClusters(const Vector* const clusters,
        const std::string& hierarchy_file, const std::string& flat_output_file,
        const char delimiter, size_t num_points, bool infinite_stability,
        vector* result) {

    //Take the list of propagated clusters from the root cluster:
    const Vector* solution = ((Cluster*)clusters->elements[1])->propagated_descendants;

    std::ifstream reader(hierarchy_file);
    size_t* flat_partioning = (size_t*)calloc(num_points, sizeof(size_t));

    //Store all the file offsets at which to find the birth points for the flat clustering:
    Map* significant_file_offsets = Map_create();

    for(size_t i = 0; i < solution->size; ++i) {
        const Cluster* cluster = (Cluster*)vector_get(solution, i);
        vector* cluster_list = nullptr;
        size_t index = Map_find(significant_file_offsets, cluster->file_offset);
        if(index == Map_end(significant_file_offsets)) {
            cluster_list = vector_create();
            Map_insert(significant_file_offsets, cluster->file_offset, (void*)cluster_list);
        } else {
            cluster_list = (vector*)Map_get_idx(significant_file_offsets, index);
        }

        size_t* label = (size_t*)malloc(sizeof(size_t));
        *label = cluster->label;
        vector_push_back(cluster_list, (void*)label);
    }

    //Go through the hierarchy file, setting labels for the flat clustering:
    for(size_t it = Map_begin(significant_file_offsets); it < Map_end(significant_file_offsets); it = Map_next(significant_file_offsets, it)) {
        vector* cluster_list = (vector*)Map_get_idx(significant_file_offsets, it);
        size_t offset = Map_key_from_idx(significant_file_offsets, it);

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
            
            bool contains = false;
            for(size_t i = 0; i < cluster_list->size; ++i) {
                if(*((size_t*)cluster_list->elements[i]) == label) {
                    contains = true;
                    break;
                }
            }

            if(contains) {
                flat_partioning[i-1] = label;
            }

            ++i;
        }
    }

    Map_clear(significant_file_offsets);

    //Output the flat clustering result:
    std::ofstream writer(flat_output_file);
    if(infinite_stability) {
        writer << warning_message << "\n";
    }

    for(size_t i = 0; i < num_points-1; ++i) {
        writer << flat_partioning[i] << delimiter;
    }

    writer << flat_partioning[num_points-1] << "\n";
    free(flat_partioning);
}