#include "ds/key_utils.hpp"
#include <cctype>

bool isValidKey(const std::string& key) {
    if (key.size() != 6) return false;
    return std::isupper(static_cast<unsigned char>(key[0])) &&
           std::isdigit(static_cast<unsigned char>(key[1])) &&
           std::isdigit(static_cast<unsigned char>(key[2])) &&
           std::isdigit(static_cast<unsigned char>(key[3])) &&
           std::isdigit(static_cast<unsigned char>(key[4])) &&
           std::isupper(static_cast<unsigned char>(key[5]));
}

// приведение к прописному формату
std::string normalizeKey(std::string key) {
    for (char &c : key) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return key;
}
