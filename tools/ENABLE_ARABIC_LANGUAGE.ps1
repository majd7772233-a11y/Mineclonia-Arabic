#requires -Version 5.1
[CmdletBinding()]
param(
    [string]$LuantiDir,
    [switch]$Elevated,
    [switch]$NonInteractive
)

$ErrorActionPreference = "Stop"

function Write-ExitPrompt {
    if (!$NonInteractive) {
        Read-Host "اضغط Enter للإغلاق" | Out-Null
    }
}

function Resolve-LuantiRoot([string]$SelectedPath) {
    $selected = (Resolve-Path -LiteralPath $SelectedPath).Path

    # The Luanti root contains builtin\settingtypes.txt.
    if (Test-Path -LiteralPath (Join-Path $selected "builtin\settingtypes.txt") -PathType Leaf) {
        return $selected
    }

    $item = Get-Item -LiteralPath $selected

    # If builtin itself was selected, move one level up to the installation root.
    if ($item.Name -ieq "builtin" -and
        (Test-Path -LiteralPath (Join-Path $selected "settingtypes.txt") -PathType Leaf)) {
        $parent = $item.Parent.FullName
        if ($parent -and (Test-Path -LiteralPath (Join-Path $parent "builtin\settingtypes.txt") -PathType Leaf)) {
            return $parent
        }
    }

    # Also accept the bin child when it belongs to a normal Luanti installation.
    if ($item.Name -ieq "bin" -and
        (Test-Path -LiteralPath (Join-Path $selected "luanti.exe") -PathType Leaf)) {
        $parent = $item.Parent.FullName
        if ($parent -and (Test-Path -LiteralPath (Join-Path $parent "builtin\settingtypes.txt") -PathType Leaf)) {
            return $parent
        }
    }

    return $selected
}
function Get-LuantiDirectory {
    if ($LuantiDir) {
        return Resolve-LuantiRoot $LuantiDir
    }

    Add-Type -AssemblyName System.Windows.Forms
    $picker = New-Object System.Windows.Forms.FolderBrowserDialog
    $picker.Description = "اختر جذر تثبيت Luanti الذي يحتوي على bin و builtin"
    $picker.ShowNewFolderButton = $false
    if ($picker.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) {
        throw "لم يتم اختيار مجلد Luanti."
    }
    return Resolve-LuantiRoot $picker.SelectedPath
}

