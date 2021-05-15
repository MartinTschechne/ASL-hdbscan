#include <hdbscan/HDBSCAN_star.h>

std::vector<std::vector<double>> ReadInDataSet(std::string const& file_name, const char delimiter) {
    std::vector<std::vector<double>> result;
    std::ifstream file(file_name);
    std::string line;
    int num_attributes = -1;
    size_t line_count = 0;

    while(std::getline(file, line)) {
        std::string value;
        std::vector<double> row;
        std::stringstream stream(line);
        try {
            std::string value;
            while(std::getline(stream, value, delimiter)) {
                row.push_back(std::stod(value));
            }

            if(num_attributes == -1) {
                num_attributes = row.size();
            }
            if(row.size() != num_attributes) {
                throw std::runtime_error("Line " + std::to_string(line_count) + " has a different number of attributes than the first line");
            }

            result.push_back(row);
        } catch(const std::exception& e) {
            std::cout << "Failed to parse line " << line_count << ". " << e.what() << std::endl;
        }

        line_count++;
    }

    return result;
}

std::vector<Constraint> ReadInConstraints(std::string const& file_name) {
    std::vector<Constraint> result;
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

        Constraint::CONSTRAINT_TYPE contraint_type =
            link_type == Constraint::MUST_LINK_TAG
            ? Constraint::CONSTRAINT_TYPE::MUST_LINK
            : Constraint::CONSTRAINT_TYPE::CANNOT_LINK;

        result.push_back(Constraint(point_a, point_b, contraint_type));

        line_count++;
    }

    return result;
}
