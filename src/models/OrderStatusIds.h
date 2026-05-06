#ifndef ORDERSTATUSIDS_H
#define ORDERSTATUSIDS_H

namespace StatusId {
    constexpr int Accepted   = 1; // Прийнято
    constexpr int InProgress = 2; // В роботі
    constexpr int Ready      = 3; // Готово
    constexpr int Issued     = 4; // Видано
    constexpr int Cancelled  = 5; // Скасовано
}

#endif