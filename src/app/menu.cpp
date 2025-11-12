#include "app/menu.hpp"
#include "ds/hash_table.hpp"
#include "ds/key_utils.hpp"
#include "ds/exporter.hpp"
#include "ds/hash_func.hpp"

#include <filesystem>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <random>

// заряжаем рандомные ключи нужного формата
std::string random_key() {
    static const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> d_num(0, 9);
    static std::uniform_int_distribution<int> d_let(0, 25);

    std::string s;
    s += letters[d_let(rng)];
    for (int i = 0; i < 4; ++i)
        s += char('0' + d_num(rng));
    s += letters[d_let(rng)];
    return s;
}

void analyze_hash_distribution(size_t N, size_t M) {
    std::vector<int> buckets(M, 0);

    for (size_t i = 0; i < N; ++i) {
        std::string key = random_key();
        size_t h = hash_key36(key) % M;
        buckets[h]++;
    }

    // гарантируем, что есть ./data рядом с рабочей директорией
    std::filesystem::path out_dir = std::filesystem::current_path() / "data";
    std::error_code ec;
    std::filesystem::create_directories(out_dir, ec); // не страшно, если уже существует

    // абсолютный путь к файлу и явная проверка открытия
    std::filesystem::path out_file = out_dir / "hash_distribution.csv";
    std::ofstream out(out_file.string(), std::ios::trunc);
    if (!out) {
        std::cerr << "Не удалось открыть файл для записи: " << out_file << "\n";
        std::cerr << "Текущая рабочая папка: " << std::filesystem::current_path() << "\n";
        return;
    }

    out << "segment;count\n";
    for (size_t i = 0; i < M; ++i) {
        if (buckets[i] > 0)
            out << i << ';' << buckets[i] << '\n';
    }
    out.close();

    std::cout << "Экспортировано " << N << " ключей в файл: " << out_file << "\n";
}

static void printMenu() {
    std::cout << "\n==== Лабораторная работа 2: Хеш-таблица (квадратичное опробование) ====\n";
    std::cout << "1) Создать таблицу (новая ёмкость)\n";
    std::cout << "2) Добавить элемент (key,value)\n";
    std::cout << "3) Найти по ключу\n";
    std::cout << "4) Удалить по ключу\n";
    std::cout << "5) Просмотреть сегмент по индексу\n";
    std::cout << "6) Показать размер, ёмкость и load factor\n";
    std::cout << "7) Выгрузить CSV (data/slots.csv, probes.csv, clusters.csv)\n";
    std::cout << "0) Выход\n";
    std::cout << ">> ";
}

void runMenu() {
    HashTable table(101);
    while (true) {
        printMenu();
        int cmd = -1;
        if (!(std::cin >> cmd)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        if (cmd == 0) {
            std::cout << "Пока!\n";
            break;
        }
        else if (cmd == 1) { // recreate
            size_t m;
            std::cout << "Введите желаемую ёмкость (M): ";
            std::cin >> m;
            if (m < 5) m = 5;
            table.reinit(m);
            std::cout << "Создана таблица с capacity=" << table.capacity() << "\n";
        }
        else if (cmd == 2) { // insert
            std::string key, value;
            std::cout << "Введите ключ (формат БццццБ): ";
            std::cin >> key;
            key = normalizeKey(key);
            if (!isValidKey(key)) {
                std::cout << "Ошибка формата ключа. Пример: A1234Z\n";
                continue;
            }
            std::cout << "Введите значение (одно слово): ";
            std::cin >> value;

            int probes = 0;
            bool ok = table.insert(key, value, &probes);
            if (ok) {
                std::cout << "ОК: вставлено, проб при вставке = " << probes << "\n";
            } else {
                std::cout << "Не удалось вставить (возможно, дубликат или переполнение)\n";
            }
        }
        else if (cmd == 3) { // find
            std::string key;
            std::cout << "Введите ключ: ";
            std::cin >> key;
            key = normalizeKey(key);
            int probes = 0;
            std::string val;
            if (table.find(key, &val, &probes)) {
                std::cout << "Найдено: value=" << val << ", пробы=" << probes << "\n";
            } else {
                std::cout << "Не найдено. Пробы=" << probes << "\n";
            }
        }
        else if (cmd == 4) { // erase
            std::string key;
            std::cout << "Введите ключ: ";
            std::cin >> key;
            key = normalizeKey(key);
            if (!isValidKey(key)) {
                std::cout << "Ошибка формата ключа\n";
                continue;
            }
            if (table.erase(key)) {
                std::cout << "Удалено (помечено Deleted).\n";

                // Тестовая фича - собираем тех, кто мог коллидировать
                auto colliders = table.collidersForLastDeleted();
                if (!colliders.empty()) {
                    std::cout << "Элементы, чей путь пробирования проходил через удалённый слот:\n";
                    for (auto &k : colliders) std::cout << "  " << k << "\n";
                }
            } else {
                std::cout << "Ключ не найден.\n";
            }
        }
        else if (cmd == 5) { // view segment
            size_t idx;
            std::cout << "Введите индекс сегмента [0.." << (table.capacity()?table.capacity()-1:0) << "]: ";
            std::cin >> idx;
            if (idx >= table.capacity()) {
                std::cout << "Индекс вне диапазона.\n";
                continue;
            }
            auto st = table.slot_status(idx);
            std::cout << "Индекс " << idx << " | статус=" << (int)st;
            if (st == SlotStatus::Occupied) {
                std::cout << " | key=" << table.slot_key(idx)
                          << " | value=" << table.slot_value(idx)
                          << " | probes_on_insert=" << table.slot_probes(idx);
            }
            std::cout << "\n";
        }
        else if (cmd == 6) { // stats
            std::cout << "size=" << table.size()
                      << ", capacity=" << table.capacity()
                      << ", load_factor=" << table.load_factor() << "\n";
        }
        else if (cmd == 7) { // export
            if (exportAllCSV(table, "data")) {
                std::cout << "Выгружено в data/*.csv\n";
            } else {
                std::cout << "Ошибка выгрузки.\n";
            }
        }
        else {
            std::cout << "Неизвестная команда.\n";
        }
    }
}