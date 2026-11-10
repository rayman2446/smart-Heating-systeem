# Firmware

Deze map bevat de firmware voor het edge device van de slimme klimaatregeling per ruimte. Het huidige project draait op een ESP32 Development Board en verwerkt temperatuur- en aanwezigheidsgegevens via WiFi en MQTT.

```text
src/
├── README.md                 # Documentatie van de firmware
├── main.ino                  # Hoofdprogramma voor de ESP32
├── configuration.yaml        # Projectconfiguratie / instellingen
└── libs/                     # Optionele bibliotheken (indien nodig)
```

## 💻 Software stack

| Component | Versie / type | Doel |
|-----------|---------------|------|
| Microcontroller | ESP32-WROOM-32E | Edge device voor meting en communicatie |
| IDE | Arduino IDE / ESP32 Core | Ontwikkelen en uploaden van firmware |
| Taal | C++ | Programmeertaal voor de ESP32 |
| Communicatie | WiFi + MQTT | Verzendt meetdata naar Home Assistant |
| Sensor | DHT22 | Temperatuurmeting |
| Sensor | HC-SR501 PIR | Detecteert beweging / aanwezigheid |
| Broker | Mosquitto MQTT | Ontvangt en distribueert sensor- en besturingsberichten |

## 🎯 Doel van de firmware

De ESP32 leest periodiek de temperatuur uit via de DHT22 en controleert de PIR-sensor op beweging. De gemeten waarden worden via MQTT naar de Raspberry Pi / Home Assistant verstuurd. De regeling is bedoeld om de verwarming lokaal per ruimte te controleren op basis van aanwezigheid en temperatuur.

## 🚀 Aan de slag

### Vereisten

- Arduino IDE
- ESP32 Board Manager geïnstalleerd
- USB-kabel voor de ESP32
- WiFi-gegevens en MQTT-broker IP-adres
- DHT22 en HC-SR501 aangesloten volgens het schema

### Project openen

1. Open de Arduino IDE.
2. Laad `src/main.ino`.
3. Selecteer het juiste ESP32-board (bijv. `ESP32 Dev Module`).
4. Kies de juiste seriële poort.
5. Upload de code naar de ESP32.
6. Controleer de seriële output in de monitor.

### Belangrijk: configuratie

In `src/main.ino` moeten de volgende waarden juist worden ingesteld:

- `ssid` = WiFi-netwerknaam
- `password` = WiFi-wachtwoord
- `mqtt_server` = IP-adres van de Raspberry Pi / Mosquitto-broker

Het huidige project gebruikt deze MQTT-topics:

- `kamer1/temperatuur`
- `kamer1/beweging`

## 🧩 Code overzicht

### `main.ino`

De firmware doet het volgende:

1. Initialiseert WiFi en MQTT-verbinding.
2. Zet de DHT22 in de juiste modus.
3. Configureert GPIO voor de PIR-sensor.
4. Meets elke 10 seconden de temperatuur.
5. Verstuurt de temperatuur naar MQTT.
6. Controleert de PIR-status en stuurt bij verandering een aanwezigheidsbepaling naar MQTT.

```cpp
#define DHTPIN 4
#define DHTTYPE DHT22
#define PIRPIN 5
```

### Belangrijkste functionaliteit

- `setup_wifi()` initialiseert de WiFi-verbinding.
- `reconnect()` maakt de MQTT-verbinding opnieuw als deze wegvalt.
- `loop()` verwerkt het hoofdprogramma en stuurt sensorwaarden.

## 🧷 Pinout

| Pin | Functie | Verbonden met |
|-----|---------|---------------|
| GPIO 4 | DHT22 DATA | Temperatuursensor |
| GPIO 5 | PIR OUT | Aanwezigheidssensor |
| 3V3 | Voeding | DHT22 en sensoren (controleer waardes volgens datasheet) |
| GND | Aarde | Sensor-/ESP32-aarde |
| VIN / 5V | Power | Relaismodule / externe voeding (indien gebruikt) |

> Let op: de daadwerkelijke aansluiting van de relaismodule is nog niet volledig gedocumenteerd. Controleer voor elke externe component de 3.3V-compatibiliteit en de juiste voedings- en aardverbindingen.

## 📡 MQTT en communicatie

Het systeem gebruikt MQTT over WiFi. De huidige firmware publiceert sensorwaarden op deze topics:

- `kamer1/temperatuur` → temperatuur in °C
- `kamer1/beweging` → `Aanwezig` of `Niemand`

## ⚠️ Bekende punten / aandachtspunten

- De firmware is gericht op ESP32 en niet op PSoC 6 / ModusToolbox.
- De huidige code stuurt nog geen relaisaansturing; de actuatorschakeling moet nog worden toegevoegd en getest.
- De exacte relaismodule en de stroomvereisten moeten nog worden gecontroleerd.
- De WiFi- en MQTT-instellingen moeten lokaal worden aangepast naar de juiste netwerkwaarden.

## 📚 Bronnen

- [Project README](../README.md)
- [Hardware README](../hardware/README.md)
- [DHT22 datasheet](../docs/datasheets/DMXSl20.pdf) — controleer of deze bestandnaam overeenkomt met het juiste onderdeel; update als nodig.
- [PSoC 6 datasheet](../docs/datasheets/P5oK6.pdf) — alleen relevant als de hardware later wordt verschoven naar een PSoC-gebaseerde oplossing.

## ✅ Checklist

- [x] ESP32-firmware aanwezig
- [x] DHT22-temperatuurmeting geïmplementeerd
- [x] PIR-aanwezigheid geimplementeerd
- [x] MQTT-publicatie ingesteld
- [ ] Relaisontwerp en -aansluiting volledig gedocumenteerd
- [ ] Elektrisch schema gecontroleerd en toegevoegd
- [ ] WiFi- en MQTT-settings geverifieerd op de werkelijke omgeving
- [ ] Firmware getest op volledige opstelling
- [ ] Pinout definitief vastgelegd
