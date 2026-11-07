# Test the Luanti 5.17.0 language patch with the actual builtin layout
# and the Windows user-config lookup used by the engine.
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
$lineBreak = [Environment]::NewLine

function New-LuantiFixture([string]$Name, [string]$EnumLine, [string]$ArabicEntry, [string]$StoredLanguage) {
    $root = Join-Path $testRoot $Name
    $builtin = Join-Path $root "builtin"
    $settings = Join-Path $builtin "common\settings"
    $bin = Join-Path $root "bin"
    $userPath = Join-Path $root "CustomUserData"
    $appData = Join-Path $root "Roaming"
    $localePackageRoot = Join-Path $testRoot "PackageAssets"
    $localeCatalogSource = Join-Path $localePackageRoot "luanti-locale\ar\LC_MESSAGES\luanti.mo"
    $localeCatalogDestination = Join-Path $root "locale\ar\LC_MESSAGES\luanti.mo"

    New-Item -ItemType Directory -Path $settings -Force | Out-Null
    New-Item -ItemType Directory -Path $bin -Force | Out-Null
    New-Item -ItemType Directory -Path $userPath -Force | Out-Null
    New-Item -ItemType Directory -Path $appData -Force | Out-Null
    New-Item -ItemType Directory -Path (Split-Path -Parent $localeCatalogSource) -Force | Out-Null
    if (!(Test-Path -LiteralPath $localeCatalogSource)) {
        # Valid GNU MO byte-order magic followed by fixture payload.
        [IO.File]::WriteAllBytes($localeCatalogSource, [byte[]]@(0xDE, 0x12, 0x04, 0x95, 0x01, 0x00))
    }
    $defaultUserPath = Join-Path $appData "Minetest"
    New-Item -ItemType Directory -Path $defaultUserPath -Force | Out-Null
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

    $configLines = @("# Preserve unrelated settings")
    if ($StoredLanguage -ne "__MISSING__") {
        $configLines += "language = $StoredLanguage"
    }
    $configLines += "fullscreen = true"
    $configText = [string]::Join($lineBreak, $configLines) + $lineBreak
    $configFile = Join-Path $userPath "minetest.conf"
    $defaultConfig = Join-Path $defaultUserPath "minetest.conf"
    [IO.File]::WriteAllText($configFile, $configText, [Text.UTF8Encoding]::new($false))
    [IO.File]::WriteAllText($defaultConfig, $configText, [Text.UTF8Encoding]::new($false))

    return [pscustomobject]@{
        Root = $root
        Builtin = $builtin
        Bin = $bin
        UserPath = $userPath
        AppData = $appData
        ConfigFile = $configFile
        LocaleCatalogSource = $localeCatalogSource
        LocaleCatalogDestination = $localeCatalogDestination
        SettingPath = Join-Path $builtin "settingtypes.txt"
        DialogPath = Join-Path $settings "dlg_settings.lua"
    }
}

function Invoke-Helper($Fixture, [string]$SelectedPath, [ValidateSet("env", "appdata", "run-in-place")][string]$ConfigMode) {
    $oldLuantiUser = [Environment]::GetEnvironmentVariable("LUANTI_USER_PATH", "Process")
    $oldMinetestUser = [Environment]::GetEnvironmentVariable("MINETEST_USER_PATH", "Process")
    $oldAppData = [Environment]::GetEnvironmentVariable("APPDATA", "Process")
    try {
        [Environment]::SetEnvironmentVariable("APPDATA", $Fixture.AppData, "Process")
        [Environment]::SetEnvironmentVariable("MINETEST_USER_PATH", $null, "Process")
        if ($ConfigMode -eq "env") {
            [Environment]::SetEnvironmentVariable("LUANTI_USER_PATH", $Fixture.UserPath, "Process")
        } elseif ($ConfigMode -eq "run-in-place") {
            # Luanti's RUN_IN_PLACE branch bypasses setSystemPaths(), so the
            # install-root minetest.conf must win even when this variable is set.
            [Environment]::SetEnvironmentVariable("LUANTI_USER_PATH", $Fixture.UserPath, "Process")
        } else {
            [Environment]::SetEnvironmentVariable("LUANTI_USER_PATH", $null, "Process")
        }

        $output = & $powerShell -NoProfile -ExecutionPolicy Bypass -File $helper -LuantiDir $SelectedPath -LocaleCatalog $Fixture.LocaleCatalogSource -NonInteractive 2>&1
        $exitCode = $LASTEXITCODE
        if ($exitCode -ne 0) {
            $detail = $output -join [Environment]::NewLine
            throw ("Helper failed for '{0}' (exit {1}):{2}{3}" -f $SelectedPath, $exitCode, [Environment]::NewLine, $detail)
        }
        return ($output -join [Environment]::NewLine)
    }
    finally {
        [Environment]::SetEnvironmentVariable("LUANTI_USER_PATH", $oldLuantiUser, "Process")
        [Environment]::SetEnvironmentVariable("MINETEST_USER_PATH", $oldMinetestUser, "Process")
        [Environment]::SetEnvironmentVariable("APPDATA", $oldAppData, "Process")
    }
}

