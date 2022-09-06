//Se envio diferentes tipo de valores desde el ESP32 a Mi App Inventor

#include <WiFi.h> // Para el ESP32
WiFiClient WIFI_CLIENT; //Libreria Wifi
#include <PubSubClient.h> //Libreria para metodo MQTT
#include <TimeLib.h> //Libreria del Reloj
PubSubClient MQTT_CLIENT;

const char* ssid = "Leouu"; //Nombre del Wifi
const char* password = "leo12345A"; //Contraseña del wifi

const byte LED = 13; // LED ESP32

const int fotoresistencia = 32;
const int sensorTemperatura = 35;
const int sensorSuelo = 34;
const int relay = 19;
int lectura;

void setup() {

  //Configuracion del reloj
  setTime(0, 0, 0, 29, 3, 22); //hr,mm,ss,d,m,y

  //Configuracion del ESP32
  pinMode(LED, OUTPUT); //LED como salida

  /*Iniciamos el terminal Serial a una velocidad de 115200, junto a un retardo de 1 segundo y definimos los pines a utilizar*/
  pinMode(32, INPUT); //Configuramos fotoresistencia como entrada o INPUT
  pinMode(35, INPUT); //Configuramos sensor Temperatura como entrada o INPUT
  pinMode(34, INPUT); //Configuramos sensor de Suelo como entrada o INPUT
  pinMode(relay, OUTPUT); // Configurar relay como salida o OUTPUT

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
  char receivedChar;
  String var;
  Serial.print("Message received: ");
  Serial.print(recibido);
  Serial.print("   ");

  for (int i = 0; i < length; i++) {
    receivedChar = (char)payload[i];
    (var).concat(receivedChar); //Une todo el codigo segmentado en uno mismo y lo guarda como String
  }
  Serial.println(var);
  menu(var);
}

void loop() {
  if (!MQTT_CLIENT.connected()) {
    reconnect();
  }
  //reloj();

  int aleatorio = random(1, 90);  //Se crea una variable aleatorio dentro de los numero de 1 y 90
  String aleatorioString = String(aleatorio); //Se convierte el tipo de variable de int a String
  char alea[6];
  aleatorioString.toCharArray(alea, 6); //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )

  MQTT_CLIENT.publish("Leo/Datos", alea); //Envia la informacion dentro del arreglo char

  String dato = "Hola Mundo";
  char a[12];
  dato.toCharArray(a, 12); //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
  
  MQTT_CLIENT.publish("Leo/Informacion", a); //Envia la informacion dentro del arreglo char

  delay(2000);
  MQTT_CLIENT.loop(); // Testea la suscripcion
}

void mediciones() {
  Serial.println("====================================");
  lectura = analogRead(fotoresistencia);
  Serial.print("Valor de la fotoresistencia: ");
  Serial.println(lectura);
  lectura = analogRead(sensorTemperatura);
  Serial.print("Valor del sensor de temperatura: ");
  Serial.println(lectura);
  lectura = analogRead(sensorSuelo);
  Serial.print("Valor del sensor de suelo: ");
  Serial.println(lectura);
  delay(2000);

  //Prueba de Modulos Relay
  digitalWrite(relay, HIGH); // envia señal alta al relay
  Serial.println("Relay accionado");
  delay(1000);           // 1 segundo

  digitalWrite(relay, LOW);  // envia señal baja al relay
  Serial.println("Relay no accionado");
  delay(1000);           // 1 segundo
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
  if (dato = "9") {
    Serial.println("No se selecciono ninguna verdura");
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
