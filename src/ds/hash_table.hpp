#pragma once
#include <string>
#include <vector>
#include <cstddef>

enum class SlotStatus { Empty = 0, Occupied = 1, Deleted = 2 };

// элемент таблицы
struct Entry {
    std::string key;
    std::string value;
    SlotStatus status = SlotStatus::Empty;
    int probes_on_insert = 0;
};

class HashTable {
public:
    explicit HashTable(size_t capacity = 101);

    bool insert(const std::string& key, const std::string& value, int* probes_out = nullptr);
    bool find(const std::string& key, std::string* value_out = nullptr, int* probes_out = nullptr) const;
    bool erase(const std::string& key);

    void clear();
    void reinit(size_t new_capacity);

    size_t size() const { return sz; }
    size_t capacity() const { return M; }
    double load_factor() const;

    // Доступ для просмотра сегмента и экспорта
    SlotStatus slot_status(size_t idx) const;
    const std::string& slot_key(size_t idx) const;
    const std::string& slot_value(size_t idx) const;
    int slot_probes(size_t idx) const;

    // После erase — список ключей, пути которых проходили через последний удалённый индекс
    std::vector<std::string> collidersForLastDeleted() const;

private:
    std::vector<Entry> table;
    size_t M = 0; // емкость таблицы
    size_t sz = 0; // фактический размер таблицы
    double max_load = 0.7;
    int c1 = 1, c2 = 1; // коэффициенты квадратичного пробирования

    mutable size_t last_deleted_idx = (size_t)-1; // для отчёта
    mutable std::vector<std::string> last_colliders; // кеш последней выборки

    size_t raw_hash(const std::string& key) const; // базовый хеш (из hash_func)
    size_t index_for(size_t h, int i) const;       // (h + i + i*i) mod M

    void maybe_rehash();
    void rehash_to(size_t newM);

    static bool is_prime(size_t n);
    static size_t next_prime(size_t n);
};
