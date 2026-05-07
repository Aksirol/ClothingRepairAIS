-- Увімкнення підтримки зовнішніх ключів
PRAGMA foreign_keys = ON;

-- 1. Службова таблиця для версіонування
CREATE TABLE IF NOT EXISTS db_version (
                                          version INTEGER PRIMARY KEY,
                                          applied_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 2. Незалежні довідники (не мають зовнішніх ключів)
CREATE TABLE IF NOT EXISTS positions (
                                         position_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                         position_name TEXT NOT NULL,
                                         hourly_rate REAL NOT NULL
);

CREATE TABLE IF NOT EXISTS clients (
                                       client_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                       last_name TEXT NOT NULL,
                                       first_name TEXT NOT NULL,
                                       patronymic TEXT,
                                       phone TEXT NOT NULL,
                                       email TEXT,
                                       address TEXT,
                                       registration_date TEXT DEFAULT (date('now'))
    );

CREATE TABLE IF NOT EXISTS order_statuses (
                                              status_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                              status_name TEXT NOT NULL,
                                              color_code TEXT,
                                              is_system INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS repair_categories (
                                                 category_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                                 category_name TEXT NOT NULL
);

-- 3. Довідники 2-го рівня (залежать від інших довідників)
CREATE TABLE IF NOT EXISTS employees (
                                         employee_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                         position_id INTEGER NOT NULL,
                                         last_name TEXT NOT NULL,
                                         first_name TEXT NOT NULL,
                                         patronymic TEXT,
                                         phone TEXT NOT NULL,
                                         hire_date TEXT NOT NULL,
                                         FOREIGN KEY (position_id) REFERENCES positions (position_id) ON DELETE RESTRICT
    );

CREATE TABLE IF NOT EXISTS repair_services (
                                               service_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                               category_id INTEGER NOT NULL,
                                               service_name TEXT NOT NULL,
                                               description TEXT,
                                               base_price REAL NOT NULL,
                                               estimated_days INTEGER NOT NULL,
                                               FOREIGN KEY (category_id) REFERENCES repair_categories (category_id) ON DELETE RESTRICT
    );

-- 4. Основна бізнес-логіка (замовлення та їхні позиції)
CREATE TABLE IF NOT EXISTS orders (
                                      order_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                      client_id INTEGER NOT NULL,
                                      employee_id INTEGER NOT NULL,
                                      status_id INTEGER NOT NULL,
                                      received_date TEXT NOT NULL,
                                      required_date TEXT NOT NULL,
                                      completed_date TEXT,
                                      deposit_amount REAL DEFAULT 0.0,
                                      payment_status TEXT NOT NULL
                                          CHECK(payment_status IN ('Неоплачено', 'Частково', 'Оплачено'))
                                          DEFAULT 'Неоплачено',
                                      notes TEXT,
                                      FOREIGN KEY (client_id) REFERENCES clients (client_id) ON DELETE RESTRICT,
                                      FOREIGN KEY (employee_id) REFERENCES employees (employee_id) ON DELETE RESTRICT,
                                      FOREIGN KEY (status_id) REFERENCES order_statuses (status_id) ON DELETE RESTRICT
);

CREATE TABLE IF NOT EXISTS order_items (
                                           item_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                           order_id INTEGER NOT NULL,
                                           service_id INTEGER NOT NULL,
                                           quantity INTEGER NOT NULL DEFAULT 1,
                                           unit_price REAL NOT NULL,
                                           clothing_description TEXT,
                                           item_notes TEXT,
                                           FOREIGN KEY (order_id) REFERENCES orders (order_id) ON DELETE CASCADE,
    FOREIGN KEY (service_id) REFERENCES repair_services (service_id) ON DELETE RESTRICT
    );

-- ==========================================
-- Початкове наповнення даними (Seed Data)
-- ==========================================

-- Встановлюємо початкову версію бази
INSERT INTO db_version (version) VALUES (1);

-- Базові статуси (Життєвий цикл: Прийнято -> В роботі -> Готово -> Видано / Скасовано)
INSERT INTO order_statuses (status_name, color_code, is_system) VALUES
                                                                    ('Прийнято', '#FFA500', 1),
                                                                    ('В роботі', '#1E90FF', 1),
                                                                    ('Готово', '#32CD32', 1),
                                                                    ('Видано', '#808080', 1),
                                                                    ('Скасовано', '#FF0000', 1);

-- Базові посади
INSERT INTO positions (position_name, hourly_rate) VALUES
                                                       ('Майстер з ремонту', 150.0),
                                                       ('Старший майстер', 200.0),
                                                       ('Адміністратор', 120.0);

-- Базові категорії послуг
INSERT INTO repair_categories (category_name) VALUES
                                                  ('Дрібний ремонт'),
                                                  ('Заміна блискавки'),
                                                  ('Підгонка по фігурі'),
                                                  ('Ремонт шкіряних виробів');