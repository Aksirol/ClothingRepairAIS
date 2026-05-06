#ifndef EMPLOYEEREPOSITORY_H
#define EMPLOYEEREPOSITORY_H

#include <vector>
#include <optional>
#include "Employee.h"

class EmployeeRepository {
public:
    std::vector<Employee> getAll();
    std::optional<Employee> getById(int id);
    bool insert(const Employee& emp);
    bool update(const Employee& emp);
    bool deleteById(int id);
};

#endif