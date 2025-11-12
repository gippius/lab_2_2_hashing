#pragma once
#include <string>
#include <cstddef>

// Базовый хеш для формата БццццБ: интерпретация как число в базе 36
// + лёгкий xorshift
std::size_t hash_key36(const std::string& key);
