#include "data_loader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

// Helper function to load data from a CSV file
bool load_data(const std::string& filename, std::vector<PointData>& data) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: could not open file " << filename << "\n";
        return false;
    }

    data.clear();
    std::string line;
    int current_id = 0;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;
        std::vector<int> values;
        bool error = false;

        while (std::getline(ss, value, ';')) {
            try {
                values.push_back(std::stoi(value));
            } catch (...) {
                error = true;
                break;
            }
        }

        if (!error && values.size() == 3) {
            data.push_back({current_id, values[0], values[1], values[2]});
            current_id++;
        }
    }

    file.close();

    if (data.empty()) {
        std::cerr << "Error: No data loaded from " << filename << std::endl;
        return false;
    }
    return true;
}
