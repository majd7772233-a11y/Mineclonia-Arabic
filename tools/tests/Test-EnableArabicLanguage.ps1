# This test exercises the real Windows PowerShell 5.1 helper against
# a fixture matching Luanti 5.17.0's on-disk structure and language files.
$ErrorActionPreference = "Stop"

$helper = Join-Path (Split-Path -Parent $PSScriptRoot) "ENABLE_ARABIC_LANGUAGE.ps1"
if (!(Test-Path -LiteralPath $helper -PathType Leaf)) {
    throw "Cannot find installer helper: $helper"
}

$powerShell = Join-Path $env:SystemRoot "System32\WindowsPowerShell\v1.0\powershell.exe"
if (!(Test-Path -LiteralPath $powerShell -PathType Leaf)) {
    throw "Windows PowerShell 5.1 was not found."
}

$testRoot = Join-Path $env:TEMP ("Mineclonia Arabic Language Patch Test " + [guid]::NewGuid().ToString("N"))

function New-LuantiFixture([string]$Name, [string]$EnumLine, [bool]$ArabicAlreadyEnabled) {
    $root = Join-Path $testRoot $Name
    $builtin = Join-Path $root "builtin"
    $settings = Join-Path $builtin "common\settings"
    $bin = Join-Path $root "bin"

    New-Item -ItemType Directory -Path $settings -Force | Out-Null
    New-Item -ItemType Directory -Path $bin -Force | Out-Null
    [IO.File]::WriteAllText((Join-Path $bin "luanti.exe"), "fixture", [Text.UTF8Encoding]::new($false))

    $settingText = @"
# Luanti 5.17.0 language option from builtin/settingtypes.txt
#    Set the language. By default, the system language is used.
$EnumLine
"@
    [IO.File]::WriteAllText((Join-Path $builtin "settingtypes.txt"), $settingText, [Text.UTF8Encoding]::new($false))

    if ($ArabicAlreadyEnabled) {
        $arabicLine = '        ar = "العربية [ar]",'
    } else {
        $arabicLine = '        --ar = " [ar]", blacklisted'
    }

    $dialogText = @"
-- Luanti 5.17.0 language labels from builtin/common/settings/dlg_settings.lua
get_setting_info("language").option_labels = {
        [""] = fgettext_ne("(Use system language)"),
$arabicLine
        en = "English [en]",
        tr = "Türkçe [tr]",
}
"@
    [IO.File]::WriteAllText((Join-Path $settings "dlg_settings.lua"), $dialogText, [Text.UTF8Encoding]::new($false))

    return [pscustomobject]@{
        Root = $root
        Builtin = $builtin
        SettingPath = Join-Path $builtin "settingtypes.txt"
        DialogPath = Join-Path $settings "dlg_settings.lua"
    }
}

function Invoke-Helper([string]$SelectedPath) {
    $output = & $powerShell -NoProfile -ExecutionPolicy Bypass -File $helper -LuantiDir $SelectedPath -NonInteractive 2>&1
    $exitCode = $LASTEXITCODE
    if ($exitCode -ne 0) {
        $detail = $output -join [Environment]::NewLine
        throw ("Helper failed for '{0}' (exit {1}):{2}{3}" -f $SelectedPath, $exitCode, [Environment]::NewLine, $detail)
    }
    return ($output -join [Environment]::NewLine)
}

function Assert-ArabicIsSecondOption($Fixture) {
    $line = [IO.File]::ReadAllLines($Fixture.SettingPath) |
        Where-Object { $_ -match '^[ \t]*language[ \t]+\(Language\)[ \t]+enum[ \t]+' } |
        Select-Object -First 1
    if (!$line) { throw "Language enum was not found in $($Fixture.SettingPath)" }

    $values = [regex]::Replace($line, '^[ \t]*language[ \t]+\(Language\)[ \t]+enum[ \t]+', '')
    $parts = @($values.Split(","))
    $arabicCount = @($parts | Where-Object { $_.Trim() -eq "ar" }).Count
    if ($arabicCount -ne 1 -or $parts.Count -lt 2 -or
        -not [string]::IsNullOrWhiteSpace($parts[0]) -or
        $parts[1].Trim() -ne "ar") {
        throw "Expected empty system-language option first and ar second, got: $values"
    }

    $dialog = [IO.File]::ReadAllText($Fixture.DialogPath)
    if ($dialog -notmatch '(?m)^[ \t]*ar[ \t]*=[ \t]*"العربية \[ar\]"[ \t]*,[ \t]*$') {
        throw "Arabic label is not active in dlg_settings.lua."
    }
    if ($dialog -match '(?m)^[ \t]*--[ \t]*ar[ \t]*=') {
        throw "The Arabic language label is still commented out."
    }
}

try {
    New-Item -ItemType Directory -Path $testRoot -Force | Out-Null

    # Case 1: the user selects the installation root (the first reported path).
    $rootFixture = New-LuantiFixture "Install Root With Spaces" "language (Language) enum   ,be,bg,ca,en,tr" $false
    $output = Invoke-Helper $rootFixture.Root
    Assert-ArabicIsSecondOption $rootFixture

    # Case 2: the user selects the builtin directory itself (the second reported path).
    $builtinFixture = New-LuantiFixture "Builtin Folder Selection" "language (Language) enum   ,be,bg,en,tr" $false
    $output = Invoke-Helper $builtinFixture.Builtin
    Assert-ArabicIsSecondOption $builtinFixture

    # Case 3: repair the exact malformed ordering left by the previous helper:
    # ar, [empty system-language item], be, ...; the label may already be active.
    $partialFixture = New-LuantiFixture "Repair Previous Partial Attempt" "language (Language) enum ar,  ,be,bg,en,tr" $true
    $output = Invoke-Helper $partialFixture.Root
    Assert-ArabicIsSecondOption $partialFixture

    # Case 4: idempotency. Running again must succeed without adding duplicates.
    $secondRun = Invoke-Helper $rootFixture.Root
    Assert-ArabicIsSecondOption $rootFixture

    Write-Host "PASS: Luanti 5.17.0 helper tests succeeded (root selection, builtin selection, previous partial repair, idempotency)." -ForegroundColor Green
}
finally {
    if (Test-Path -LiteralPath $testRoot) {
        Remove-Item -LiteralPath $testRoot -Recurse -Force
    }
}
