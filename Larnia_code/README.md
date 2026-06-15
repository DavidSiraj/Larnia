# Larnia FINAL v5

Finálny balík pre CrowPanel Advanced 7" ESP32-P4 HMI AI Display 1024x600.

## Dôležité Arduino nastavenia
- Board: ESP32P4 Dev Module
- Port: COM port CrowPanelu
- Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)
- PSRAM: Enabled
- LVGL: 8.3.11
- Upload: UART0 / Hardware CDC

## Postup
1. Rozbaľ projekt do čistého priečinka.
2. Do hlavného priečinka projektu vlož svoj `poppins.zip`.
3. Spusti `prepare_windows.bat`.
4. Spusti `generate_poppins_fonts.bat`.
5. Skopíruj OBSAH priečinka `sdcard` na SD kartu. Na karte má byť priamo `ui`, `avatars`, `config`, `data`, `materials`.
6. Otvor `generated/Larnia_Figma_Final/Larnia_Figma_Final.ino` v Arduino IDE.
7. Nastav Huge APP a PSRAM Enabled.
8. Verify.
9. Upload.
10. Vlož SD kartu a resetni panel.

## Čo je vo finále
- Poppins LVGL fonty so slovenskou diakritikou po spustení generátora.
- Reálne Figma assety pre logo, ikony a pozadia.
- Splash, profily, učitelia, detail učiteľa, testy, chat, materiály, nastavenia Avatar/AI/Generálne.
- Funkčné textové polia cez LVGL textarea a klávesnicu.
- Funkčné vytvorenie profilu, učiteľa, URL/materialu a mazanie posledných položiek.
- Šípka späť na podstránkach.

OpenAI API kľúč patrí len do backendu, nie do ESP firmvéru.
