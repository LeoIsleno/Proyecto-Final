//Se envio diferentes tipo de valores desde el ESP32 a Mi App Inventor

#include <WiFi.h> // Para el ESP32
WiFiClient WIFI_CLIENT; //Libreria Wifi
#include <PubSubClient.h> //Libreria para metodo MQTT
#include <TimeLib.h> //Libreria del Reloj
PubSubClient MQTT_CLIENT;

const char* ssid = "Leouu"; //Nombre del Wifi
const char* password = "leo12345A"; //Contraseña del wifi

const byte LED = 13; // LED ESP32

const int fotoresistencia = 36;
const int LED1 = 2;
const int LED2 = 4;
const int sensorSuelo3 = 35;
const int sensorSuelo2 = 34;
const int sensorSuelo1 = 32;
//const int relay = 19;


void setup() {

  //Configuracion del reloj
  setTime(0, 0, 0, 29, 3, 22); //hr,mm,ss,d,m,y

  //Configuracion del ESP32
  pinMode(LED, OUTPUT); //LED como salida

  /*Iniciamos el terminal Serial a una velocidad de 115200, junto a un retardo de 1 segundo y definimos los pines a utilizar*/
  pinMode(fotoresistencia, INPUT); //Configuramos fotoresistencia como entrada o INPUT
  //pinMode(sensorSuelo1, INPUT); //Configuramos sensor Temperatura como entrada o INPUT
  //pinMode(sensorSuelo2, INPUT); //Configuramos sensor Temperatura como entrada o INPUT
  //pinMode(sensorSuelo3, INPUT); //Configuramos sensor Temperatura como entrada o INPUT
  //pinMode(34, INPUT); //Configuramos sensor de Suelo como entrada o INPUT
  pinMode(LED1, OUTPUT); // Configurar relay como salida o OUTPUT
  pinMode(LED2, OUTPUT); // Configurar relay como salida o OUTPUT

  Serial.begin(115200); //Inicializa el serial begin a 115200
  delay(1000); // 1 segundo
  Serial.println("Sensores Instalados y listos");
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
  int receivedInt;
  int num;
  String var;
  Serial.print("Message received: ");
  Serial.print(recibido);
  Serial.print("   ");

  for (int i = 0; i < length; i++) {
    receivedInt = (char)payload[i] - '0';
    (var).concat(receivedInt); //Une todo el codigo segmentado en uno mismo y lo guarda como String
  }
  Serial.println(var);
  menu(receivedInt);
}

void loop() {
  if (!MQTT_CLIENT.connected()) {
    reconnect();
  }
  //mediciones();
  delay(1000);
  MQTT_CLIENT.loop(); // Testea la suscripcion
}

void mediciones() {
  float lectura_fotoresistencia;
  int lectura_sensorSuelo1;
  int lectura_sensorSuelo2;
  int lectura_sensorSuelo3;
  Serial.println("====================================");
  lectura_fotoresistencia = analogRead(fotoresistencia);
  //Serial.println(lectura);
  lectura_sensorSuelo1 = analogRead(sensorSuelo1);
  Serial.println(lectura_sensorSuelo1);
  lectura_sensorSuelo2 = analogRead(sensorSuelo2);
  Serial.println(lectura_sensorSuelo2);
  lectura_sensorSuelo3 = analogRead(sensorSuelo3);
  Serial.println(lectura_sensorSuelo3);

  lectura_fotoresistencia = (lectura_fotoresistencia / 4095) * 100;
  Serial.print("Valor de la fotoresistencia: ");
  Serial.println(lectura_fotoresistencia);
  delay(5000);

  if (lectura_fotoresistencia > 50) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
  }
  else {
    digitalWrite(LED2, HIGH);
    digitalWrite(LED1, LOW);
  }

  String dato = String(lectura_fotoresistencia); //Se convierte el tipo de variable de int a String
  char a[1];
  dato.toCharArray(a, 12); //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
  Serial.println(a);
  MQTT_CLIENT.publish("Leo/Informacion", a); //Envia la informacion dentro del arreglo char
  //return dato;
}

void menu(int dato) {
  switch (dato) {
    case 001:
      digitalWrite(LED, HIGH);
      Serial.print("Led Encendido");
      break;
    case 002:
      Serial.print("Led Apagado");
      digitalWrite(LED, LOW);
      break;
    default:
      Serial.println("No se selecciono ninguna verdura");
      break ;
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
