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

#include <cstdlib>
#include <iostream>
#include <string>

#include "src/image/gemini_recognizer.h"

int main() {
  int failures = 0;
  auto check = [&](bool ok, const char* name) {
    if (!ok) {
      std::cerr << "FAIL: " << name << '\n';
      ++failures;
    }
  };
  const std::string good =
      R"({"candidates":[{"finishReason":"STOP","content":{"parts":[{"text":"{\"status\":\"ok\",\"plates\":[\"MA-12-34\",\"MB-56-78\"]}"}]}}]})";
  auto parsed = ParseGeminiResponse(good);
  check(
      parsed.status == RecognitionStatus::kOk && parsed.candidates.size() == 2,
      "valid multiple candidates");
  check(ParseGeminiResponse("not json").status == RecognitionStatus::kError,
        "malformed response");
  check(ParseGeminiResponse(R"({"candidates":[{"finishReason":"SAFETY"}]})")
                .status == RecognitionStatus::kError,
        "refusal response");
  const std::string empty =
      R"({"candidates":[{"finishReason":"STOP","content":{"parts":[{"text":"{\"status\":\"none\",\"plates\":[]}"}]}}]})";
  check(ParseGeminiResponse(empty).status == RecognitionStatus::kNoPlate,
        "no plate response");
  _putenv_s("GEMINI_API_KEY", "");
  _putenv_s("GEMINI_MODEL", "");
  GeminiRecognizer recognizer;
  check(recognizer.Recognize("missing.png").status == RecognitionStatus::kError,
        "missing configuration is handled without network");
  if (failures) return 1;
  std::cout << "Gemini response tests passed.\n";
}
