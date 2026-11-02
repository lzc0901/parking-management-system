#pragma once

#include <cstdint>
#include <ctime>

struct ParkingFee {
    std::int64_t totalMinutes = 0;
    std::int64_t days = 0;
    int hours = 0;
    int minutes = 0;
    int startedHoursInPartialBlock = 0;
    std::int64_t mop = 0;
};

// Both times are normalized to local minutes. Failure leaves result unchanged.
bool calculateParkingFee(std::time_t entry, std::time_t reference, ParkingFee& result);
