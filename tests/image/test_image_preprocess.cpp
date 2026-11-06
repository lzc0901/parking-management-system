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

#include <windows.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "src/image/image_preprocess.h"

int main() {
  int failures = 0;
  auto check = [&](bool ok, const char* name) {
    if (!ok) {
      std::cerr << "FAIL: " << name << '\n';
      ++failures;
    }
  };
  wchar_t directory[MAX_PATH], unique[MAX_PATH];
  if (!GetTempPathW(MAX_PATH, directory) ||
      !GetTempFileNameW(directory, L"pms", 0, unique))
    return 1;
  const std::wstring wide = std::wstring(unique) + L" 中文 image.png";
  const int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0,
                                       nullptr, nullptr);
  if (!size) return 1;
  std::vector<char> utf8(size);
  if (!WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), size,
                           nullptr, nullptr))
    return 1;
  const std::string path(utf8.data());
  std::vector<unsigned char> png;
  cv::imencode(".png", cv::Mat(32, 32, CV_8UC3, cv::Scalar(20, 40, 60)), png);
  {
    FILE* file = _wfopen(wide.c_str(), L"wb");
    if (!file) return 1;
    std::fwrite(png.data(), 1, png.size(), file);
    std::fclose(file);
  }
  std::vector<unsigned char> output;
  std::string error;
  check(PrepareImage(path, output, error) && !output.empty() &&
            !cv::imdecode(output, cv::IMREAD_COLOR).empty(),
        "Chinese and spaced path decodes to JPEG");
  {
    FILE* file = _wfopen(wide.c_str(), L"wb");
    if (!file) return 1;
    std::fputs("bad image", file);
    std::fclose(file);
  }
  check(!PrepareImage(path, output, error), "invalid image rejected");
  {
    FILE* file = _wfopen(wide.c_str(), L"wb");
    if (!file) return 1;
    const unsigned char huge_png[] = {
        0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a, 0, 0, 0, 13,
        'I',  'H', 'D', 'R', 0,    1,    0,    0,    0, 1, 0, 0};
    std::fwrite(huge_png, 1, sizeof(huge_png), file);
    std::fclose(file);
  }
  check(!PrepareImage(path, output, error),
        "oversized PNG dimensions rejected before decode");
  {
    FILE* file = _wfopen(wide.c_str(), L"wb");
    if (!file) return 1;
    std::fseek(file, 10 * 1024 * 1024, SEEK_SET);
    std::fputc('x', file);
    std::fclose(file);
  }
  check(!PrepareImage(path, output, error), "large file rejected");
  DeleteFileW(wide.c_str());
  DeleteFileW(unique);
  if (failures) return 1;
  std::cout << "Image preprocessing tests passed.\n";
}
