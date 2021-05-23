#include <hdbscan/HDBSCAN_star.h>

double** ReadInDataSet(std::string const& file_name, const char delimiter, const size_t num_points, const size_t point_dimension) {
    double** result = new double*[num_points];
    for(size_t i = 0; i < num_points; ++i) {
        result[i] = new double[point_dimension];
    }
    std::ifstream file(file_name);
    std::string line;
    size_t line_count = 0;

    while(std::getline(file, line)) {
        std::string value;
        std::stringstream stream(line);
        try {
            std::string value;
            size_t i = 0;
            while(std::getline(stream, value, delimiter)) {
                result[line_count][i] = std::stod(value);
                ++i;
            }

            if(i != point_dimension) {
                throw std::runtime_error("Line " + std::to_string(line_count) + " has a different number of attributes than the first line");
            }
        } catch(const std::exception& e) {
            std::cout << "Failed to parse line " << line_count << ". " << e.what() << std::endl;
        }

        line_count++;
    }

    return result;
}

void FreeDataset(double** dataset, size_t num_points) {
    for(size_t i = 0; i < num_points; ++i) {
        free(dataset[i]);
    }

    free(dataset);
}

Vector* ReadInConstraints(std::string const& file_name) {
    Vector* result = (Vector*)malloc(sizeof(Vector));
    vector_init(result);
    std::ifstream file(file_name);
    std::string line;
    int num_attributes = -1;
    size_t line_count = 0;

    while(std::getline(file, line)) {
        std::stringstream stream(line);

        size_t point_a;
        size_t point_b;
        std::string link_type;

        stream >> point_a;
        stream.ignore();
        stream >> point_b;
        stream.ignore();
        stream >> link_type;

        Constraint::CONSTRAINT_TYPE constraint_type =
            link_type == Constraint::MUST_LINK_TAG
            ? Constraint::CONSTRAINT_TYPE::MUST_LINK
            : Constraint::CONSTRAINT_TYPE::CANNOT_LINK;

        Constraint* constraint = (Constraint*)malloc(sizeof(Constraint));
        constraint->point_a = point_a;
        constraint->point_b = point_b;
        constraint->type = constraint_type;

        vector_push_back(result, (void*)constraint);

        line_count++;
    }

    return result;
}
