#include "fee_calculator.h"
#include "vehicle.h"
#include <cmath>
#include <limits>

bool calculateParkingFee(std::time_t entry, std::time_t reference, ParkingFee& result) {
    std::time_t start, end;
    if (!normalizeToMinute(entry, start) || !normalizeToMinute(reference, end)) return false;
    const double seconds = std::difftime(end, start);
    if (seconds < 0 || !std::isfinite(seconds) ||
        seconds / 60.0 > static_cast<double>(std::numeric_limits<std::int64_t>::max() / 400)) return false;
    ParkingFee fee;
    fee.totalMinutes = static_cast<std::int64_t>(seconds / 60.0);
    fee.days = fee.totalMinutes / 1440;
    const auto remainder = fee.totalMinutes % 1440;
    fee.hours = static_cast<int>(remainder / 60);
    fee.minutes = static_cast<int>(remainder % 60);
    fee.startedHoursInPartialBlock = static_cast<int>((remainder + 59) / 60);
    // Each full 24-hour block costs 400 MOP; the remainder costs 20 MOP
    // per started hour, capped at 400 (e.g. 24h01m = 420 MOP).
    const int partial = fee.startedHoursInPartialBlock * 20;
    fee.mop = fee.days * 400 + (partial < 400 ? partial : 400);
    result = fee;
    return true;
}