function Assert-ArabicIsInstalled($Fixture, [string]$ConfigMode) {
    $line = [IO.File]::ReadAllLines($Fixture.SettingPath) |
        Where-Object { $_ -match '^[ \t]*language[ \t]+\(Language\)[ \t]+enum[ \t]+' } |
        Select-Object -First 1
    if (!$line) { throw "Language enum was not found in $($Fixture.SettingPath)" }

    $values = [regex]::Replace($line, '^[ \t]*language[ \t]+\(Language\)[ \t]+enum[ \t]+', '')
    $parts = @($values.Split(","))
    $arabicCount = @($parts | Where-Object { $_.Trim() -eq "ar" }).Count
    $belarusianCount = @($parts | Where-Object { $_.Trim() -eq "be" }).Count
    $malformedCount = @($parts | Where-Object { $_.Trim() -eq "arbe" }).Count
    if ($arabicCount -ne 1 -or $belarusianCount -ne 1 -or
        $malformedCount -ne 0 -or $parts.Count -lt 2 -or
        -not [string]::IsNullOrWhiteSpace($parts[0]) -or
        $parts[1].Trim() -ne "ar") {
        throw "Expected empty system-language first and separate ar/be values, got: $values"
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
    $malformedRows = @($dialog -split "\r?\n" | Where-Object { $_ -match '^[ \t]*(?:--[ \t]*)?arbe[ \t]*=' })
    if ($malformedRows.Count -gt 0) {
        throw "Malformed 'arbe' language label remains: $($malformedRows -join ' | ')"
    }

    if ($ConfigMode -eq "env") {
        $configPath = $Fixture.ConfigFile
    } elseif ($ConfigMode -eq "run-in-place") {
        $configPath = $Fixture.InPlaceConfig
    } else {
        $configPath = Join-Path $Fixture.AppData "Minetest\minetest.conf"
    }
    $config = [IO.File]::ReadAllText($configPath)
    $languageRows = @(
        $config -split "\r?\n" |
            Where-Object { $_ -match '^[ \t]*language[ \t]*=' }
    )
    if ($languageRows.Count -ne 1 -or $languageRows[0] -notmatch '^[ \t]*language[ \t]*=[ \t]*ar[ \t]*$') {
        throw "Expected exactly one persisted language = ar setting in $configPath, got: $($languageRows -join ' | ')"
    }
    if (!$config.Contains("fullscreen = true")) {
        throw "The helper did not preserve unrelated settings in $configPath. Contents: $config"
    }

    if (!(Test-Path -LiteralPath $Fixture.LocaleCatalogDestination -PathType Leaf)) {
        throw "Luanti language-tag catalog was not installed at $($Fixture.LocaleCatalogDestination)"
    }
    $sourceCatalog = [Convert]::ToBase64String([IO.File]::ReadAllBytes($Fixture.LocaleCatalogSource))
    $installedCatalog = [Convert]::ToBase64String([IO.File]::ReadAllBytes($Fixture.LocaleCatalogDestination))
    if ($sourceCatalog -cne $installedCatalog) {
        throw "Installed Luanti language-tag catalog differs from the package catalog."
    }
}

try {
    New-Item -ItemType Directory -Path $testRoot -Force | Out-Null

    # Repair the exact malformed enum token and a stale persisted config value.
    $rootFixture = New-LuantiFixture "Install Root With Spaces" "language (Language) enum   ,arbe,bg,be,en,tr" '        --ar = " [ar]", blacklisted' "arbe"
    [void](Invoke-Helper $rootFixture $rootFixture.Root "env")
    Assert-ArabicIsInstalled $rootFixture "env"

    # Selecting ...\builtin resolves to its parent; APPDATA\Minetest is used.
    $builtinFixture = New-LuantiFixture "Builtin Folder Selection" "language (Language) enum   ,be,bg,en,tr" '        --ar = " [ar]", blacklisted' "en"
    [void](Invoke-Helper $builtinFixture $builtinFixture.Builtin "appdata")
    Assert-ArabicIsInstalled $builtinFixture "appdata"

    # Selecting ...\bin resolves to its parent; a missing language value is added.
    $binFixture = New-LuantiFixture "Bin Folder Selection" "language (Language) enum   ,be,bg,en,tr" '        --ar = " [ar]", blacklisted' "__MISSING__"
    $binConfig = [IO.File]::ReadAllText($binFixture.ConfigFile)
    $binConfig = $binConfig -replace '(?m)^language = .*(?:\r?\n)?', ''
    [IO.File]::WriteAllText($binFixture.ConfigFile, $binConfig, [Text.UTF8Encoding]::new($false))
    [void](Invoke-Helper $binFixture $binFixture.Bin "env")
    Assert-ArabicIsInstalled $binFixture "env"

    # Remove an existing malformed label key as well as the enum token.
    $partialFixture = New-LuantiFixture "Repair Previous Partial Attempt" "language (Language) enum   ,arbe,bg,en,tr" '        arbe = "Broken Arabic label",' "arbe"
    [void](Invoke-Helper $partialFixture $partialFixture.Root "env")
    Assert-ArabicIsInstalled $partialFixture "env"

    # Detect RUN_IN_PLACE on Windows: the user config belongs beside minetest.conf.example.
    $portableFixture = New-LuantiFixture "Portable RUN_IN_PLACE" "language (Language) enum   ,be,bg,en,tr" '        --ar = " [ar]", blacklisted' "arbe"
    [IO.File]::WriteAllText((Join-Path $portableFixture.Root "minetest.conf.example"), "# default config marker", [Text.UTF8Encoding]::new($false))
    $portableConfig = Join-Path $portableFixture.Root "minetest.conf"
    Copy-Item -LiteralPath $portableFixture.ConfigFile -Destination $portableConfig
    $portableFixture | Add-Member -NotePropertyName InPlaceConfig -NotePropertyValue $portableConfig
    [void](Invoke-Helper $portableFixture $portableFixture.Root "run-in-place")
    Assert-ArabicIsInstalled $portableFixture "run-in-place"

    # RUN_IN_PLACE must beat both APPDATA and LUANTI_USER_PATH: the engine itself
    # ignores those system-path overrides in this compile-time mode.
    $untouchedAppDataConfig = [IO.File]::ReadAllText((Join-Path $portableFixture.AppData "Minetest\minetest.conf"))
    if (!$untouchedAppDataConfig.Contains("language = arbe")) {
        throw "RUN_IN_PLACE detection incorrectly modified the APPDATA config instead of the install-root config."
    }
    $untouchedOverrideConfig = [IO.File]::ReadAllText($portableFixture.ConfigFile)
    if (!$untouchedOverrideConfig.Contains("language = arbe")) {
        throw "RUN_IN_PLACE detection incorrectly honored LUANTI_USER_PATH even though Luanti bypasses it in this mode."
    }

    # Insert Arabic when the option-label table has no Arabic row.
    $missingFixture = New-LuantiFixture "Missing Arabic Row" "language (Language) enum   ,be,bg,en,tr" '        de = "Deutsch [de]",' "en"
    [void](Invoke-Helper $missingFixture $missingFixture.Root "appdata")
    Assert-ArabicIsInstalled $missingFixture "appdata"

    # Idempotency: a second run must not duplicate labels, enum values, or config keys.
    [void](Invoke-Helper $rootFixture $rootFixture.Root "env")
    Assert-ArabicIsInstalled $rootFixture "env"

    Write-Host "PASS: Windows PowerShell 5.1 tests succeeded (root/builtin/bin selection, malformed arbe repair, RUN_IN_PLACE config, MO language-code bridge installation, preservation, and idempotency)." -ForegroundColor Green
}
finally {
    if (Test-Path -LiteralPath $testRoot) {
        Remove-Item -LiteralPath $testRoot -Recurse -Force
    }
}
