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

#include "src/plate_recognition.h"

#include <istream>
#include <ostream>
#include <string>

#include "src/vehicle.h"

PlateInputResult ReadConfirmedPlate(std::istream& in, std::ostream& out,
                                    PlateRecognizer& recognizer,
                                    std::string& plate) {
  std::string choice;
  while (true) {
    out << "1 = import image, 2 = manual entry, 0 = cancel\n> ";
    if (!std::getline(in, choice)) return PlateInputResult::kEndOfInput;
    if (choice == "0") return PlateInputResult::kCancelled;
    std::string candidate;
    if (choice == "1") {
      out << "The image will be sent to Gemini. Continue? (y/n): ";
      if (!std::getline(in, choice)) return PlateInputResult::kEndOfInput;
      if (choice != "y" && choice != "Y") continue;
      out << "Image path: ";
      std::string path;
      if (!std::getline(in, path)) return PlateInputResult::kEndOfInput;
      const auto result = recognizer.Recognize(path);
      if (result.status != RecognitionStatus::kOk ||
          result.candidates.empty()) {
        out << "Recognition failed or no readable plate. " << result.message
            << '\n';
        continue;
      }
      for (std::size_t i = 0; i < result.candidates.size(); ++i)
        out << i + 1 << ": " << result.candidates[i] << '\n';
      if (result.candidates.size() == 1) {
        candidate = result.candidates.front();
      } else {
        out << "Choose a candidate number, or 0 to retry: ";
        if (!std::getline(in, choice)) return PlateInputResult::kEndOfInput;
        bool found = false;
        for (std::size_t i = 0; i < result.candidates.size(); ++i) {
          if (choice == std::to_string(i + 1)) {
            candidate = result.candidates[i];
            found = true;
            break;
          }
        }
        if (!found) continue;
      }
    } else if (choice != "2") {
      out << "Invalid choice.\n";
      continue;
    }
    while (true) {
      out << "Plate [" << candidate
          << "]: enter correction, or empty to accept: ";
      std::string correction;
      if (!std::getline(in, correction)) return PlateInputResult::kEndOfInput;
      if (!correction.empty()) candidate = correction;
      std::string normalized;
      if (!NormalizeField(candidate, true, normalized)) {
        out << "Invalid plate.\n";
        continue;
      }
      out << "Confirm plate " << normalized << "? (y/n): ";
      if (!std::getline(in, choice)) return PlateInputResult::kEndOfInput;
      if (choice == "y" || choice == "Y") {
        plate = normalized;
        return PlateInputResult::kConfirmed;
      }
      break;
    }
  }
}
