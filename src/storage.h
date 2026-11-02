#pragma once
#include "parking_service.h"
#include <filesystem>
#include <string>

struct StorageResult { bool ok; std::string message; };

// Load before saving. A failed load or failed rollback blocks subsequent saves.
class Storage {
public:
    explicit Storage(std::filesystem::path path = "parking_data.txt");
    StorageResult load(ParkingService& service);
    StorageResult save(const ParkingService& service);
private:
    std::filesystem::path path_;
    bool blocked_ = true;
};
