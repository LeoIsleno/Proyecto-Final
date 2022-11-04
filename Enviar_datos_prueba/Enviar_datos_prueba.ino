/*
 * 4/11/22 - Se implemento el envio de la fecha a cosechar al app inventor
 * Todos los datos de los sensores se envian correctamente y se reciben los valores correspondientes
 * Proximamente se dividira los sectores para activar los correspondientes sensores segun el sector a trabajar. 
*/

#include <WiFi.h>          // Para el ESP32
WiFiClient WIFI_CLIENT;    //Libreria Wifi
#include <PubSubClient.h>  //Libreria para metodo MQTT
#include <TimeLib.h>       //Libreria del Reloj
PubSubClient MQTT_CLIENT;

const char* ssid = "Leouu";          //Nombre del Wifi
const char* password = "leo12345B";  //Contraseña del wifi

const int fotoresistencia = 39;
const int LED1 = 2;
const int LED2 = 4;
const int sensorSuelo3 = 35;
const int sensorSuelo2 = 34;
const int sensorSuelo1 = 32;
const int sensorTemp = 33;
//const int relay = 19;

void recived(String topic, String valor);

void setup() {

  //Configuracion del reloj
  setTime(0, 0, 0, 29, 3, 22);  //hr,mm,ss,d,m,y

  /*Iniciamos el terminal Serial a una velocidad de 115200, junto a un retardo de 1 segundo y definimos los pines a utilizar*/
  pinMode(LED1, OUTPUT);  // Configurar relay como salida o OUTPUT
  pinMode(LED2, OUTPUT);  // Configurar relay como salida o OUTPUT

  pinMode(fotoresistencia, INPUT);  // Configurar relay como salida o OUTPUT

  Serial.begin(115200);  //Inicializa el serial begin a 115200
  delay(1000);           // 1 segundo
  Serial.println("Sensores Instalados y listos");
  Serial.println();
  Serial.print("Conectando con ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);  //Inicializa el modulo WIFI
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
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

  Serial.print("Message received: ");
  Serial.print(recibido);
  Serial.print("   ");

  String recibidoStr = String(recibido);

  for (int i = 0; i < length; i++) {
    receivedInt = (char)payload[i] - '0';  //Convierte de Char to int
    (var).concat(receivedInt);             // Une todo el codigo segmentado en uno mismo y lo guarda como Int, de momento no se utiliza al no tener gran variedad de plantas a utilizar
  }

  Serial.print("Codigo concatenado: ");
  Serial.println(var);
  Serial.println("  ");

  recived(recibidoStr, var);  //Topic - Valor recibido
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
  delay(5000);
  MQTT_CLIENT.loop();  // Testea la suscripcion
}

void recived(String topic, String valor) {
  int flagStateTomates;
  int eleccion = 0;

  flagStateTomates = 0;

  if (topic == "Tomates/Fecha") {
    eleccion = 1;
    Serial.print("Codigo recibido: ");
    Serial.println(eleccion);
    Serial.println("  ");

    Serial.print("Fecha de cultivo elegido:");
    Serial.println(valor);
    Serial.println("  ");

    seleccion_cultivo(eleccion, valor);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }
}

void seleccion_cultivo(int eleccion, String Fecha) {
  int cantMes;
  String fechaCosechada;

  switch (eleccion) {
    case 1:         //Eleccion de Tomates
      cantMes = 4;  //Se cosecha luego de los 4 meses
      Serial.println("Comenzando el calculo de la cosecha de Tomates...");
      Serial.println("  ");
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.print("Fecha de cosecha: ");
      Serial.println(fechaCosechada);
      
      char a[1];
      fechaCosechada.toCharArray(a, 12);  //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
      Serial.print("Dato enviado: ");
      Serial.println(a);
      MQTT_CLIENT.publish("Fecha/cosecha", a);  //Envia la informacion dentro del arreglo char
      delay(500);
      break;
  }
}

String fecha_cosecha(String Fecha, int cantMes) {
  int dia, mes, anio;
  String var;
  String dato;

  var = Fecha.substring(0, 3);
  dia = var.toInt();
  var = Fecha.substring(3, 6);
  mes = var.toInt();
  var = Fecha.substring(6, 9);
  anio = var.toInt();

  if (mes < 12) {
    mes = mes - 12;
    anio = anio + 1;
  }

  dato.concat(dia);
  dato.concat("/");
  dato.concat(mes + cantMes);
  dato.concat("/");
  dato.concat(anio);

  return dato;
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
    MQTT_CLIENT.subscribe("Tomates/Fecha");  // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Leo/Cebollas");   // Aca realiza la suscripcion
                                             // MQTT_CLIENT.subscribe("Leo/Fecha");     // Aca realiza la suscripcion

    // Espera para que conecte denuevo
    delay(3000);
  }
  Serial.println("Conectado a MQTT.");
}