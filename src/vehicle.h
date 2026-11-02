#pragma once

#include <ctime>
#include <string>

// The entry timestamp is derived from the canonical local-time text.
struct VehicleRecord {
    std::string plate;
    std::string category;
    std::string color;
    std::string entryTimeText;
    std::time_t entryTimestamp{};
};

// Reject ASCII controls before trimming outer ASCII spaces.
bool normalizeField(const std::string& input, bool plate, std::string& output);
bool parseTimestamp(const std::string& input, std::string& canonical, std::time_t& timestamp);
bool normalizeToMinute(std::time_t input, std::time_t& output);
