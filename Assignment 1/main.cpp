#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "assignment1.h"

int main() {
    std::ifstream file("TSPA.csv");  // Open your CSV file
    if (!file.is_open()) {
        std::cerr << "Error: could not open file\n";
        return 1;
    }

    std::vector<PointData> data;
    std::string line;
    // New variable to auto-generate the ID, starting at 0
    int current_id = 0; 

    // Skip the first line (if it's a header, common in CSV)
    // std::getline(file, line); 

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;

        int temp_x, temp_y, temp_cost;
        int count = 0;
        bool error = false;

        // Loop to extract the three values (x, y, cost) separated by ';'
        while (std::getline(ss, value, ';')) {  // Use ';' as separator
            try {
                int num = std::stoi(value);
                if (count == 0) {
                    temp_x = num;
                } else if (count == 1) {
                    temp_y = num;
                } else if (count == 2) {
                    temp_cost = num;
                }
                count++;
            } catch (...) {
                std::cerr << "Invalid number: " << value << " in line: " << line << "\n";
                error = true;
                break; // Stop processing this row on error
            }
        }

        // Check if exactly 3 values were read and no error occurred
        if (!error && count == 3) {
            // Create a PointData struct and populate it, assigning the current ID
            PointData point = {current_id, temp_x, temp_y, temp_cost};
            
            // Add the struct to the vector
            data.push_back(point);
            
            // Increment the ID counter for the next successful row
            current_id++; 
        } else if (!error && count != 0) {
            std::cerr << "Warning: Row skipped due to incorrect number of columns (expected 3, got " << count << "): " << line << "\n";
        }
    }

    file.close();

    // ✅ Print out the loaded data, including the new ID
    std::cout << "Loaded " << data.size() << " rows with auto-generated IDs:\n";
    std::cout << "ID\tX\tY\tCost\n";
    std::cout << "--------------------------------\n";
    for (const auto& point : data) {
        std::cout << point.id << "\t" << point.x << "\t" << point.y << "\t" << point.cost << "\n";
    }

    // Generate a random solution
    std::vector<int> random_solution = generate_random_solution(data);

    // Print the random solution
    std::cout << "\nGenerated Random Solution (node IDs):\n";
    for (int id : random_solution) {
        std::cout << id << " ";
    }
    std::cout << std::endl;

    return 0;
}
