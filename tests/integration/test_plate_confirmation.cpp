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

#include <iostream>
#include <sstream>
#include <string>

#include "src/plate_recognition.h"

class FakeRecognizer : public PlateRecognizer {
 public:
  RecognitionResult result{
      RecognitionStatus::kOk, {"MA-12-34", "MB-56-78"}, ""};
  int calls = 0;
  std::string path;
  RecognitionResult Recognize(const std::string& image_path) override {
    ++calls;
    path = image_path;
    return result;
  }
};

int main() {
  int failures = 0;
  auto check = [&](bool ok, const char* name) {
    if (!ok) {
      std::cerr << "FAIL: " << name << '\n';
      ++failures;
    }
  };
  FakeRecognizer recognizer;
  std::ostringstream out;
  std::string plate = "unchanged";
  std::istringstream multiple("1\ny\nimage path.jpg\n2\nMC-00-01\ny\n");
  check(ReadConfirmedPlate(multiple, out, recognizer, plate) ==
                PlateInputResult::kConfirmed &&
            plate == "MC-00-01" && recognizer.path == "image path.jpg",
        "multiple candidates and correction");
  plate = "unchanged";
  std::istringstream cancel("1\nn\n0\n");
  check(ReadConfirmedPlate(cancel, out, recognizer, plate) ==
                PlateInputResult::kCancelled &&
            plate == "unchanged" && recognizer.calls == 1,
        "cancel upload without recognition");
  std::istringstream eof("1\ny\nimage.jpg\n1\n\n");
  check(ReadConfirmedPlate(eof, out, recognizer, plate) ==
                PlateInputResult::kEndOfInput &&
            plate == "unchanged",
        "EOF before confirmation");
  recognizer.result = {RecognitionStatus::kError, {}, "network error"};
  std::istringstream retry("1\ny\nimage.jpg\n2\nMA-12-34\ny\n");
  check(ReadConfirmedPlate(retry, out, recognizer, plate) ==
                PlateInputResult::kConfirmed &&
            plate == "MA-12-34",
        "error then manual fallback");
  recognizer.result = {RecognitionStatus::kNoPlate, {}, "no plate"};
  std::istringstream empty("1\ny\nimage.jpg\n0\n");
  plate = "unchanged";
  check(ReadConfirmedPlate(empty, out, recognizer, plate) ==
                PlateInputResult::kCancelled &&
            plate == "unchanged",
        "empty result does not confirm a plate");
  recognizer.result = {RecognitionStatus::kOk, {"MA-12-34", "MB-56-78"}, ""};
  std::istringstream select("1\ny\nimage.jpg\n2\n\ny\n");
  check(ReadConfirmedPlate(select, out, recognizer, plate) ==
                PlateInputResult::kConfirmed &&
            plate == "MB-56-78",
        "explicit candidate selection");
  if (failures) return 1;
  std::cout << "Plate confirmation tests passed.\n";
}
