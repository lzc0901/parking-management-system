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

#ifndef SRC_PLATE_RECOGNITION_H_
#define SRC_PLATE_RECOGNITION_H_

#include <iosfwd>
#include <string>
#include <vector>

enum class RecognitionStatus { kOk, kNoPlate, kUnreadable, kError };

struct RecognitionResult {
  RecognitionStatus status;
  std::vector<std::string> candidates;
  std::string message;
};

// Implementations only inspect the supplied image; they do not access records.
class PlateRecognizer {
 public:
  virtual ~PlateRecognizer() = default;
  virtual RecognitionResult Recognize(const std::string& path) = 0;
};

enum class PlateInputResult { kConfirmed, kCancelled, kEndOfInput };

// Confirms one candidate or a manually entered correction. Never queries or
// changes parking records. The recognizer is replaceable for offline tests.
PlateInputResult ReadConfirmedPlate(std::istream& in, std::ostream& out,
                                    PlateRecognizer& recognizer,
                                    std::string& plate);

#endif  // SRC_PLATE_RECOGNITION_H_
