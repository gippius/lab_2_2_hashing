#include "ds/exporter.hpp"
#include "ds/hash_table.hpp"

#include <fstream>
#include <filesystem>

static bool ensureDir(const std::string& path) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (fs::exists(path, ec)) return true;
    return fs::create_directories(path, ec);
}

bool exportAllCSV(const HashTable& table, const std::string& dir) {
    if (!ensureDir(dir)) return false;

    // slots.csv: index;state;key
    {
        std::ofstream out(dir + "/slots.csv");
        if (!out) return false;
        out << "index;state;key\n";
        for (size_t i = 0; i < table.capacity(); ++i) {
            auto st = table.slot_status(i);
            out << i << ";" << (int)st << ";";
            if (st == SlotStatus::Occupied) out << table.slot_key(i);
            out << "\n";
        }
    }

    // probes.csv: key;probes_on_insert (только для занятых)
    {
        std::ofstream out(dir + "/probes.csv");
        if (!out) return false;
        out << "key;probes_on_insert\n";
        for (size_t i = 0; i < table.capacity(); ++i) {
            if (table.slot_status(i) == SlotStatus::Occupied) {
                out << table.slot_key(i) << ";" << table.slot_probes(i) << "\n";
            }
        }
    }

    // clusters.csv: cluster_id;length (непрерывные участки Occupied)
    {
        std::ofstream out(dir + "/clusters.csv");
        if (!out) return false;
        out << "cluster_id;length\n";
        size_t cluster_id = 0;
        size_t i = 0, N = table.capacity();
        while (i < N) {
            // пропускаем не-Occupied
            while (i < N && table.slot_status(i) != SlotStatus::Occupied) ++i;
            if (i >= N) break;
            size_t start = i;
            while (i < N && table.slot_status(i) == SlotStatus::Occupied) ++i;
            size_t len = i - start;
            if (len > 0) {
                out << cluster_id++ << ";" << len << "\n";
            }
        }
    }

    return true;
}
