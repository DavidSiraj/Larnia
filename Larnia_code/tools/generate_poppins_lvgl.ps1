$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$Project = Join-Path $Root "generated\Larnia_Figma_Final"
$Zip = Join-Path $Root "poppins.zip"
$Extract = Join-Path $Root "tools\poppins_src"

Write-Host ""
Write-Host "=== Larnia Poppins -> LVGL converter ==="
Write-Host "Project: $Project"
Write-Host ""

if (!(Test-Path $Project)) {
  throw "Nenasiel som generated\Larnia_Figma_Final. Najprv spusti prepare_windows.bat."
}

if (!(Test-Path $Zip)) {
  throw "Chyba poppins.zip. Skopiruj subor poppins.zip do hlavneho priecinka projektu vedla prepare_windows.bat."
}

$node = Get-Command node -ErrorAction SilentlyContinue
$npx = Get-Command npx -ErrorAction SilentlyContinue
if ($null -eq $node -or $null -eq $npx) {
  throw "Chyba Node.js/npx. Nainstaluj Node.js LTS, potom spusti tento skript znova."
}

if (Test-Path $Extract) {
  Remove-Item $Extract -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $Extract | Out-Null
Expand-Archive -Path $Zip -DestinationPath $Extract -Force

function Find-Font($Name, $Fallback1, $Fallback2) {
  $f = Get-ChildItem -Path $Extract -Recurse -Filter $Name | Select-Object -First 1
  if ($null -eq $f -and $Fallback1) {
    $f = Get-ChildItem -Path $Extract -Recurse -Filter $Fallback1 | Select-Object -First 1
  }
  if ($null -eq $f -and $Fallback2) {
    $f = Get-ChildItem -Path $Extract -Recurse -Filter $Fallback2 | Select-Object -First 1
  }
  if ($null -eq $f) {
    throw "Nenasiel som font $Name"
  }
  return $f.FullName
}

$Regular = Find-Font "Poppins-Regular.ttf" $null $null
$Medium = Find-Font "Poppins-Medium.ttf" "Poppins-Regular.ttf" $null
$SemiBold = Find-Font "Poppins-SemiBold.ttf" "Poppins-Medium.ttf" "Poppins-Regular.ttf"

Write-Host "Regular:  $Regular"
Write-Host "Medium:   $Medium"
Write-Host "SemiBold: $SemiBold"
Write-Host ""

# Latin + Latin Extended-A. Covers Slovak/Czech diacritics used by Larnia.
$Range = "0x20-0x7E,0xA0-0x17F"

function Convert-LvglFont($FontPath, $Size, $OutName) {
  $OutFile = Join-Path $Project ($OutName + ".c")
  Write-Host "Generating $OutName from $([System.IO.Path]::GetFileName($FontPath)) size $Size ..."

  & npx -y lv_font_conv --font $FontPath --range $Range --size $Size --bpp 4 --format lvgl --no-compress --lv-include "lvgl.h" -o $OutFile
  if ($LASTEXITCODE -ne 0) {
    throw "lv_font_conv zlyhal pre $OutName"
  }
  if (!(Test-Path $OutFile)) {
    throw "Nevznikol subor $OutFile"
  }

  # Make sure the exported lv_font_t symbol matches LarniaPoppinsFonts.h.
  $txt = Get-Content $OutFile -Raw
  $txt = $txt -replace '(?m)(const\s+)?lv_font_t\s+[A-Za-z_][A-Za-z0-9_]*\s*=', ('const lv_font_t ' + $OutName + ' =')
  $txt = $txt -replace '#include\s+"lvgl/lvgl.h"', '#include "lvgl.h"'
  Set-Content -Path $OutFile -Value $txt -Encoding UTF8
}

Convert-LvglFont $Regular 13 "lv_font_poppins_13"
Convert-LvglFont $Regular 16 "lv_font_poppins_16"
Convert-LvglFont $Medium 20 "lv_font_poppins_20"
Convert-LvglFont $SemiBold 24 "lv_font_poppins_24"
Convert-LvglFont $Medium 34 "lv_font_poppins_34"
Convert-LvglFont $SemiBold 36 "lv_font_poppins_36"
Convert-LvglFont $Medium 40 "lv_font_poppins_40"

Write-Host ""
Write-Host "HOTOVO: Poppins LVGL fonty su v generated\Larnia_Figma_Final"
Write-Host "Teraz otvor Arduino IDE, daj Verify a potom Upload."
Write-Host ""
