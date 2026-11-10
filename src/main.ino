#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ================= NETWERK & BROKER =================
const char* ssid = "Proximus_1258";
const char* password = "JOUW_WIFI_WACHTWOORD"; // Wachtwoord afgeschermd voor GitHub
const char* mqtt_server = "192.168.65.01";    // IP-adres van je Raspberry Pi

// ================= PIN CONFIGURATIE =================
#define DHTPIN 4       // DHT22 Data op GPIO 4
#define DHTTYPE DHT22
#define PIRPIN 5       // PIR Motion op GPIO 5
#define RELAYPIN 18    // Relais Signaal op GPIO 18

// ================= MQTT TOPICS =================
const char* topic_temp = "kamer1/temperatuur";
const char* topic_motion = "kamer1/beweging";
const char* topic_relay_set = "kamer1/relais/set";

// ================= OBJECTEN & VARIABELEN =================
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

long lastMsg = 0;
int lastPIRState = LOW;

// ================= MQTT CALLBACK (ONTVANGEN BERICHTEN) =================
void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }

  // Controleer of het bericht voor het relais is bestemd
  if (String(topic) == topic_relay_set) {
    if (messageTemp == "AAN") {
      digitalWrite(RELAYPIN, HIGH);
    } else if (messageTemp == "UIT") {
      digitalWrite(RELAYPIN, LOW);
    }
  }
}

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32_Kamer1")) {
      // Abonneer op het relais-topic zodra de verbinding actief is
      client.subscribe(topic_relay_set);
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIRPIN, INPUT);
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, LOW); // Standaard uit bij opstarten

  dht.begin();
  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); // Stel de callback-functie in
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  
  // VERWERKING & SENSOR: Elke 10 seconden temperatuur uitlezen
  if (now - lastMsg > 10000) {
    lastMsg = now;
    float t = dht.readTemperature();
    
    if (!isnan(t)) {
      char tempString[8];
      dtostrf(t, 1, 2, tempString);
      client.publish(topic_temp, tempString);
    }
  }

  // SENSOR: PIR Beweging direct uitlezen
  int pirState = digitalRead(PIRPIN);
  if (pirState != lastPIRState) {
    if (pirState == HIGH) {
      client.publish(topic_motion, "Aanwezig");
    } else {
      client.publish(topic_motion, "Niemand");
    }
    lastPIRState = pirState;
  }
}
