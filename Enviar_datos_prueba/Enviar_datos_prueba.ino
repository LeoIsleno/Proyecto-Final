//Broker diferente, envio de datos desde la app hacia el ESP32 
//Prueba 1, cambiar estado de un led

#include <WiFi.h> // Para el ESP32
WiFiClient WIFI_CLIENT;
#include <PubSubClient.h>
PubSubClient MQTT_CLIENT;

const char* ssid = "Leouu"; //Nombre del Wifi
const char* password = "leo12345A"; //Contraseña del wifi

const byte LED = 5; // LED ESP32

void setup() {
  pinMode(LED, OUTPUT); //LED como salida
  Serial.begin(115200); //Inicializa el serial begin a 115200
  delay(10);
  Serial.println();
  Serial.print("Conectando con ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password); //Inicializa el modulo WIFI

while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print("Conectando");
Serial.print(".");
}

Serial.println("");
Serial.print("WiFi conectado. IP: ");
Serial.println(WiFi.localIP());

// Setting Callback.
  MQTT_CLIENT.setCallback(callback);
}

// Funcion a seguir cuando recive la informacion
void callback(char* recibido, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.print(recibido);
  Serial.print("   ");
  for (int i=0;i<length;i++) {
    char receivedChar = (char)payload[i];
    Serial.print(receivedChar);
  if (receivedChar == '1') {digitalWrite(LED, HIGH);}
  if (receivedChar == '0') {digitalWrite(LED, LOW);}
  }
  Serial.println();
}
 
void loop() {
  if (!MQTT_CLIENT.connected()) {
    reconnect();
  }
  MQTT_CLIENT.loop(); // Testea la suscripcion
}

// Reconecta con MQTT broker
void reconnect() {
MQTT_CLIENT.setServer("broker.hivemq.com", 1883);  //IP y Puerto Utilizados
//MQTT_CLIENT.setServer("mqtt.eclipse.org", 1883); //IP y Puerto en prueba
MQTT_CLIENT.setClient(WIFI_CLIENT);

// Intentando conectar con el broker
while (!MQTT_CLIENT.connected()) {
Serial.println("Trying to connect with Broker MQTT.");
MQTT_CLIENT.connect("LeoIsleno"); 
MQTT_CLIENT.subscribe("Leo/boton"); // Aca realiza la suscripcion

// Espera para que conecte denuevo
delay(3000);
}
Serial.println("Conectado a MQTT.");
}
