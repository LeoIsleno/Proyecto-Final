#include <WiFi.h> // Para el ESP32
WiFiClient WIFI_CLIENT;
#include <PubSubClient.h>
PubSubClient MQTT_CLIENT;

const char* ssid = "Leouu";
const char* password = "leo12345A";

void setup() {
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

}

void loop() {
// Check MQTT Broker connection,
// en caso de que no tenga, reconecta.
if (!MQTT_CLIENT.connected()) {
reconnect();
}

// Publish topic.
// Convierte el entero a char. Debe ser char.
int aleatorio = random(1,90);
String aleatorioString = String(aleatorio);
char alea[6];
aleatorioString.toCharArray(alea, 6);

MQTT_CLIENT.publish("Leo/aleatorio", alea);

// Wait 5 s.
delay(5000);
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

// Wait to try to reconnect again...
delay(3000);
}

Serial.println("Conectado a MQTT.");
}