function Is-Administrator {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($id)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Restart-Elevated([string]$Directory) {
    $argsLine = '-NoProfile -ExecutionPolicy Bypass -File "' + $PSCommandPath + '" -LuantiDir "' + $Directory + '" -Elevated'
    Start-Process -FilePath "powershell.exe" -Verb RunAs -ArgumentList $argsLine | Out-Null
}

function Get-LanguageEnumMatch([string]$Text) {
    $pattern = [regex]'(?m)^(?<prefix>[ \t]*language[ \t]+\(Language\)[ \t]+enum[ \t]+)(?<values>[^\r\n]*)$'
    return $pattern.Match($Text)
}

function Get-LanguageCodes([string]$Values) {
    return @(
        $Values.Split(",") |
            ForEach-Object { $_.Trim() } |
            Where-Object { $_ -ne "" }
    )
}

function Set-ArabicLanguageEnum([string]$Text) {
    $match = Get-LanguageEnumMatch $Text
    if (!$match.Success) {
        throw "تعريف language (Language) enum غير موجود في builtin\settingtypes.txt."
    }

    # Normalize instead of concatenating tokens; also repair the old malformed "arbe" token.
    $parts = @($match.Groups["values"].Value.Split(","))
    $otherCodes = New-Object 'System.Collections.Generic.List[string]'
    $seen = @{}

    foreach ($part in $parts) {
        $code = $part.Trim()
        if ($code -ceq "arbe") { $code = "be" }
        if ($code -eq "" -or $code -eq "ar") { continue }
        if (!$seen.ContainsKey($code)) {
            $seen[$code] = $true
            [void]$otherCodes.Add($code)
        }
    }

    # Keep the empty system-language option first, then Arabic exactly once.
    $newValues = ",ar"
    if ($otherCodes.Count -gt 0) {
        $newValues += "," + ($otherCodes -join ",")
    }

    if ($newValues -ceq $match.Groups["values"].Value) {
        return [pscustomobject]@{ Text = $Text; Changed = $false }
    }

    $pattern = [regex]'(?m)^(?<prefix>[ \t]*language[ \t]+\(Language\)[ \t]+enum[ \t]+)(?<values>[^\r\n]*)$'
    $newText = $pattern.Replace(
        $Text,
        [System.Text.RegularExpressions.MatchEvaluator]{
            param($m)
            return $m.Groups["prefix"].Value + $newValues
        },
        1
    )

    return [pscustomobject]@{ Text = $newText; Changed = $true }
}
function Set-ArabicLanguageLabel([string]$Text) {
    $desired = 'ar = "العربية [ar]",'
    $marker = 'get_setting_info("language").option_labels'
    $markerIndex = $Text.IndexOf($marker, [StringComparison]::Ordinal)
    if ($markerIndex -lt 0) {
        throw "لم أجد جدول option_labels الخاص بإعداد اللغة في builtin\common\settings\dlg_settings.lua."
    }

    $tableOpen = $Text.IndexOf("{", $markerIndex)
    if ($tableOpen -lt 0) {
        throw "لم أجد بداية جدول خيارات اللغة في dlg_settings.lua."
    }
    $tableClose = $Text.IndexOf("}", $tableOpen + 1)
    if ($tableClose -lt 0) {
        throw "جدول خيارات اللغة غير مكتمل في dlg_settings.lua."
    }

    $tableBody = $Text.Substring($tableOpen + 1, $tableClose - $tableOpen - 1)
    $tableLines = New-Object 'System.Collections.Generic.List[string]'
    $arabicIndent = $null

    # Avoid regex parsing differences between Windows PowerShell versions.
    # Remove any active or commented ar = ... row, remembering its indentation.
    foreach ($line in [regex]::Split($tableBody, "\r\n|\n")) {
        $trimmed = $line.TrimStart()
        $probe = $trimmed
        if ($probe.StartsWith("--", [StringComparison]::Ordinal)) {
            $probe = $probe.Substring(2).TrimStart()
        }

        $isArabicKey = $false
        if ($probe.StartsWith("ar", [StringComparison]::Ordinal) -and $probe.Length -gt 2) {
            $remainder = $probe.Substring(2).TrimStart()
            if ($remainder.StartsWith("=", [StringComparison]::Ordinal)) {
                $isArabicKey = $true
            }
        }

        if ($isArabicKey) {
            if ($null -eq $arabicIndent) {
                $arabicIndent = $line.Substring(0, $line.Length - $trimmed.Length)
            }
            continue
        }

        [void]$tableLines.Add($line)
    }

    # If there was no Arabic row, inherit the indentation from the system-language row.
    $anchorIndex = -1
    for ($i = 0; $i -lt $tableLines.Count; $i++) {
        if ($tableLines[$i].TrimStart().StartsWith('[""]', [StringComparison]::Ordinal)) {
            $anchorIndex = $i
            if ($null -eq $arabicIndent) {
                $trimmed = $tableLines[$i].TrimStart()
                $arabicIndent = $tableLines[$i].Substring(0, $tableLines[$i].Length - $trimmed.Length)
            }
            break
        }
    }

    if ($anchorIndex -lt 0) {
        for ($i = 0; $i -lt $tableLines.Count; $i++) {
            if ($tableLines[$i].TrimStart().StartsWith("en", [StringComparison]::Ordinal)) {
                $anchorIndex = $i
                if ($null -eq $arabicIndent) {
                    $trimmed = $tableLines[$i].TrimStart()
                    $arabicIndent = $tableLines[$i].Substring(0, $tableLines[$i].Length - $trimmed.Length)
                }
                break
            }
        }
    }

    if ($null -eq $arabicIndent) {
        $arabicIndent = ([string][char]9) + ([string][char]9)
    }
    if ($anchorIndex -ge 0) {
        $tableLines.Insert($anchorIndex + 1, $arabicIndent + $desired)
    } else {
        $insertAt = 0
        while ($insertAt -lt $tableLines.Count -and $tableLines[$insertAt].Trim() -eq "") {
            $insertAt++
        }
        $tableLines.Insert($insertAt, $arabicIndent + $desired)
    }

    $crlf = ([char]13).ToString() + ([char]10).ToString()
    $separator = if ($Text.Contains($crlf)) { $crlf } else { ([char]10).ToString() }
    $tableBody = [string]::Join($separator, $tableLines.ToArray())
    # Preserve the newline before the closing brace of the Lua table.
    if (!$tableBody.EndsWith($separator)) {
        $tableBody += $separator
    }

    $newText = $Text.Substring(0, $tableOpen + 1) + $tableBody + $Text.Substring($tableClose)
    return [pscustomobject]@{ Text = $newText; Changed = ($newText -cne $Text) }
}
try {
    Write-Host "تفعيل العربية في قائمة لغات Luanti 5.17.0" -ForegroundColor Cyan
    Write-Host "اختر جذر تثبيت Luanti الذي يحتوي على builtin و bin."
    Write-Host "يمكنك أيضاً اختيار مجلد builtin أو bin نفسه؛ سيُكتشف الجذر تلقائياً."

    $root = Get-LuantiDirectory
    $settingPath = Join-Path $root "builtin\settingtypes.txt"
    $dialogPath = Join-Path $root "builtin\common\settings\dlg_settings.lua"
    $exePath = Join-Path $root "bin\luanti.exe"

    if (!(Test-Path -LiteralPath $settingPath -PathType Leaf)) {
        throw "لم أجد builtin\settingtypes.txt. اختر C:\Users\Magd\AppData\Local\luanti\5.17.0 أو مجلد builtin بداخله."
    }
    if (!(Test-Path -LiteralPath $dialogPath -PathType Leaf)) {
        throw "لم أجد builtin\common\settings\dlg_settings.lua؛ تأكد من اختيار جذر تثبيت Luanti 5.17.0."
    }
    if (!(Test-Path -LiteralPath $exePath -PathType Leaf)) {
        Write-Warning "لم أجد bin\luanti.exe في الجذر المحدد؛ سأتابع لأن بعض الحزم توزع الملف التنفيذي في مسار مختلف."
    }
    if (Get-Process -Name "luanti","minetest" -ErrorAction SilentlyContinue) {
        throw "أغلق Luanti بالكامل قبل تشغيل الرقعة."
    }

    $settingText = [IO.File]::ReadAllText($settingPath)
    $dialogText = [IO.File]::ReadAllText($dialogPath)

    $enumResult = Set-ArabicLanguageEnum $settingText
    $labelResult = Set-ArabicLanguageLabel $dialogText
    $newSettingText = $enumResult.Text
    $newDialogText = $labelResult.Text
    $settingChanged = $enumResult.Changed
    $dialogChanged = $labelResult.Changed

    if (!$settingChanged -and !$dialogChanged) {
        Write-Host "العربية [ar] مفعّلة بالفعل وفي الموضع الصحيح." -ForegroundColor Green
        Write-Host "مجلد Luanti: $root"
        Write-Host "اختر Settings > Accessibility > Language > العربية [ar]."
        Write-ExitPrompt
        exit 0
    }

    $backupRoot = Join-Path $env:LOCALAPPDATA ("Mineclonia-Arabic\LuantiBackups\" + (Get-Date -Format "yyyyMMdd-HHmmss-fff"))
    New-Item -ItemType Directory -Path $backupRoot -Force | Out-Null
    $backupList = @()

    if ($settingChanged) {
        $backupTarget = Join-Path $backupRoot "builtin\settingtypes.txt"
        New-Item -ItemType Directory -Path (Split-Path $backupTarget -Parent) -Force | Out-Null
        Copy-Item -LiteralPath $settingPath -Destination $backupTarget
        $backupList += "builtin\settingtypes.txt"
    }
    if ($dialogChanged) {
        $backupTarget = Join-Path $backupRoot "builtin\common\settings\dlg_settings.lua"
        New-Item -ItemType Directory -Path (Split-Path $backupTarget -Parent) -Force | Out-Null
        Copy-Item -LiteralPath $dialogPath -Destination $backupTarget
        $backupList += "builtin\common\settings\dlg_settings.lua"
    }

    $backupInfo = [ordered]@{
        install_root = $root
        created_at = (Get-Date).ToString("o")
        backup_files = $backupList
    }
    $utf8 = [System.Text.UTF8Encoding]::new($false)
    [IO.File]::WriteAllText(
        (Join-Path $backupRoot "backup-manifest.json"),
        ($backupInfo | ConvertTo-Json -Depth 4),
        $utf8
    )

    try {
        if ($settingChanged) {
            [IO.File]::WriteAllText($settingPath, $newSettingText, $utf8)
        }
        if ($dialogChanged) {
            [IO.File]::WriteAllText($dialogPath, $newDialogText, $utf8)
        }
    }
    catch [System.UnauthorizedAccessException] {
        if (!$Elevated -and !(Is-Administrator)) {
            Write-Host "مجلد Luanti محمي. سأطلب صلاحية المسؤول لإكمال التعديل." -ForegroundColor Yellow
            Restart-Elevated $root
            Write-ExitPrompt
            exit 0
        }
        throw
    }

    # Re-read the on-disk files and verify both the order and visible label.
    $checkSetting = [IO.File]::ReadAllText($settingPath)
    $checkDialog = [IO.File]::ReadAllText($dialogPath)
    $checkEnum = Get-LanguageEnumMatch $checkSetting
    if (!$checkEnum.Success) {
        throw "فشل التحقق: تعريف language enum غير موجود بعد التعديل."
    }
    $checkParts = @($checkEnum.Groups["values"].Value.Split(","))
    $checkCodes = Get-LanguageCodes $checkEnum.Groups["values"].Value
    if ($checkParts.Count -lt 2 -or
        ![string]::IsNullOrWhiteSpace($checkParts[0]) -or
        $checkParts[1].Trim() -ne "ar" -or
        $checkCodes -notcontains "ar") {
        throw "فشل التحقق من ترتيب ar بعد خيار لغة النظام في enum اللغة."
    }
    if ($checkDialog.IndexOf('ar = "العربية [ar]",', [StringComparison]::Ordinal) -lt 0) {
        $debugArabicRows = @($checkDialog -split "\r?\n" | Where-Object { $_ -match '(^|--)ar[ \t]*=' })
        throw ("فشل التحقق من إضافة تسمية العربية إلى قائمة اللغة. الأسطر التي تبدأ بمفتاح ar: " + ($debugArabicRows -join " | "))
    }

    Write-Host ""
    Write-Host "نجح التعديل: أصبحت العربية [ar] متاحة في قائمة اللغة." -ForegroundColor Green
    Write-Host "مجلد Luanti المعتمد: $root"
    Write-Host "النسخ الاحتياطية محفوظة في: $backupRoot" -ForegroundColor Yellow
    Write-Host "افتح Luanti ثم Settings > Accessibility > Language واختر العربية [ar]."
    Write-Host "أعد تشغيل Luanti بعد الاختيار."
    Write-Host ""
    Write-Host "تنبيه: هذه الرقعة تُظهر خيار العربية فقط؛ لا تضيف تشكيل الحروف أو تخطيط RTL إلى محرك Luanti 5.17.0."
    Write-ExitPrompt
}
catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host "لم تُحذف ملفات اللعبة أو ملفات العالم."
    Write-ExitPrompt
    exit 1
}
