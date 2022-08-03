//#include <ESP8266WiFi.h> // Para el ESP8266
#include <WiFi.h> // Para el ESP32
WiFiClient WIFI_CLIENT;
#include <PubSubClient.h>
PubSubClient MQTT_CLIENT;

const char* ssid = "Leouu";
const char* password = "leo12345A";

// const byte led4 = D4; // LED ESP8266
const byte LED2 = 5; // LED ESP32

void setup() {
  pinMode(LED2, OUTPUT);
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.print("Connecting with ");
  Serial.println(ssid);

  
  WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}

Serial.println("");
Serial.print("WiFi conected. IP: ");
Serial.println(WiFi.localIP());

// Setting Callback.
  MQTT_CLIENT.setCallback(callback);
}

// What to do when it receives the data. 
void callback(char* recibido, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.print(recibido);
  Serial.print("   ");
  for (int i=0;i<length;i++) {
    char receivedChar = (char)payload[i];
    Serial.print(receivedChar);
  if (receivedChar == '1') {digitalWrite(LED2, HIGH);}
  if (receivedChar == '0') {digitalWrite(LED2, LOW);}
  }
  Serial.println();
}
 
void loop() {
  if (!MQTT_CLIENT.connected()) {
    reconnect();
  }
  MQTT_CLIENT.loop(); // Check Subscription.
}

// Reconecta con MQTT broker
void reconnect() {
MQTT_CLIENT.setServer("broker.hivemq.com", 1883);  
//MQTT_CLIENT.setServer("mqtt.eclipse.org", 1883);
MQTT_CLIENT.setClient(WIFI_CLIENT);

// Trying connect with broker.
while (!MQTT_CLIENT.connected()) {
Serial.println("Trying to connect with Broker MQTT.");
MQTT_CLIENT.connect("LeoIsleno"); // it isn't necessary..
MQTT_CLIENT.subscribe("Leo/boton"); // HERE SUBSCRIBE.

// Wait to try to reconnect again...
delay(3000);
}

Serial.println("Conectado a MQTT.");
}
