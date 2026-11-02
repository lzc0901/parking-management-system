// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/fee_calculator.h"

#include <cmath>
#include <cstdint>
#include <ctime>
#include <limits>

#include "src/vehicle.h"

bool CalculateParkingFee(std::time_t entry, std::time_t reference,
                         ParkingFee& result) {
  std::time_t start, end;
  if (!NormalizeToMinute(entry, start) || !NormalizeToMinute(reference, end))
    return false;
  const double seconds = std::difftime(end, start);
  if (seconds < 0 || !std::isfinite(seconds) ||
      seconds / 60.0 >
          static_cast<double>(std::numeric_limits<std::int64_t>::max() / 400))
    return false;
  ParkingFee fee;
  fee.total_minutes = static_cast<std::int64_t>(seconds / 60.0);
  fee.days = fee.total_minutes / 1440;
  const auto remainder = fee.total_minutes % 1440;
  fee.hours = static_cast<int>(remainder / 60);
  fee.minutes = static_cast<int>(remainder % 60);
  fee.started_hours_in_partial_block = static_cast<int>((remainder + 59) / 60);
  // Each full 24-hour block costs 400 MOP; the remainder costs 20 MOP
  // per started hour, capped at 400 (e.g. 24h01m = 420 MOP).
  const int partial = fee.started_hours_in_partial_block * 20;
  fee.mop = fee.days * 400 + (partial < 400 ? partial : 400);
  result = fee;
  return true;
}
