# Windows 빌드 (win-debug, win-release)
Set-Location $PSScriptRoot\..

Write-Host "[*] Configuring win-debug..."
cmake --preset win-debug
Write-Host "[*] Building win-debug..."
cmake --build --preset win-debug

Write-Host "[*] Configuring win-release..."
cmake --preset win-release
Write-Host "[*] Building win-release..."
cmake --build --preset win-release

Write-Host "[+] Windows build complete."
Write-Host "    Artifacts: build/win-debug/, build/win-release/"
