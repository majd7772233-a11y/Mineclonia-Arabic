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
    $item = Get-Item -LiteralPath $selected

    # Accept either the Luanti installation root or its builtin child.
    # If the user chose ...\5.17.0\builtin, normalize to ...\5.17.0.
    if ($item.Name -ieq "builtin" -and
        (Test-Path -LiteralPath (Join-Path $selected "settingtypes.txt") -PathType Leaf)) {
        $parent = $item.Parent.FullName
        if (Test-Path -LiteralPath (Join-Path $parent "builtin\settingtypes.txt") -PathType Leaf) {
            return $parent
        }
    }

    # Also accept the bin child as a convenience, although the dialog asks
    # for the installation root.
    if ($item.Name -ieq "bin" -and
        (Test-Path -LiteralPath (Join-Path $selected "luanti.exe") -PathType Leaf)) {
        $parent = $item.Parent.FullName
        if (Test-Path -LiteralPath (Join-Path $parent "builtin\settingtypes.txt") -PathType Leaf) {
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

    $values = $match.Groups["values"].Value
    $parts = @($values.Split(","))
    $leadingBlank = ""

    # The first empty enum item means “use system language”. Preserve it,
    # then put ar immediately after it. Also repairs the misplaced ar value
    # left by the previous helper version.
    if ($parts.Count -gt 0 -and [string]::IsNullOrWhiteSpace($parts[0])) {
        $leadingBlank = $parts[0]
        $startIndex = 1
    } else {
        # Preserve the engine's system-language option if an older package
        # or downstream build omitted its leading blank enum item.
        $startIndex = 0
    }

    $otherCodes = @()
    for ($index = $startIndex; $index -lt $parts.Count; $index++) {
        $code = $parts[$index].Trim()
        if ($code -ne "" -and $code -ne "ar") {
            $otherCodes += $code
        }
    }

    $newValues = $leadingBlank + ",ar"
    if ($otherCodes.Count -gt 0) {
        $newValues += "," + ($otherCodes -join ",")
    }

    if ($newValues -ceq $values) {
        return [pscustomobject]@{
            Text = $Text
            Changed = $false
        }
    }

    $replacement = $match.Groups["prefix"].Value + $newValues
    $pattern = [regex]'(?m)^(?<prefix>[ \t]*language[ \t]+\(Language\)[ \t]+enum[ \t]+)(?<values>[^\r\n]*)$'
    $newText = $pattern.Replace(
        $Text,
        [System.Text.RegularExpressions.MatchEvaluator]{
            param($m)
            return $m.Groups["prefix"].Value + $newValues
        },
        1
    )

    return [pscustomobject]@{
        Text = $newText
        Changed = $true
    }
}

function Set-ArabicLanguageLabel([string]$Text) {
    $desired = 'ar = "العربية [ar]",'
    $activeAny = [regex]'(?m)^(?<indent>[ \t]*)ar[ \t]*=[ \t]*"[^"]*"[ \t]*,[ \t]*$'
    $activeMatch = $activeAny.Match($Text)

    if ($activeMatch.Success) {
        if ($activeMatch.Value.Trim() -ceq $desired) {
            return [pscustomobject]@{ Text = $Text; Changed = $false }
        }
        $replacement = $activeMatch.Groups["indent"].Value + $desired
        $newText = $activeAny.Replace(
            $Text,
            [System.Text.RegularExpressions.MatchEvaluator]{ param($m) $replacement },
            1
        )
        return [pscustomobject]@{ Text = $newText; Changed = $true }
    }

    # Luanti 5.17.0 ships this language label commented out as blacklisted.
    $blocked = [regex]'(?m)^(?<indent>[ \t]*)--[ \t]*ar[ \t]*=[ \t]*"[^"]*"[ \t]*,[ \t]*blacklisted[^\r\n]*$'
    $blockedMatch = $blocked.Match($Text)
    if ($blockedMatch.Success) {
        $replacement = $blockedMatch.Groups["indent"].Value + $desired
        $newText = $blocked.Replace(
            $Text,
            [System.Text.RegularExpressions.MatchEvaluator]{ param($m) $replacement },
            1
        )
        return [pscustomobject]@{ Text = $newText; Changed = $true }
    }

    # If the old comment format differs, insert the visible option after English.
    $english = [regex]'(?m)^(?<indent>[ \t]*)en[ \t]*=[ \t]*"English \[en\]",[ \t]*$'
    $englishMatch = $english.Match($Text)
    if (!$englishMatch.Success) {
        throw "لم أجد سطر English داخل قائمة اللغة في builtin\common\settings\dlg_settings.lua."
    }
    $replacement = $englishMatch.Value + [Environment]::NewLine +
        $englishMatch.Groups["indent"].Value + $desired
    $newText = $english.Replace($Text, $replacement, 1)

    return [pscustomobject]@{ Text = $newText; Changed = $true }
}

try {
    Write-Host "تفعيل العربية في قائمة لغات Luanti 5.17.0" -ForegroundColor Cyan
    Write-Host "اختر جذر Luanti الذي يحتوي على bin و builtin."
    Write-Host "يمكنك أيضاً اختيار مجلد builtin نفسه؛ سيُكتشف الجذر تلقائياً."

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
    if ($checkDialog -notmatch '(?m)^[ \t]*ar[ \t]*=[ \t]*"العربية \[ar\]"[ \t]*,[ \t]*$') {
        throw "فشل التحقق من إضافة تسمية العربية إلى قائمة اللغة."
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
