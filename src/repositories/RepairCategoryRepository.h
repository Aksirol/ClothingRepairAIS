#ifndef REPAIRCATEGORYREPOSITORY_H
#define REPAIRCATEGORYREPOSITORY_H

#include <vector>
#include <optional>
#include "RepairCategory.h"

class RepairCategoryRepository {
public:
    std::vector<RepairCategory> getAll();
    std::optional<RepairCategory> getById(int id);
    bool insert(const RepairCategory& category);
    bool update(const RepairCategory& category);
    bool deleteById(int id);
};

#endif