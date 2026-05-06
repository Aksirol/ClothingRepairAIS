#ifndef POSITIONREPOSITORY_H
#define POSITIONREPOSITORY_H

#include <vector>
#include <optional>
#include "Position.h"

class PositionRepository {
public:
    std::vector<Position> getAll();
    std::optional<Position> getById(int id);
    bool insert(const Position& position);
    bool update(const Position& position);
    bool deleteById(int id);
};

#endif