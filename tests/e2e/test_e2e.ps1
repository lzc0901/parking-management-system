$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$buildDir = Join-Path $repoRoot 'build'
$exe = (Resolve-Path (Join-Path $buildDir 'parking.exe')).Path
$testDir = Join-Path $buildDir ('e2e-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testDir | Out-Null
function Run-App([string[]]$lines) {
    $result = $lines | & $exe
    if ($LASTEXITCODE -ne 0) { throw "Application exited $LASTEXITCODE" }
    return ($result -join "`n")
}
function Require([bool]$condition, [string]$message) { if (-not $condition) { throw $message } }
Push-Location $testDir
try {
    $first = Run-App @('1','1','4','0','2','4','A1','SUV','White','2020-01-01 00:00','4','B2','Sedan','Blue','2020-01-01 00:00','4','A1','SUV','White','2020-01-01 00:00','5','A1','B2','SUV','White','2020-01-01 00:00','5','A1','A1','SUV','Red','2020-01-01 00:00','0','1','2','A1','3','0','0')
    Require ($first.Contains('No vehicles.') -and $first.Contains('Invalid choice.')) 'Empty lot or ordinary role rejection missing'
    Require ($first.Contains('Plate already exists.') -and $first.Contains('Color: Red') -and $first.Contains('Current count: 2') -and $first.Contains('Duration:') -and $first.Contains('Fee:')) 'CRUD/query output mismatch'
    $data = Get-Content -Raw parking_data.txt
    Require ($data.Contains("A1`tSUV`tRed`t2020-01-01 00:00") -and $data.Contains("B2`tSedan`tBlue`t2020-01-01 00:00")) 'Saved records mismatch'
    $second = Run-App @('1','2','A1','0','2','6','B2','0','0')
    Require ($second.Contains('Color: Red') -and $second.Contains('Current count: 2') -and $second.Contains('Updated and saved.')) 'Reload or delete failed'
    $third = Run-App @('1','1','3','0','0')
    Require ($third.Contains('Plate: A1') -and -not $third.Contains('Plate: B2') -and $third.Contains('Current count: 1')) 'Restart after delete failed'
    $data = Get-Content -Raw parking_data.txt
    Require ($data.Contains('A1') -and -not $data.Contains('B2')) 'File after delete mismatch'
    Write-Output 'End-to-end multi-process workflow passed.'
} finally { Pop-Location }
