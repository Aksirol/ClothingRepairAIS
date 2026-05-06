#include "EmployeeRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

std::vector<Employee> EmployeeRepository::getAll() {
    std::vector<Employee> employees;
    QSqlQuery query("SELECT employee_id, position_id, last_name, first_name, patronymic, phone, hire_date FROM employees");

    while (query.next()) {
        Employee e;
        e.id = query.value(0).toInt();
        e.positionId = query.value(1).toInt();
        e.lastName = query.value(2).toString();
        e.firstName = query.value(3).toString();
        e.patronymic = query.value(4).toString();
        e.phone = query.value(5).toString();
        e.hireDate = QDate::fromString(query.value(6).toString(), Qt::ISODate);
        employees.push_back(e);
    }
    return employees;
}

std::optional<Employee> EmployeeRepository::getById(int id) {
    QSqlQuery query;
    query.prepare("SELECT employee_id, position_id, last_name, first_name, patronymic, phone, hire_date FROM employees WHERE employee_id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        Employee e;
        e.id = query.value(0).toInt();
        e.positionId = query.value(1).toInt();
        e.lastName = query.value(2).toString();
        e.firstName = query.value(3).toString();
        e.patronymic = query.value(4).toString();
        e.phone = query.value(5).toString();
        e.hireDate = QDate::fromString(query.value(6).toString(), Qt::ISODate);
        return e;
    }
    return std::nullopt;
}

bool EmployeeRepository::insert(const Employee& emp) {
    QSqlQuery query;
    query.prepare("INSERT INTO employees (position_id, last_name, first_name, patronymic, phone, hire_date) "
                  "VALUES (:posId, :lName, :fName, :patr, :phone, :hDate)");
    query.bindValue(":posId", emp.positionId);
    query.bindValue(":lName", emp.lastName);
    query.bindValue(":fName", emp.firstName);
    query.bindValue(":patr", emp.patronymic);
    query.bindValue(":phone", emp.phone);
    query.bindValue(":hDate", emp.hireDate.toString(Qt::ISODate));

    if(!query.exec()) {
        qDebug() << "Помилка додавання працівника:" << query.lastError().text();
        return false;
    }
    return true;
}

bool EmployeeRepository::update(const Employee& emp) {
    QSqlQuery query;
    query.prepare("UPDATE employees SET position_id = :posId, last_name = :lName, first_name = :fName, "
                  "patronymic = :patr, phone = :phone, hire_date = :hDate WHERE employee_id = :id");
    query.bindValue(":posId", emp.positionId);
    query.bindValue(":lName", emp.lastName);
    query.bindValue(":fName", emp.firstName);
    query.bindValue(":patr", emp.patronymic);
    query.bindValue(":phone", emp.phone);
    query.bindValue(":hDate", emp.hireDate.toString(Qt::ISODate));
    query.bindValue(":id", emp.id);
    return query.exec();
}

bool EmployeeRepository::deleteById(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM employees WHERE employee_id = :id");
    query.bindValue(":id", id);
    return query.exec();
}