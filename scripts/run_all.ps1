$builds = @("build\win-debug", "build\win-release")

foreach ($build in $builds) {
    $exes = @(
        "$build\dfbench_win_core.exe",
        "$build\dfbench_cpp.exe",
        "$build\dfbench_cpp_exceptions.exe"
    )

    foreach ($exe in $exes) {
        $full = Join-Path $PSScriptRoot "..\" $exe
        if (Test-Path $full) {
            Write-Host "[*] $exe --list"
            & $full --list

            Write-Host "[*] $exe --run-all"
            & $full --run-all
        }
    }
}
