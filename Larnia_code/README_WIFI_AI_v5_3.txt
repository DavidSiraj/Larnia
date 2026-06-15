Larnia v5.3 – Wi‑Fi, nastavenia a AI hotfix

Opravené:
- Nastavenia -> Generálne ukladajú Wi‑Fi SSID, heslo, Backend IP a port do SD karty.
- Po uložení sa firmware pokúsi znovu pripojiť na Wi‑Fi a backend.
- Firmware pri štarte číta Wi‑Fi/backend zo SD karty z /config/settings.json.
- Chat z panelu posiela otázku backendu cez WebSocket.
- Backend prijíma správu chat-question z CrowPanelu a vracia odpoveď späť na panel.

Dôležité:
- ESP32-P4 nemá Wi‑Fi priamo v čipe. CrowPanel používa ESP32-C6 ako Wi‑Fi/Bluetooth koprocesor.
- Ak Arduino core / board package nespustí C6 Wi‑Fi, samotné vyplnenie SSID nepomôže. Vtedy treba použiť správny Elecrow P4 Wi‑Fi príklad alebo ich ESP-Hosted konfiguráciu.
- AI funguje iba keď beží backend a panel sa pripojí na jeho IP adresu.

Postup:
1. Nahraj firmware.
2. Na paneli otvor Nastavenia -> Generálne.
3. Vyplň:
   Wi‑Fi SSID = názov tvojej siete
   Heslo = heslo siete
   Backend IP = IP počítača, kde beží backend
   Port = 3000
4. Stlač Uložiť a pripojiť.
5. Backend spusti v priečinku backend:
   npm install
   npm run dev

Pre skúšku bez OpenAI:
- v backend/.env nechaj MOCK_AI=true.
- panel dostane testovaciu odpoveď bez plateného API.

Pre reálnu AI:
- v backend/.env nastav OPENAI_API_KEY.
- nastav MOCK_AI=false.
