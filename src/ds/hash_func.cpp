#include "ds/hash_func.hpp"
#include <cctype>
#include <cstdint>

static int val36(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'z') return 10 + (c - 'a');
    return 0;
}

std::size_t hash_key36(const std::string& key) {
    // соберём число в базе 36 (длина 6 символов, но сделаем универсально)
    unsigned long long H = 0;
    for (char c : key) {
        int v = val36(c);
        H = H * 36ull + static_cast<unsigned long long>(v);
    }

    // xorshift-миксер (для равномерности на учебных данных)
    H ^= (H << 13);
    H ^= (H >> 7);
    H ^= (H << 17);
    return H;
}
