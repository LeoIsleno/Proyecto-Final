//Broker diferente, envio de datos desde la app hacia el ESP32
//Prueba 1, cambiar estado de un led

#include <WiFi.h> // Para el ESP32
WiFiClient WIFI_CLIENT; //Libreria Wifi
#include <PubSubClient.h> //Libreria para metodo MQTT
#include <TimeLib.h> //Libreria del Reloj
PubSubClient MQTT_CLIENT;

const char* ssid = "Leouu"; //Nombre del Wifi
const char* password = "leo12345A"; //Contraseña del wifi

const byte LED = 13; // LED ESP32

void setup() {
  
  //Configuracion del reloj
  setTime(0, 0, 0, 29, 3, 22); //hr,mm,ss,d,m,y

  //Configuracion del ESP32
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
  char receivedChar;
  String var;
  Serial.print("Message received: ");
  Serial.print(recibido);
  Serial.print("   ");
  
  for (int i = 0; i < length; i++) {
   receivedChar = (char)payload[i];
   (var).concat(receivedChar);
   Serial.println(payload[i]);
 }
 Serial.println(var);
 menu(var);
}

void loop() {
  if (!MQTT_CLIENT.connected()) {
    reconnect();
  }
  //reloj();
  delay(1000);
  MQTT_CLIENT.loop(); // Testea la suscripcion
}

void menu(String dato) {
  if (dato == "001") {
    digitalWrite(LED, HIGH);
    Serial.print("Led Encendido");
  }
  if (dato == "002") {
    Serial.print("Led Apagado");
    digitalWrite(LED, LOW);
  }
}

String dato(int digit) {
  String dt = String("0") + digit;
  return dt.substring(dt.length() - 2);
}

void reloj () {
  String tiempo = String (hour()) + ";" + dato(minute()) + ":" + dato(second());
  Serial.println(tiempo);
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
    MQTT_CLIENT.subscribe("Leo/Tomates"); // Aca realiza la suscripcion
    //MQTT_CLIENT.subscribe("Leo/Cebollas");

    // Espera para que conecte denuevo
    delay(3000);
  }
  Serial.println("Conectado a MQTT.");
}
