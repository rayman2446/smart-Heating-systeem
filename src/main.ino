#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

const char* ssid = "wifi-code";
const char* password = "wachtwoord";
const char* mqtt_server = "192.168.65.01"; // IP van je Raspberry Pi

#define DHTPIN 4
#define DHTTYPE DHT22
#define PIRPIN 5

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

long lastMsg = 0;
int lastPIRState = LOW;

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIRPIN, INPUT);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32_Kamer1")) {
      // Verbonden met MQTT
    } else { delay(5000); }
  }
}

void loop() {
  if (!client.connected()) { reconnect(); }
  client.loop();

  long now = millis();
  
  // VERWERKING & SENSOR: Elke 10 sec temperatuur uitlezen
  if (now - lastMsg > 10000) {
    lastMsg = now;
    float t = dht.readTemperature(); // Leest in graden Celsius
    
    if (!isnan(t)) {
      char tempString[8];
      dtostrf(t, 1, 2, tempString);
      // IOT COMMUNICATIE: Verzenden via MQTT
      client.publish("kamer1/temperatuur", tempString);
    }
  }

  // SENSOR: PIR Beweging direct uitlezen
  int pirState = digitalRead(PIRPIN);
  if (pirState != lastPIRState) {
    if (pirState == HIGH) {
      client.publish("kamer1/beweging", "Aanwezig"); // Verzenden MQTT
    } else {
      client.publish("kamer1/beweging", "Niemand");
    }
    lastPIRState = pirState;
  }
}
