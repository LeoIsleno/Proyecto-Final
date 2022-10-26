//Se envio diferentes tipo de valores desde el ESP32 a Mi App Inventor
/*
 * 26/10/22 - Se implemento el envio de 2 sensores al mismo tiempo a mi App Inventor sin problemas.
 * Apartir del topic recibido por el ESP32 se pueden difereneciar las acciones a trabajar
 * Se logro enviar formato Fecha al ESP32 para luego trabajarla para el calculo de cosecha
 * Variable nueva aprender: parceInt() - Link: https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&cad=rja&uact=8&ved=2ahUKEwjqs4nutv76AhXcrJUCHZsZBokQFnoECBAQAQ&url=https%3A%2F%2Fwww.arduino.cc%2Freference%2Fes%2Flanguage%2Ffunctions%2Fcommunication%2Fserial%2Fparseint&usg=AOvVaw0YJv_LhYxHrFJglwGjwFph
*/

#include <WiFi.h> // Para el ESP32
WiFiClient WIFI_CLIENT; //Libreria Wifi
#include <PubSubClient.h> //Libreria para metodo MQTT
#include <TimeLib.h> //Libreria del Reloj
PubSubClient MQTT_CLIENT;

const char* ssid = "Leouu"; //Nombre del Wifi
const char* password = "leo12345B"; //Contraseña del wifi

const int fotoresistencia = 39;
const int LED1 = 2;
const int LED2 = 4;
const int sensorSuelo3 = 35;
const int sensorSuelo2 = 34;
const int sensorSuelo1 = 32;
const int sensorTemp = 33;
//const int relay = 19;


void setup() {

  //Configuracion del reloj
  setTime(0, 0, 0, 29, 3, 22); //hr,mm,ss,d,m,y

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
  String var;
  int num;
  Serial.print("Message received: ");
  Serial.print(recibido);
  Serial.print("   ");

  for (int i = 0; i < length; i++) {
    receivedInt = (char)payload[i] - '0'; //Convierte de Char to int
    (var).concat(receivedInt); // Une todo el codigo segmentado en uno mismo y lo guarda como Int, de momento no se utiliza al no tener gran variedad de plantas a utilizar
  }
  //num = var.toInt
  if (recibido == "Leo/Fecha"){
    Serial.println("Fecha:");
    Serial.println(var);
  }
  else{
    Serial.println(var);
  }
  //Serial.println(var);
  //menu(receivedInt);
}
void loop() {
  if (!MQTT_CLIENT.connected()) {
    reconnect();
  }
  /*
    String dato = String(mediciones()); //Se convierte el tipo de variable de int a String
    char a[1];
    dato.toCharArray(a, 12); //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
    Serial.println(a);
    MQTT_CLIENT.publish("Leo/Informacion", a); //Envia la informacion dentro del arreglo char
  */
  mediciones();
  delay(1000);
  MQTT_CLIENT.loop(); // Testea la suscripcion
}

void mediciones() {
  float lectura_fotoresistencia, lectura_sensorTemp;
  int lectura_sensorSuelo1, lectura_sensorSuelo2, lectura_sensorSuelo3, TAM = 3;
  int lecturaAnalog[5];

  //Lectura de los sensores
  Serial.println(">====================================<");

  lectura_fotoresistencia = analogRead(fotoresistencia); //(analogRead(fotoresistencia)/ 4095) * 100; // PIN 39
  lectura_sensorSuelo1 = analogRead(sensorSuelo1);
  lectura_sensorSuelo2 = analogRead(sensorSuelo2);
  lectura_sensorSuelo3 = analogRead(sensorSuelo3);
  lectura_sensorTemp = analogRead(sensorTemp) * 0.1;  //  10mv/°C PIN 33
  Serial.println(lectura_fotoresistencia);
  /*
    //Muestra de datos
    //Serial.println(lectura);
    //Serial.println(lectura_sensorSuelo1);
    //Serial.println(lectura_sensorSuelo2);
    //Serial.println(lectura_sensorSuelo3);
    //Serial.println(lectura_sensorTemp);

    //lectura_fotoresistencia = (lectura_fotoresistencia ;
    //Serial.print("Valor de la fotoresistencia: ");


    if (lectura_fotoresistencia > 50) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    }
    else {
    digitalWrite(LED2, HIGH);
    digitalWrite(LED1, LOW);
    }
    lecturaAnalog[1] = (analogRead(fotoresistencia)/ 4095) * 100;
    Serial.println(lecturaAnalog[1]);
    lecturaAnalog[2] = analogRead(sensorTemp) * 0.1;  //  10mv/°C
    Serial.println(lecturaAnalog[2]);

    /*
    for (int i = 0; i < TAM; i ++){

    }
  */
  Serial.println("<====================================>");
  Serial.println("Valores de publish");

  //Publish de todos los valores de los sensores analogicos
  
  String dato1 = String(lectura_sensorTemp); //Se convierte el tipo de variable de int a String
  char a[1];
  dato1.toCharArray(a, 12); //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
  Serial.println(a);
  MQTT_CLIENT.publish("Inf/SensorTemperatura", a); //Envia la informacion dentro del arreglo char

  delay(500);

  String dato2 = String(lectura_fotoresistencia); //Se convierte el tipo de variable de int a String
  char b[1];
  dato2.toCharArray(b, 12); //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
  Serial.println(b);
  MQTT_CLIENT.publish("Inf/Fotoresistencia", b); //Envia la informacion dentro del arreglo char

  delay(5000);  
  //return lectura_sensorTemp;
}

void menu(int dato) {
  switch (dato) {
    case 1:
      digitalWrite(LED1, HIGH);
      Serial.println("Led Encendido");
      break;
    case 2:
      Serial.println("Led Apagado");
      digitalWrite(LED1, LOW);
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
    MQTT_CLIENT.subscribe("Leo/Cebollas"); // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Leo/Fecha"); // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Fecha"); // Aca realiza la suscripcion

    // Espera para que conecte denuevo
    delay(3000);
  }
  Serial.println("Conectado a MQTT.");
}
