param([int]$Jobs = 4)
$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')
New-Item -ItemType Directory -Force build | Out-Null
if (-not (Test-Path 'build/tooling/cmake/data/bin/cmake.exe')) {
    python -m pip install --target build/tooling cmake==3.31.10 ninja==1.11.1.4
    if ($LASTEXITCODE -ne 0) { throw 'Tool installation failed' }
}
if (-not (Test-Path 'build/opencv-src/.git')) {
    git clone --depth 1 --branch 4.10.0 https://github.com/opencv/opencv.git build/opencv-src
    if ($LASTEXITCODE -ne 0) { throw 'OpenCV download failed' }
}
if ((git -C build/opencv-src rev-parse HEAD) -ne '71d3237a093b60a27601c20e9ee6c3e52154e8b1') {
    throw 'Unexpected OpenCV revision; preserve build/opencv-src and inspect it.'
}
if (-not (Test-Path 'build/json-src/.git')) {
    git clone --depth 1 --branch v3.11.3 https://github.com/nlohmann/json.git build/json-src
    if ($LASTEXITCODE -ne 0) { throw 'JSON download failed' }
}
if ((git -C build/json-src rev-parse HEAD) -ne '9cca280a4d0ccf0c08f47a99aa71d1b0e52f8d03') {
    throw 'Unexpected JSON revision; preserve build/json-src and inspect it.'
}
$cmake = (Resolve-Path build/tooling/cmake/data/bin/cmake.exe).Path
$ninja = (Resolve-Path build/tooling/bin/ninja.exe).Path
$install = Join-Path (Get-Location).Path 'build/opencv-install'
& $cmake -S build/opencv-src -B build/opencv-build -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninja" `
    -DCMAKE_BUILD_TYPE=Release '-DBUILD_LIST=core,imgproc,imgcodecs' `
    -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF `
    -DBUILD_EXAMPLES=OFF -DBUILD_opencv_apps=OFF -DBUILD_JAVA=OFF `
    -DBUILD_opencv_python3=OFF -DBUILD_JPEG=ON -DBUILD_PNG=ON `
    -DWITH_IPP=OFF -DWITH_OPENCL=OFF -DWITH_PROTOBUF=OFF -DBUILD_PROTOBUF=OFF `
    -DWITH_FFMPEG=OFF -DWITH_ADE=OFF "-DCMAKE_INSTALL_PREFIX=$install"
if ($LASTEXITCODE -ne 0) { throw 'OpenCV configuration failed' }
& $cmake --build build/opencv-build --parallel $Jobs --target install
if ($LASTEXITCODE -ne 0) { throw 'OpenCV build failed' }
$opencv = Join-Path $install 'x64/mingw/staticlib'
$json = (Resolve-Path build/json-src/single_include).Path
& $cmake -S . -B build/image-test -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninja" `
    "-DOpenCV_DIR=$opencv" "-DJSON_INCLUDE_DIR=$json" -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) { throw 'Application configuration failed' }
& $cmake --build build/image-test --parallel $Jobs
if ($LASTEXITCODE -ne 0) { throw 'Application build failed' }
& (Join-Path (Split-Path $cmake) 'ctest.exe') --test-dir build/image-test --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Image tests failed' }
Write-Host 'Image-enabled application: build/image-test/parking.exe'
