#ifndef REPAIRSERVICEREPOSITORY_H
#define REPAIRSERVICEREPOSITORY_H

#include <vector>
#include <optional>
#include "RepairService.h"

class RepairServiceRepository {
public:
    std::vector<RepairService> getAll();
    std::optional<RepairService> getById(int id);
    bool insert(const RepairService& service);
    bool update(const RepairService& service);
    bool deleteById(int id);
};

#endif