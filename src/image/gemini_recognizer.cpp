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

#include "src/image/gemini_recognizer.h"

#include <windows.h>
#include <winhttp.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"
#include "src/image/image_preprocess.h"

namespace {
using json = nlohmann::json;

struct InternetHandle {
  HINTERNET handle = nullptr;
  ~InternetHandle() {
    if (handle) WinHttpCloseHandle(handle);
  }
};

std::string Base64(const std::vector<unsigned char>& input) {
  const char alphabet[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string output;
  for (std::size_t i = 0; i < input.size(); i += 3) {
    const unsigned int n =
        (static_cast<unsigned int>(input[i]) << 16) |
        (i + 1 < input.size() ? static_cast<unsigned int>(input[i + 1]) << 8
                              : 0) |
        (i + 2 < input.size() ? input[i + 2] : 0);
    output.push_back(alphabet[(n >> 18) & 63]);
    output.push_back(alphabet[(n >> 12) & 63]);
    output.push_back(i + 1 < input.size() ? alphabet[(n >> 6) & 63] : '=');
    output.push_back(i + 2 < input.size() ? alphabet[n & 63] : '=');
  }
  return output;
}

RecognitionResult Error(const std::string& message) {
  return {RecognitionStatus::kError, {}, message};
}
}  // namespace

RecognitionResult ParseGeminiResponse(const std::string& response) {
  try {
    const auto envelope = json::parse(response);
    const auto& candidate = envelope.at("candidates").at(0);
    if (candidate.value("finishReason", "") != "STOP")
      return Error("Gemini did not finish a response.");
    const auto result = json::parse(candidate.at("content")
                                        .at("parts")
                                        .at(0)
                                        .at("text")
                                        .get<std::string>());
    const auto status = result.at("status").get<std::string>();
    const auto& plates = result.at("plates");
    if (!plates.is_array() || plates.size() > 20)
      return Error("Invalid response format.");
    std::vector<std::string> candidates;
    for (const auto& value : plates) {
      const auto plate = value.get<std::string>();
      if (plate.empty() || plate.size() > 80 ||
          std::any_of(plate.begin(), plate.end(),
                      [](unsigned char c) { return c < 0x20 || c == 0x7f; }))
        return Error("Invalid candidate in response.");
      candidates.push_back(plate);
    }
    if (status == "ok" && !candidates.empty())
      return {RecognitionStatus::kOk, candidates, ""};
    if (status == "none" && candidates.empty())
      return {RecognitionStatus::kNoPlate, {}, "No plate found."};
    if (status == "unreadable" && candidates.empty())
      return {RecognitionStatus::kUnreadable, {}, "Plate is unreadable."};
    return Error("Invalid response format.");
  } catch (const std::exception&) {
    return Error("Invalid or refused Gemini response.");
  }
}

RecognitionResult GeminiRecognizer::Recognize(const std::string& path) {
  const char* api_key = std::getenv("GEMINI_API_KEY");
  const char* model_env = std::getenv("GEMINI_MODEL");
  if (!api_key || !*api_key || !model_env || !*model_env)
    return Error(
        "Set GEMINI_API_KEY and GEMINI_MODEL before using image recognition.");
  const std::string model(model_env);
  if (model.size() > 100 ||
      model.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUV"
                              "WXYZ0123456789-_.") != std::string::npos)
    return Error("Invalid model name.");
  std::vector<unsigned char> image;
  std::string image_error;
  if (!PrepareImage(path, image, image_error)) return Error(image_error);

  const json schema = {
      {"type", "object"},
      {"properties",
       {{"status",
         {{"type", "string"}, {"enum", {"ok", "none", "unreadable"}}}},
        {"plates", {{"type", "array"}, {"items", {{"type", "string"}}}}}}},
      {"required", {"status", "plates"}}};
  const json request = {
      {"contents",
       {{{"parts",
          {{{"text",
             "Read all visible vehicle registration plates. Never guess hidden "
             "or unclear characters. Return unreadable if uncertain. Preserve "
             "the registration characters exactly."}},
           {{"inline_data",
             {{"mime_type", "image/jpeg"}, {"data", Base64(image)}}}}}}}}},
      {"generationConfig",
       {{"responseMimeType", "application/json"}, {"responseSchema", schema}}}};
  const std::string body = request.dump();
  InternetHandle session{
      WinHttpOpen(L"ParkingManagement/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                  WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0)};
  if (!session.handle) return Error("Unable to initialize HTTPS.");
  if (!WinHttpSetTimeouts(session.handle, 5000, 5000, 10000, 10000))
    return Error("Unable to set HTTPS timeouts.");
  InternetHandle connection{WinHttpConnect(session.handle,
                                           L"generativelanguage.googleapis.com",
                                           INTERNET_DEFAULT_HTTPS_PORT, 0)};
  if (!connection.handle) return Error("Unable to connect to Gemini.");
  const std::wstring endpoint = L"/v1beta/models/" +
                                std::wstring(model.begin(), model.end()) +
                                L":generateContent";
  InternetHandle req{WinHttpOpenRequest(
      connection.handle, L"POST", endpoint.c_str(), nullptr, WINHTTP_NO_REFERER,
      WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE)};
  if (!req.handle) return Error("Unable to prepare HTTPS request.");
  DWORD disabled = WINHTTP_DISABLE_REDIRECTS;
  if (!WinHttpSetOption(req.handle, WINHTTP_OPTION_DISABLE_FEATURE, &disabled,
                        sizeof(disabled)))
    return Error("Unable to secure HTTPS request.");
  const std::string key(api_key);
  if (key.size() > 512 ||
      key.find_first_not_of(
          "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-") !=
          std::string::npos)
    return Error("Invalid API key configuration.");
  const std::wstring headers =
      L"Content-Type: application/json\r\nx-goog-api-key: " +
      std::wstring(key.begin(), key.end()) + L"\r\n";
  if (!WinHttpSendRequest(req.handle, headers.c_str(), static_cast<DWORD>(-1L),
                          const_cast<char*>(body.data()),
                          static_cast<DWORD>(body.size()),
                          static_cast<DWORD>(body.size()), 0) ||
      !WinHttpReceiveResponse(req.handle, nullptr))
    return Error("Gemini request failed or timed out.");
  DWORD code = 0, code_size = sizeof(code);
  if (!WinHttpQueryHeaders(
          req.handle, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
          WINHTTP_HEADER_NAME_BY_INDEX, &code, &code_size,
          WINHTTP_NO_HEADER_INDEX) ||
      code != 200)
    return Error("Gemini returned an HTTP error (" + std::to_string(code) +
                 ").");
  std::string response;
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(30);
  while (true) {
    if (std::chrono::steady_clock::now() > deadline)
      return Error("Gemini response timed out.");
    DWORD available = 0;
    if (!WinHttpQueryDataAvailable(req.handle, &available))
      return Error("Gemini response read failed.");
    if (!available) break;
    if (available > 1024 * 1024 || response.size() + available > 1024 * 1024)
      return Error("Gemini response is too large.");
    std::string chunk(available, '\0');
    DWORD received = 0;
    if (!WinHttpReadData(req.handle, &chunk[0], available, &received))
      return Error("Gemini response read failed.");
    chunk.resize(received);
    response += chunk;
  }
  return ParseGeminiResponse(response);
}
