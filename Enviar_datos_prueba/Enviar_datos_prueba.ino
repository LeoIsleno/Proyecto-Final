//Broker diferente, envio de datos desde la app hacia el ESP32
//Prueba 1, cambiar estado de un led

#include <WiFi.h> // Para el ESP32
WiFiClient WIFI_CLIENT;
#include <PubSubClient.h>
PubSubClient MQTT_CLIENT;

const char* ssid = "Leouu"; //Nombre del Wifi
const char* password = "leo12345A"; //Contraseña del wifi

const byte LED = 5; // LED ESP32
//const byte LED2 = 18; // LED ESP32

String LED_status = "- ? -";

void setup() {
  pinMode(LED, OUTPUT); //LED como salida
  // pinMode(LED2, OUTPUT); //LED como salida
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
  //for (int i=0;i<length;i++) {
  char receivedChar = (char)payload[0];
  Serial.print(receivedChar);

  if (receivedChar == '0') {
    digitalWrite(LED, LOW);
  }
  if (receivedChar == '1') {
    digitalWrite(LED, HIGH);
  }

  if (digitalRead(LED) == HIGH) {
    LED_status = "LED ON";
  } else {
    LED_status = "LED OFF";
  }
  /*
    if (receivedChar == '2') {
    //Serial.print("ON\n");
    digitalWrite(LED2, HIGH);
    }
    if (receivedChar == '3') {
    digitalWrite(LED2, LOW);
    }
  */
  //}
  Serial.println();
}

void loop() {
  if (!MQTT_CLIENT.connected()) {
    reconnect();
  }
  
  char led_st[10];
  LED_status.toCharArray(led_st, 10);
  MQTT_CLIENT.publish("Leo/LED_status", led_st);
  delay(1000);

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

    MQTT_CLIENT.subscribe("Leo/LED_status"); // Aca realiza la suscripcion
  //  MQTT_CLIENT.subscribe("Leo/boton2"); // Aca realiza la suscripcion

    // Espera para que conecte denuevo
    delay(3000);
  }
  Serial.println("Conectado a MQTT.");
}
