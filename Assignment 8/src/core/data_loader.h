#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include <vector>
#include <string>
#include "point_data.h"

bool load_data(const std::string& filename, std::vector<PointData>& data);

#endif // DATA_LOADER_H
