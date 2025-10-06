#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

int main() {
    std::ifstream file("TSPA.csv");  // Open your CSV file
    if (!file.is_open()) {
        std::cerr << "Error: could not open file\n";
        return 1;
    }

    std::vector<std::vector<int>> data;
    std::string line;

    while (std::getline(file, line)) {
        std::vector<int> row;
        std::stringstream ss(line);
        std::string value;

        while (std::getline(ss, value, ';')) {  // Use ';' as separator
            try {
                row.push_back(std::stoi(value));
            } catch (...) {
                std::cerr << "Invalid number: " << value << "\n";
            }
        }

        if (!row.empty())
            data.push_back(row);
    }

    file.close();

    // ✅ Print out the loaded data
    for (const auto& row : data) {
        for (const auto& num : row)
            std::cout << num << "\t";
        std::cout << "\n";
    }

    return 0;
}
