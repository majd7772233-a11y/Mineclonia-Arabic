#requires -Version 5.1
[CmdletBinding()]
param([string]$LuantiDir,[switch]$Elevated)

$ErrorActionPreference = "Stop"

function Get-LuantiDirectory {
    if ($LuantiDir) { return (Resolve-Path -LiteralPath $LuantiDir).Path }
    Add-Type -AssemblyName System.Windows.Forms
    $picker = New-Object System.Windows.Forms.FolderBrowserDialog
    $picker.Description = "اختر مجلد تثبيت Luanti الذي يحتوي على مجلد builtin"
    $picker.ShowNewFolderButton = $false
    if ($picker.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) {
        throw "لم يتم اختيار مجلد Luanti."
    }
    return (Resolve-Path -LiteralPath $picker.SelectedPath).Path
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

try {
    Write-Host "تفعيل العربية في قائمة لغات Luanti 5.17.0" -ForegroundColor Cyan
    Write-Host "اختر مجلد Luanti الذي يحتوي على builtin؛ لا تختر مجلد Mineclonia."
    $root = Get-LuantiDirectory
    $settingPath = Join-Path $root "builtin\settingtypes.txt"
    $dialogPath = Join-Path $root "builtin\common\settings\dlg_settings.lua"

    if (!(Test-Path -LiteralPath $settingPath -PathType Leaf)) {
        throw "لم أجد builtin\settingtypes.txt في المجلد المحدد."
    }
    if (!(Test-Path -LiteralPath $dialogPath -PathType Leaf)) {
        throw "لم أجد builtin\common\settings\dlg_settings.lua؛ هذه الرقعة مخصصة لبنية Luanti 5.17.0."
    }
    if (Get-Process -Name "luanti","minetest" -ErrorAction SilentlyContinue) {
        throw "أغلق Luanti بالكامل قبل تشغيل الرقعة."
    }

    $settingText = [IO.File]::ReadAllText($settingPath)
    $dialogText = [IO.File]::ReadAllText($dialogPath)
    $newSettingText = $settingText
    $newDialogText = $dialogText
    $settingChanged = $false
    $dialogChanged = $false

    # The first empty enum entry means "use system language". Insert ar after it.
    $enumPattern = [regex]'(?m)^(?<prefix>\s*language\s+\(Language\)\s+enum\s+)(?<values>[^\r\n]*)$'
    $enumMatch = $enumPattern.Match($newSettingText)
    if (!$enumMatch.Success) { throw "تعريف language enum غير موجود في settingtypes.txt." }
    $values = $enumMatch.Groups["values"].Value
    $codes = @($values.Split(",") | Where-Object { $_ -ne "" })
    if ($codes -notcontains "ar") {
        if ($values.StartsWith(",")) { $values = ",ar" + $values.Substring(1) }
        elseif ([string]::IsNullOrEmpty($values)) { $values = "ar" }
        else { $values = "ar," + $values }
        $replacement = $enumMatch.Groups["prefix"].Value + $values
        $newSettingText = $enumPattern.Replace(
            $newSettingText,
            [System.Text.RegularExpressions.MatchEvaluator]{ param($m) $replacement },
            1
        )
        $settingChanged = $true
    }

    # Luanti 5.17.0 has this entry commented out as "blacklisted".
    $activeArabic = [regex]'(?m)^\s*ar\s*=\s*"العربية \[ar\]"\s*,?\s*$'
    if (!$activeArabic.IsMatch($newDialogText)) {
        $blocked = [regex]'(?m)^(?<indent>\s*)--\s*ar\s*=\s*"[^"]*"\s*,\s*blacklisted.*$'
        $blockedMatch = $blocked.Match($newDialogText)
        if ($blockedMatch.Success) {
            $replacement = $blockedMatch.Groups["indent"].Value + 'ar = "العربية [ar]",'
            $newDialogText = $blocked.Replace(
                $newDialogText,
                [System.Text.RegularExpressions.MatchEvaluator]{ param($m) $replacement },
                1
            )
        } else {
            $english = [regex]'(?m)^(?<indent>\s*)en\s*=\s*"English \[en\]",\s*$'
            $englishMatch = $english.Match($newDialogText)
            if (!$englishMatch.Success) { throw "لم أجد سطر English داخل قائمة اللغة في dlg_settings.lua." }
            $replacement = $englishMatch.Value + [Environment]::NewLine +
                $englishMatch.Groups["indent"].Value + 'ar = "العربية [ar]",'
            $newDialogText = $english.Replace($newDialogText, $replacement, 1)
        }
        $dialogChanged = $true
    }

    if (!$settingChanged -and !$dialogChanged) {
        Write-Host "العربية مفعّلة بالفعل في ملفات Luanti." -ForegroundColor Green
        Write-Host "اختر Settings > Accessibility > Language > العربية [ar]."
        Read-Host "اضغط Enter للإغلاق"
        exit 0
    }

    $backupRoot = Join-Path $env:LOCALAPPDATA ("Mineclonia-Arabic\LuantiBackups\" + (Get-Date -Format "yyyyMMdd-HHmmss"))
    New-Item -ItemType Directory -Path $backupRoot -Force | Out-Null
    $backupList = @()
    if ($settingChanged) {
        $target = Join-Path $backupRoot "builtin\settingtypes.txt"
        New-Item -ItemType Directory -Path (Split-Path $target -Parent) -Force | Out-Null
        Copy-Item -LiteralPath $settingPath -Destination $target
        $backupList += "builtin\settingtypes.txt"
    }
    if ($dialogChanged) {
        $target = Join-Path $backupRoot "builtin\common\settings\dlg_settings.lua"
        New-Item -ItemType Directory -Path (Split-Path $target -Parent) -Force | Out-Null
        Copy-Item -LiteralPath $dialogPath -Destination $target
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
        if ($settingChanged) { [IO.File]::WriteAllText($settingPath, $newSettingText, $utf8) }
        if ($dialogChanged) { [IO.File]::WriteAllText($dialogPath, $newDialogText, $utf8) }
    }
    catch [System.UnauthorizedAccessException] {
        if (!$Elevated -and !(Is-Administrator)) {
            Write-Host "مجلد Luanti محمي. سأطلب صلاحية المسؤول لإكمال التعديل." -ForegroundColor Yellow
            Restart-Elevated $root
            exit 0
        }
        throw
    }

    $checkSetting = [IO.File]::ReadAllText($settingPath)
    $checkDialog = [IO.File]::ReadAllText($dialogPath)
    if ($checkSetting -notmatch '(?m)^language\s+\(Language\)\s+enum\s+[^\r\n]*,ar(,|$)') {
        throw "فشل التحقق من إضافة ar إلى enum اللغة."
    }
    if ($checkDialog -notmatch '(?m)^\s*ar\s*=\s*"العربية \[ar\]"') {
        throw "فشل التحقق من إضافة تسمية العربية إلى قائمة اللغة."
    }

    Write-Host ""
    Write-Host "نجح التعديل: أصبحت العربية [ar] متاحة في قائمة اللغة." -ForegroundColor Green
    Write-Host "النسخ الاحتياطية محفوظة في: $backupRoot" -ForegroundColor Yellow
    Write-Host "الآن افتح Luanti ثم Settings > Accessibility > Language واختر العربية [ar]."
    Write-Host "أعد تشغيل Luanti بعد الاختيار."
    Write-Host ""
    Write-Host "تنبيه: Luanti 5.17.0 يحجب العربية أصلاً بسبب محدودية دعم RTL. هذه الرقعة تُظهر الخيار فقط ولا تضيف تشكيل الحروف أو تخطيط RTL لقوائم المحرك."
    Read-Host "اضغط Enter للإغلاق"
}
catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host "لم تُحذف ملفات اللعبة أو ملفات العالم."
    Read-Host "اضغط Enter للإغلاق"
    exit 1
}
