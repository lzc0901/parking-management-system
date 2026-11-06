"""Build and package the image-enabled Windows executable and source."""

import argparse
import hashlib
import os
from pathlib import Path
import subprocess
import zipfile


ROOT = Path(__file__).resolve().parents[1]
NAME = "parking-management-system-image"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.parse_args()
    if os.name != "nt":
        parser.error("This release includes a Windows executable; build on Windows.")
    build = ROOT / "build"
    dist = ROOT / "dist"
    build.mkdir(exist_ok=True)
    dist.mkdir(exist_ok=True)
    executable = build / "image-test/parking.exe"
    subprocess.run(
        ["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
         "scripts/build_image.ps1"], cwd=ROOT, check=True)
    files = {"bin/parking.exe": executable}
    for folder in ("src", "tests"):
        for path in sorted((ROOT / folder).rglob("*")):
            if path.is_file() and path.suffix in (".cpp", ".h", ".ps1"):
                files[path.relative_to(ROOT).as_posix()] = path
    for path in sorted((ROOT / "docs").iterdir()):
        if path.is_file() and path.suffix in (".md", ".html", ".pdf"):
            files[path.relative_to(ROOT).as_posix()] = path
    for path in sorted((ROOT / "docs/diagrams").iterdir()):
        if path.is_file() and path.suffix in (".mmd", ".svg"):
            files[path.relative_to(ROOT).as_posix()] = path
    for name in ("LICENSE", ".clang-format", "CMakeLists.txt", "scripts/build_image.ps1"):
        files[name] = ROOT / name
    files["licenses/opencv-LICENSE.txt"] = build / "opencv-src/LICENSE"
    files["licenses/nlohmann-json-LICENSE.txt"] = build / "json-src/LICENSE.MIT"
    for path in (build / "opencv-install/etc/licenses").glob("*"):
        if path.is_file():
            files["licenses/opencv/" + path.name] = path
    source_readme = (ROOT / "README.md").read_text(encoding="utf-8")
    tests = source_readme.split("## Tests\n", 1)[1].split("## Local persistence", 1)[0]
    readme = """# Parking management system (image-enabled)

This Windows delivery includes the executable, C++17 source, tests and reports.
Run commands from the extracted package root. Data is stored as parking_data.txt
in the working directory; successful administrator changes are saved immediately.

## Run

```powershell
./bin/parking.exe
```

Choose 1 for ordinary user, 2 for administrator, 0 to exit or log out.
Enter local timestamps as YYYY-MM-DD HH:MM.

## Build from source

Install Python with pip, Git and LLVM-MinGW UCRT, then run:

```powershell
./scripts/build_image.ps1
./build/image-test/parking.exe
```

## Tests

""" + tests + """
## Image recognition

Set GEMINI_API_KEY and GEMINI_MODEL in the local environment before running.
The image is sent to Gemini only after confirmation. Review and correct the
result before querying or registering. The role selector is not authentication.
Real Gemini requests and Macau sample accuracy have not been validated.
Windows UCRT and WinHTTP are required; OpenCV and compiler runtimes are linked
statically. Third-party licenses are in licenses/.

## Documentation

- [User guide](docs/juli11_user_guide_zh.md)
- [Development report](docs/juli12_development_report_zh.md)
- [Printable report](docs/juli12_development_report_zh.pdf)
- [HTML report](docs/juli12_development_report_zh.html)
- [Demo checklist](docs/juli13_demo_script_and_checklist_zh.md)
- [Historical viewing guide](docs/juli14_viewing_guide_and_alignment_zh.md)

Historical videos, evidence, assignment references and maintenance scripts are
not included in this delivery. See LICENSE for Apache License 2.0 terms.
"""
    output = dist / (NAME + ".zip")
    temporary = output.with_suffix(".zip.tmp")
    try:
        with zipfile.ZipFile(temporary, "w", zipfile.ZIP_DEFLATED) as archive:
            for name, path in sorted(files.items()):
                archive.write(path, NAME + "/" + name)
            archive.writestr(NAME + "/README.md", readme.encode("utf-8"))
        with zipfile.ZipFile(temporary) as archive:
            if archive.testzip() is not None:
                raise ValueError("Release ZIP failed integrity validation")
        temporary.replace(output)
    finally:
        temporary.unlink(missing_ok=True)
    digest = hashlib.sha256(output.read_bytes()).hexdigest()
    (dist / "SHA256SUMS.txt").write_text(
        f"{digest}  {output.name}\n", encoding="utf-8", newline="\n")
    print(f"{output}\nSHA256: {digest}\nFiles: {len(files) + 1}")


if __name__ == "__main__":
    main()
