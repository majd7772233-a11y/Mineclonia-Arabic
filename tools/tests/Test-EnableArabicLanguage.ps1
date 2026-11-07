# Exercise the Windows PowerShell 5.1 installer against Luanti 5.17.0's
# real folder layout and the exact language enum / option_labels structure.
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

function New-LuantiFixture([string]$Name, [string]$EnumLine, [string]$ArabicEntry) {
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

    $dialogText = @"
-- Luanti 5.17.0 language labels from builtin/common/settings/dlg_settings.lua
get_setting_info("language").option_labels = {
        [""] = fgettext_ne("(Use system language)"),
$ArabicEntry
        be = "Беларуская [be]",
        en = "English [en]",
        tr = "Türkçe [tr]",
}
"@
    [IO.File]::WriteAllText((Join-Path $settings "dlg_settings.lua"), $dialogText, [Text.UTF8Encoding]::new($false))

    return [pscustomobject]@{
        Root = $root
        Builtin = $builtin
        Bin = $bin
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
        throw "Expected the empty system-language option first and ar second, got: $values"
    }

    $dialog = [IO.File]::ReadAllText($Fixture.DialogPath)
    $activeArabicLines = @(
        $dialog -split "\r?\n" |
            ForEach-Object { $_.Trim() } |
            Where-Object { $_ -ceq 'ar = "العربية [ar]",' }
    )
    if ($activeArabicLines.Count -ne 1) {
        throw "Expected exactly one active Arabic label, found $($activeArabicLines.Count). Dialog contains: $dialog"
    }
    $commentedArabicLines = @(
        $dialog -split "\r?\n" |
            ForEach-Object { $_.TrimStart() } |
            Where-Object { $_ -match '^--[ \t]*ar[ \t]*=' }
    )
    if ($commentedArabicLines.Count -gt 0) {
        throw ("The Arabic language label is still commented out. Remaining rows: " + ($commentedArabicLines -join " || ") + ". Full dialog: " + $dialog)
    }
}

try {
    New-Item -ItemType Directory -Path $testRoot -Force | Out-Null

    # Exact Luanti 5.17.0 state: root selected, Arabic commented as blacklisted.
    $rootFixture = New-LuantiFixture "Install Root With Spaces" "language (Language) enum   ,be,bg,ca,cs,da,de,el,en,eo,es,et,eu,fi,fr,gd,gl,hu,id,it,ja,jbo,kk,ko,lt,lv,ms,nb,nl,nn,pl,pt,pt_BR,ro,ru,sk,sl,sr_Cyrl,sr_Latn,sv,sw,tr,uk,vi,zh_CN,zh_TW" '        --ar = " [ar]", blacklisted'
    [void](Invoke-Helper $rootFixture.Root)
    Assert-ArabicIsSecondOption $rootFixture

    # Selecting ...\builtin must resolve to its parent installation root.
    $builtinFixture = New-LuantiFixture "Builtin Folder Selection" "language (Language) enum   ,be,bg,en,tr" '        --ar = " [ar]", blacklisted'
    [void](Invoke-Helper $builtinFixture.Builtin)
    Assert-ArabicIsSecondOption $builtinFixture

    # Selecting ...\bin must also resolve to the installation root.
    $binFixture = New-LuantiFixture "Bin Folder Selection" "language (Language) enum   ,be,bg,en,tr" '        --ar = " [ar]", blacklisted'
    [void](Invoke-Helper $binFixture.Bin)
    Assert-ArabicIsSecondOption $binFixture

    # Repair the old malformed enum token "arbe" and keep an already-active label.
    $partialFixture = New-LuantiFixture "Repair Previous Partial Attempt" "language (Language) enum   ,arbe,bg,en,tr" '        ar = "العربية [ar]",'
    [void](Invoke-Helper $partialFixture.Root)
    Assert-ArabicIsSecondOption $partialFixture

    # Insert Arabic when the source table has no Arabic row at all.
    $missingFixture = New-LuantiFixture "Missing Arabic Row" "language (Language) enum   ,be,bg,en,tr" '        de = "Deutsch [de]",'
    [void](Invoke-Helper $missingFixture.Root)
    Assert-ArabicIsSecondOption $missingFixture

    # Idempotency: a second run must not add duplicate enum values or labels.
    [void](Invoke-Helper $rootFixture.Root)
    Assert-ArabicIsSecondOption $rootFixture

    Write-Host "PASS: Luanti 5.17.0 helper tests succeeded (root, builtin, bin, malformed enum repair, absent Arabic row, and idempotency)." -ForegroundColor Green
}
finally {
    if (Test-Path -LiteralPath $testRoot) {
        Remove-Item -LiteralPath $testRoot -Recurse -Force
    }
}
