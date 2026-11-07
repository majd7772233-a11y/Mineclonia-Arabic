# Luanti 5.17.0 - enable Arabic glyph coverage on Windows
# This changes font paths only. It does not modify worlds, games, mods, or translations.

$ErrorActionPreference = "Stop"

function Find-LuantiRoot {
    $current = Get-Item -LiteralPath $PSScriptRoot
    while ($null -ne $current) {
        $exe = Join-Path $current.FullName "bin\luanti.exe"
        $example = Join-Path $current.FullName "minetest.conf.example"
        if ((Test-Path -LiteralPath $exe -PathType Leaf) -and
            (Test-Path -LiteralPath $example -PathType Leaf)) {
            return $current.FullName
        }
        $current = $current.Parent
    }
    throw "Luanti portable root was not found. Place this script beside the bin folder and minetest.conf.example."
}

function Set-ConfigValue {
    param(
        [System.Collections.Generic.List[string]]$Lines,
        [string]$Name,
        [string]$Value
    )

    $pattern = "^\s*" + [Regex]::Escape($Name) + "\s*="
    for ($i = $Lines.Count - 1; $i -ge 0; $i--) {
        if ($Lines[$i] -match $pattern) {
            $Lines.RemoveAt($i)
        }
    }
    $Lines.Add("$Name = $Value")
}

try {
    $root = Find-LuantiRoot
    $configPath = Join-Path $root "minetest.conf"
    $fontsDir = Join-Path $env:WINDIR "Fonts"

    # Tahoma is preferred because it has broad Arabic glyph coverage.
    # Segoe UI and Arial are fallbacks for customized Windows installations.
    $candidates = @(
        [pscustomobject]@{
            Name = "Tahoma"
            Regular = "tahoma.ttf"
            Bold = "tahomabd.ttf"
            Italic = "tahoma.ttf"
            BoldItalic = "tahomabd.ttf"
        },
        [pscustomobject]@{
            Name = "Segoe UI"
            Regular = "segoeui.ttf"
            Bold = "segoeuib.ttf"
            Italic = "segoeuii.ttf"
            BoldItalic = "segoeuiz.ttf"
        },
        [pscustomobject]@{
            Name = "Arial"
            Regular = "arial.ttf"
            Bold = "arialbd.ttf"
            Italic = "ariali.ttf"
            BoldItalic = "arialbi.ttf"
        }
    )

    $chosen = $null
    foreach ($candidate in $candidates) {
        $regularPath = Join-Path $fontsDir $candidate.Regular
        if (Test-Path -LiteralPath $regularPath -PathType Leaf) {
            $chosen = $candidate
            break
        }
    }

    if ($null -eq $chosen) {
        throw "Tahoma, Segoe UI, or Arial was not found in the Windows Fonts directory: $fontsDir"
    }

    $resolveFont = {
        param([string]$FileName, [string]$Fallback)
        $path = Join-Path $fontsDir $FileName
        if (Test-Path -LiteralPath $path -PathType Leaf) {
            return $path
        }
        return (Join-Path $fontsDir $Fallback)
    }

    $regular = Join-Path $fontsDir $chosen.Regular
    $bold = & $resolveFont $chosen.Bold $chosen.Regular
    $italic = & $resolveFont $chosen.Italic $chosen.Regular
    $boldItalic = & $resolveFont $chosen.BoldItalic $chosen.Bold

    if (Test-Path -LiteralPath $configPath -PathType Leaf) {
        $stamp = Get-Date -Format "yyyyMMdd-HHmmss"
        $backupPath = "$configPath.backup-$stamp"
        Copy-Item -LiteralPath $configPath -Destination $backupPath
    }

    $lines = New-Object 'System.Collections.Generic.List[string]'
    if (Test-Path -LiteralPath $configPath -PathType Leaf) {
        foreach ($line in [System.IO.File]::ReadAllLines($configPath)) {
            $lines.Add($line)
        }
    }

    Set-ConfigValue -Lines $lines -Name "language" -Value "ar"

    # Use an installed Arabic-capable Windows font as the main face.
    # Keep fallback_font_path unchanged so Luanti retains its CJK fallback.
    Set-ConfigValue -Lines $lines -Name "font_path" -Value $regular
    Set-ConfigValue -Lines $lines -Name "font_path_bold" -Value $bold
    Set-ConfigValue -Lines $lines -Name "font_path_italic" -Value $italic
    Set-ConfigValue -Lines $lines -Name "font_path_bold_italic" -Value $boldItalic

    # Some GUI and console elements request the monospaced font mode.
    # Use the same Arabic-capable family there so those strings are not blank.
    Set-ConfigValue -Lines $lines -Name "mono_font_path" -Value $regular
    Set-ConfigValue -Lines $lines -Name "mono_font_path_bold" -Value $bold
    Set-ConfigValue -Lines $lines -Name "mono_font_path_italic" -Value $italic
    Set-ConfigValue -Lines $lines -Name "mono_font_path_bold_italic" -Value $boldItalic

    $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllLines($configPath, $lines.ToArray(), $utf8NoBom)

    Write-Host ""
    Write-Host "Luanti font settings were updated successfully." -ForegroundColor Green
    Write-Host "Luanti root: $root"
    Write-Host "Selected font: $($chosen.Name)"
    Write-Host "The original fallback font was preserved. Worlds and Mineclonia files were not changed."
    Write-Host ""
    if (Get-Process -Name "luanti" -ErrorAction SilentlyContinue) {
        Write-Host "Close Luanti completely, then restart it to load the new fonts." -ForegroundColor Yellow
    } else {
        Write-Host "Start Luanti and open your Mineclonia world." -ForegroundColor Green
    }
    Write-Host ""
    Write-Host "Note: this fixes missing Arabic glyph coverage only. Luanti 5.17.0 still lacks full Arabic shaping and RTL layout, so letters may remain disconnected or appear in the wrong order."
}
catch {
    Write-Host ""
    Write-Host "[ERROR] $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
