#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ================= WAARDEN AANPASSEN =================
const char* ssid = "JOUW_WIFI_NAAM";
const char* password = "JOUW_WIFI_WACHTWOORD";
const char* mqtt_server = "192.168.X.X"; // Vul hier het IP adres van je Raspberry Pi in

// ================= PIN CONFIGURATIE =================
#define DHTPIN 4       // Pin waar de DHT22 data op is aangesloten (GPIO 4)
#define DHTTYPE DHT22  // Type DHT sensor
#define PIRPIN 5       // Pin waar de HC-SR501 op is aangesloten (GPIO 5)
#define RELAYPIN 18    // Pin waar de Relais module op is aangesloten (GPIO 18)

// ================= MQTT TOPICS =================
const char* topic_temp = "kamer1/temperatuur";
const char* topic_motion = "kamer1/beweging";
const char* topic_relay_set = "kamer1/relais/set";

// ================= INITIALISATIE =================
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

long lastMsg = 0;
int lastPIRState = LOW; // Bewaart de vorige status van de bewegingsmelder

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Verbinden met ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi verbonden");
  Serial.println("IP adres: ");
  Serial.println(WiFi.localIP());
}

// Functie die wordt uitgevoerd als er een MQTT bericht binnenkomt (bijv. van Home Assistant)
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Bericht ontvangen op topic: ");
  Serial.print(topic);
  Serial.print(". Bericht: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Check of het bericht voor het relais is
  if (String(topic) == topic_relay_set) {
    if (messageTemp == "AAN") {
      digitalWrite(RELAYPIN, HIGH);
      Serial.println("Relais ingeschakeld");
    }
    else if (messageTemp == "UIT") {
      digitalWrite(RELAYPIN, LOW);
      Serial.println("Relais uitgeschakeld");
    }
  }
}

void reconnect() {
  // Blijf proberen tot we weer verbonden zijn met de MQTT broker
  while (!client.connected()) {
    Serial.print("Proberen te verbinden met MQTT broker...");
    // Verbind met een unieke Client ID
    if (client.connect("ESP32_Kamer1_Client")) {
      Serial.println("Verbonden!");
      // Abonneer op het relais topic zodat we commando's kunnen ontvangen
      client.subscribe(topic_relay_set);
    } else {
      Serial.print("Mislukt, rc=");
      Serial.print(client.state());
      Serial.println(" Probeer opnieuw over 5 seconden");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Pin modes instellen
  pinMode(PIRPIN, INPUT);
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, LOW); // Zorg dat relais standaard uit staat

  dht.begin();
  setup_wifi();
  
  client.setServer(mqtt_server, 1883); // 1883 is de standaard MQTT poort
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Zorgt dat de MQTT connectie in de lucht blijft

  long now = millis();
  
  // Elke 10 seconden temperatuur uitlezen en versturen
  if (now - lastMsg > 10000) {
    lastMsg = now;
    
    float t = dht.readTemperature();
    
    // Check of de sensor goed werkt
    if (isnan(t)) {
      Serial.println("Fout bij uitlezen van DHT sensor!");
    } else {
      // Converteer float naar string met 2 decimalen
      char tempString[8];
      dtostrf(t, 1, 2, tempString);
      
      Serial.print("Temperatuur: ");
      Serial.println(tempString);
      
      // Stuur temperatuur naar MQTT
      client.publish(topic_temp, tempString);
    }
  }

  // Bewegingssensor (PIR) direct uitlezen (snelle reactie vereist)
  int pirState = digitalRead(PIRPIN);
  
  if (pirState != lastPIRState) {
    if (pirState == HIGH) {
      Serial.println("Beweging gedetecteerd!");
      client.publish(topic_motion, "JA");
    } else {
      Serial.println("Geen beweging meer.");
      client.publish(topic_motion, "NEE");
    }
    lastPIRState = pirState;
  }
}