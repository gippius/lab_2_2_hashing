#include "app/menu.hpp"

#ifdef _WIN32
  #include <windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
#endif

    size_t M = 3001; // ближайшее простое к 3000
    analyze_hash_distribution(1000, M);

    // для работы через меню
    // runMenu();

    return 0;
}
