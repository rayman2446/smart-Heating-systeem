# Slimme Klimaatregeling per Ruimte

## 1. Korte omschrijving

**Slimme Klimaatregeling per Ruimte** is een IoT-schoolproject dat de temperatuur en aanwezigheid in een ruimte meet en deze informatie gebruikt om de verwarming lokaal aan te sturen. Een ESP32 verzamelt sensorgegevens en communiceert via MQTT met Home Assistant. De verwarming wordt alleen ingeschakeld wanneer er aanwezigheid wordt gedetecteerd én de gemeten temperatuur onder de ingestelde grens ligt.

## 2. Probleemstelling en doelgroep

Veel gebouwen worden centraal verwarmd. Daardoor worden ook lege kamers verwarmd, wat kan leiden tot onnodig energieverbruik. Dit project onderzoekt een lokale regeling per ruimte: de temperatuur en aanwezigheid (beweging) worden gemeten, waarna de verwarming uitsluitend wordt ingeschakeld als er iemand aanwezig is en de temperatuur te laag is.

De beoogde doelgroep bestaat uit **particuliere huishoudens en kleine kantoren met een bestaande centrale verwarming**. De aanpak maakt het mogelijk de warmtevraag per ruimte af te stemmen op het daadwerkelijke gebruik.

## 3. Gebruikte hardware

| Component | Model/type | Functie |
|---|---|---|
| Edge device | ESP32 Development Board (ESP32-WROOM-32E) | Leest sensoren uit, communiceert met de MQTT-broker en stuurt de actuator aan. |
| Temperatuursensor | DHT22 | Meet de temperatuur in de ruimte. |
| Aanwezigheidssensor | HC-SR501 PIR | Detecteert beweging als indicatie van aanwezigheid. |
| Actuator | 1-kanaals 5V-relaismodule | Schakelt de verwarmingsregeling op basis van het systeemcommando. |
| Centrale server | Raspberry Pi 4 | Draait Home Assistant en de Mosquitto MQTT-broker lokaal. |

## 4. Software, protocollen en architectuur

De IoT-communicatie verloopt via **MQTT over WiFi**. MQTT is lichtgewicht en snel en is daardoor geschikt voor het uitwisselen van meetwaarden en besturingsberichten tussen het edge device en de centrale server.

De ESP32-code is geschreven in **C++ met de Arduino IDE**. De backend en het dashboard draaien lokaal in **Home Assistant** op de Raspberry Pi 4. De **Mosquitto MQTT-broker** verzorgt de berichtuitwisseling. Home Assistant verwerkt de meetgegevens en publiceert een besturingsbericht; de ESP32 ontvangt dit bericht en stuurt daarmee het relais aan.

### Blokschema

```text
[ DHT22 + HC-SR501 ]
          │ meetwaarden en aanwezigheidsstatus
          ▼
       [ ESP32 ]
          │ MQTT over WiFi: sensordata
          ▼
[ Raspberry Pi 4: Home Assistant + Mosquitto ]
          │ MQTT over WiFi: besturingscommando
          ▼
       [ ESP32 ]
          │ stuursignaal
          ▼
      [ Relais ]
          │
          ▼
[ Verwarmingsregeling ]
```

De regelvoorwaarde is: **verwarming inschakelen wanneer aanwezigheid is gedetecteerd én de temperatuur lager is dan de ingestelde grenswaarde**. De exacte grenswaarde en MQTT-topics worden in de configuratie van de software vastgelegd.

## 5. Mapstructuur

```text
/
├── src/
│   └── main.ino       # C++-broncode voor de ESP32
└── docs/
    └── schema.png     # Elektrisch aansluitschema
```

- `/src/` bevat de C++-broncode (`main.ino`) voor de ESP32.
- `/docs/` bevat het elektrische aansluitschema (`schema.png`).

## 6. Installatie en gebruik — tussentijdse testopstelling

1. Sluit de ESP32, DHT22, HC-SR501 en 5V-relaismodule aan volgens het elektrische aansluitschema in `/docs/schema.png`.
2. Stel in de ESP32-code de WiFi- en MQTT-verbindingsgegevens in. Gebruik voor lokale tests de hostnaam of het IP-adres van de Raspberry Pi 4.
3. Open `src/main.ino` in de Arduino IDE, selecteer het juiste ESP32-board en de bijbehorende seriële poort, en flash de code naar het board.
4. Installeer en start Home Assistant en de Mosquitto MQTT-broker op de Raspberry Pi 4; configureer de MQTT-topics voor sensordata en het relaiscommando aan beide kanten.
5. Controleer in Home Assistant of temperatuur- en aanwezigheidsgegevens binnenkomen en test vervolgens of het relais alleen wordt aangestuurd bij aanwezigheid én een temperatuur onder de ingestelde grenswaarde.

> **Veiligheid:** gebruik bij de tussentijdse testopstelling bij voorkeur een veilige laagspanningsbelasting. Een relais dat een verwarmingsinstallatie of netspanning schakelt, moet correct elektrisch geïsoleerd en volgens de geldende veiligheidsvoorschriften aangesloten worden. Laat de aansluiting op netspanning uitvoeren of controleren door een bevoegd persoon. Bewaar WiFi- en MQTT-wachtwoorden niet in publiek gedeelde broncode.
