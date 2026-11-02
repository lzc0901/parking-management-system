#include "menus.h"
#include "storage.h"
#include <iostream>

int main() {
    ParkingService service;
    Storage storage;
    auto loaded = storage.load(service);
    std::cout << loaded.message << '\n';
    if (!loaded.ok) return 1;
    runMenu(std::cin, std::cout, service, storage);
    auto saved = storage.save(service);
    std::cout << saved.message << '\n';
    return saved.ok ? 0 : 1;
}
