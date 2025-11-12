#include "ds/hash_table.hpp"
#include "ds/hash_func.hpp"

#include <cmath>

HashTable::HashTable(size_t capacity) {
    reinit(capacity);
}

double HashTable::load_factor() const {
    if (M == 0) return 0.0;
    return static_cast<double>(sz) / static_cast<double>(M);
}

SlotStatus HashTable::slot_status(size_t idx) const {
    return table[idx].status;
}
const std::string& HashTable::slot_key(size_t idx) const {
    return table[idx].key;
}
const std::string& HashTable::slot_value(size_t idx) const {
    return table[idx].value;
}
int HashTable::slot_probes(size_t idx) const {
    return table[idx].probes_on_insert;
}

void HashTable::clear() {
    for (auto &e : table) e = Entry{};
    sz = 0;
    last_deleted_idx = (size_t)-1;
    last_colliders.clear();
}

void HashTable::reinit(size_t new_capacity) {
    if (new_capacity < 5) new_capacity = 5;
    size_t p = next_prime(new_capacity);
    M = p;
    table.assign(M, Entry{});
    sz = 0;
    last_deleted_idx = (size_t)-1;
    last_colliders.clear();
}

size_t HashTable::raw_hash(const std::string& key) const {
    return hash_key36(key); // наша базовая хеш-функция
}

size_t HashTable::index_for(size_t h, int i) const {
    // квадратичное опробование: c1=c2=1
    return (h + i + i*1ull*i) % M;
}

bool HashTable::insert(const std::string& key, const std::string& value, int* probes_out) {
    if (M == 0) return false;
    maybe_rehash();

    const size_t h = raw_hash(key) % M;
    int firstDeleted = -1;
    int probes = 0;

    // проверка на дубликат и поиск места
    for (int i = 0; i < static_cast<int>(M); ++i) {
        const size_t idx = index_for(h, i);
        ++probes;
        auto &e = table[idx];

        if (e.status == SlotStatus::Occupied) {
            if (e.key == key) {
                // дубликат — не вставляем
                if (probes_out) *probes_out = probes;
                return false;
            }
            continue; // продолжаем пробирование
        }
        // в soft deleted слот нельзя писать, потому что открытая адресация предполагает, что
        // если элемент можно было бы найти по цепочке проб, то она не должна быть
        // прервана никаким другим элементом
        if (e.status == SlotStatus::Deleted) {
            if (firstDeleted == -1) firstDeleted = static_cast<int>(idx);
            continue;
        }
        if (e.status == SlotStatus::Empty) {
            int target = (firstDeleted != -1) ? firstDeleted : static_cast<int>(idx);
            table[target].key = key;
            table[target].value = value;
            table[target].status = SlotStatus::Occupied;
            table[target].probes_on_insert = probes;
            ++sz;
            if (probes_out) *probes_out = probes;
            maybe_rehash();
            return true;
        }
    }

    // не нашли место — форсированный ре-хеш и повтор
    rehash_to(next_prime(M * 2));
    return insert(key, value, probes_out);
}

bool HashTable::find(const std::string& key, std::string* value_out, int* probes_out) const {
    if (M == 0) return false;
    const size_t h = raw_hash(key) % M;
    int probes = 0;

    for (int i = 0; i < static_cast<int>(M); ++i) {
        size_t idx = index_for(h, i);
        ++probes;
        const auto &e = table[idx];

        if (e.status == SlotStatus::Empty) {
            if (probes_out) *probes_out = probes;
            return false; // цепочка прервалась
        }
        if (e.status == SlotStatus::Occupied && e.key == key) {
            if (value_out) *value_out = e.value;
            if (probes_out) *probes_out = probes;
            return true;
        }
        // Deleted — продолжаем
    }
    if (probes_out) *probes_out = probes;
    return false;
}

bool HashTable::erase(const std::string& key) {
    if (M == 0) return false;
    const size_t h = raw_hash(key) % M;

    for (int i = 0; i < static_cast<int>(M); ++i) {
        const size_t idx = index_for(h, i);
        auto &e = table[idx];

        if (e.status == SlotStatus::Empty) {
            return false; // не нашли
        }
        if (e.status == SlotStatus::Occupied && e.key == key) {
            e.status = SlotStatus::Deleted;
            --sz;
            last_deleted_idx = idx;

            // считаем коллизии
            last_colliders.clear();
            // идём вправо по кластеру, пока не Empty
            for (size_t j = (idx + 1) % M; ; j = (j + 1) % M) {
                const auto &ej = table[j];
                if (ej.status == SlotStatus::Empty) break;
                if (ej.status == SlotStatus::Occupied) {
                    size_t hj = raw_hash(ej.key) % M;
                    // симулируем путь до j
                    for (int t = 0; t < (int)M; ++t) {
                        size_t path = index_for(hj, t);
                        if (path == last_deleted_idx) {
                            last_colliders.push_back(ej.key);
                            break;
                        }
                        if (path == j) break; // дошли до места хранения — далее не пойдёт
                    }
                }
                if (j == idx) break; // на всякий случай избегаем бесконечного круга
            }
            return true;
        }
    }
    return false;
}

std::vector<std::string> HashTable::collidersForLastDeleted() const {
    return last_colliders;
}

// рехеш, если превышен load_factor
void HashTable::maybe_rehash() {
    if (load_factor() > max_load) {
        rehash_to(next_prime(M * 2));
    }
}

void HashTable::rehash_to(size_t newM) {
    std::vector<Entry> old = table;
    table.assign(newM, Entry{});
    M = newM;
    sz = 0;
    last_deleted_idx = (size_t)-1;
    last_colliders.clear();

    for (auto &e : old) {
        if (e.status == SlotStatus::Occupied) {
            int dummy = 0;
            insert(e.key, e.value, &dummy);
            // пробу при ре-инсерте можно не сохранять для отчёта
        }
    }
}

// для рехеширования используем простые числа, чтобы хеши не совпадали по модулю с повторяющимися
// закономерностями цифр и букв - так снижается вероятность коллизий
bool HashTable::is_prime(size_t n) {
    if (n < 2) return false;
    if (n % 2 == 0) return n == 2;
    for (size_t d = 3; d * d <= n; d += 2) {
        if (n % d == 0) return false;
    }
    return true;
}

size_t HashTable::next_prime(size_t n) {
    if (n <= 2) return 2;
    size_t p = (n % 2 == 0) ? n + 1 : n;
    while (!is_prime(p)) p += 2;
    return p;
}
