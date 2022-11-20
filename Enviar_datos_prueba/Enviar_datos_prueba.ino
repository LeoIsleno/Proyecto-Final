/*
 * 16/11/22 
 * Los 3 sectores funcionan correctamente segun el tipo de Verdura a cultivar, calculando la fecha de cosecha y enviandola para App Inventor.
 * Correcto funcionamiento al corroborar la fecha del dia de hoy conrespecto al de cultivo, el programa se detiene y obliga al usuario cosechar nuevamente (Unicamente en el Sector 1 - Incompleto)
*/

#include <WiFi.h>          // Para el ESP32
WiFiClient WIFI_CLIENT;    //Libreria Wifi
#include <PubSubClient.h>  //Libreria para metodo MQTT
#include <TimeLib.h>       //Libreria del Reloj
#include <MQTT.h>
PubSubClient MQTT_CLIENT;

const char* ssid = "Leouu";          //Nombre del Wifi
const char* password = "leo12345S";  //Contraseña del wifi

const int fotoresistencia = 39;
const int sensorSuelo3 = 35;
const int sensorSuelo2 = 34;
const int sensorSuelo1 = 32;

const int sensorTemp = 33;
//const int relay = 19;

bool StateSector = 0, flagFechaComprobar = 0, flagActivarCFC1 = 0, flagActivarCFC2 = 0, flagActivarCFC3 = 0, flagActivarCFC = 0, flagGlobalActivarAlerta = 0;
String fechaHoy = "a", fechaCultivo1 = "b", fechaCultivo2 = "c", fechaCultivo3 = "d";

void recived(String topic, String valor);

void setup() {

  //Configuracion del reloj
  setTime(0, 0, 0, 29, 3, 22);  //hr,mm,ss,d,m,y

  /*Iniciamos el terminal Serial a una velocidad de 115200, junto a un retardo de 1 segundo y definimos los pines a utilizar*/

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

// Funcion a seguir cuando recibe la informacion
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

//Main
void loop() {
  if (!MQTT_CLIENT.connected()) {
    reconnect();
  }
  menu_sector(StateSector);
  MQTT_CLIENT.loop();  // Testea la suscripcion
}

//Segun lo recibido desde la App segun el topic y mensaje recibido actua
void recived(String topic, String valor) {
  int flagStateTomates;
  int eleccion = 0;
  char* enviarTopic1 = "Fecha/cosecha1";
  char* enviarTopic2 = "Fecha/cosecha2";
  char* enviarTopic3 = "Fecha/cosecha3";


  if (topic == "State/Sector1") {
    if (valor == "1") {
      StateSector = 1;  //Cambia el estado de Seccion - Activa el sector 1
    } else {
      StateSector = 0;  //Cambia el estado de Seccion - Desactiva el sector 1
    }
  }

  if (topic == "State/Sector2") {
    if (valor == "1") {
      StateSector = 2;  //Cambia el estado de Seccion - Activa el sector 1
    } else {
      StateSector = 0;  //Cambia el estado de Seccion - Desactiva el sector 1
    }
  }

  if (topic == "State/Sector3") {
    if (valor == "1") {
      StateSector = 3;  //Cambia el estado de Seccion - Activa el sector 1
    } else {
      StateSector = 0;  //Cambia el estado de Seccion - Desactiva el sector 1
    }
  }

  if (topic == "Comprobar/FechaHoy") {
    fechaHoy = valor;
    flagActivarCFC = 1;
  }

  if (topic == "Comprobar/Cultivo1") {
    fechaCultivo1 = valor;
    flagActivarCFC1 = 1;
  }

  if (topic == "Comprobar/Cultivo2") {
    fechaCultivo2 = valor;
    flagActivarCFC2 = 1;
  }

  if (topic == "Comprobar/Cultivo3") {
    fechaCultivo3 = valor;
    flagActivarCFC3 = 1;
  }

  if (flagActivarCFC1 && flagActivarCFC2 && flagActivarCFC3) {

    flagActivarCFC = 0;
    flagActivarCFC1 = 0;
    flagActivarCFC2 = 0;
    flagActivarCFC3 = 0;

    comprobarFechasCosechas(fechaHoy, fechaCultivo1, fechaCultivo2, fechaCultivo3);
  }

  if (topic == "Tomates/Fecha1") {
    eleccion = 1;
    seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Tomates/Fecha2") {
    eleccion = 1;

    /*
    Serial.print("Codigo recibido: ");
    Serial.println(eleccion);
    Serial.println("  ");

    Serial.print("Fecha de cultivo elegido:");
    Serial.println(valor);
    Serial.println("  ");
    */
    seleccion_cultivo(eleccion, valor, enviarTopic2);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Tomates/Fecha3") {
    eleccion = 1;
    /*
    Serial.print("Codigo recibido: ");
    Serial.println(eleccion);
    Serial.println("  ");

    Serial.print("Fecha de cultivo elegido:");
    Serial.println(valor);
    Serial.println("  ");
    */
    seleccion_cultivo(eleccion, valor, enviarTopic3);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Cebollas/Fecha1") {
    eleccion = 2;
    seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Cebollas/Fecha2") {
    eleccion = 2;
    seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Cebollas/Fecha3") {
    eleccion = 2;
    seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Lechuga/Fecha1") {
    eleccion = 3;
    seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Lechuga/Fecha2") {
    eleccion = 3;
    seleccion_cultivo(eleccion, valor, enviarTopic2);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Lechuga/Fecha3") {
    eleccion = 3;
    seleccion_cultivo(eleccion, valor, enviarTopic3);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Zanahoria/Fecha1") {
    eleccion = 4;
    seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Zanahoria/Fecha2") {
    eleccion = 4;
    seleccion_cultivo(eleccion, valor, enviarTopic2);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Zanahoria/Fecha3") {
    eleccion = 4;
    seleccion_cultivo(eleccion, valor, enviarTopic3);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Pimiento/Fecha1") {
    eleccion = 5;
    seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Pimiento/Fecha2") {
    eleccion = 5;
    seleccion_cultivo(eleccion, valor, enviarTopic2);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Pimiento/Fecha3") {
    eleccion = 5;
    seleccion_cultivo(eleccion, valor, enviarTopic3);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }
}

void comprobarFechasCosechas(String fechaHoy, String fechaCultivo1, String fechaCultivo2, String fechaCultivo3) {
  char* topicAlertaSector1 = "alerta/Sector1";
  char* topicAlertaSector2 = "alerta/Sector2";
  char* topicAlertaSector3 = "alerta/Sector3";

  //fechaCultivo1 = concatenarfecha(fechaCultivo1);
  fechaCultivo1 = concatenarfecha(fechaCultivo1);
  fechaCultivo2 = concatenarfecha(fechaCultivo2);
  fechaCultivo3 = concatenarfecha(fechaHoy);
  fechaHoy = concatenarfecha(fechaHoy);
  Serial.println(fechaCultivo3);
  Serial.println(fechaHoy);

  if (fechaCultivo1 == fechaHoy) {
    Serial.println("Alerta de Cosecha al sector 1");
    alertaCosecha(1, topicAlertaSector1);
    flagGlobalActivarAlerta = 1;
  } else if (fechaCultivo2 == fechaHoy) {
    alertaCosecha(2, topicAlertaSector2);
    flagGlobalActivarAlerta = 1;
  } else if (fechaCultivo3 == fechaHoy) {
    alertaCosecha(3, topicAlertaSector3);
    flagGlobalActivarAlerta = 1;
  } /* else {
    fechaCultivo1 = "a";
    fechaCultivo2 = "b";
    fechaCultivo3 = "c";
    flagGlobalActivarAlerta = 0;
  }*/
}

void alertaCosecha(int i, char* topic) {
  if (flagGlobalActivarAlerta == 1) {
    switch (i) {
      case 1:
        enviarAlertas(1, topic);
        flagGlobalActivarAlerta = 0;
        break;
      case 2:
        enviarAlertas(1, topic);
        flagGlobalActivarAlerta = 0;
        break;
      case 3:
        enviarAlertas(1, topic);
        flagGlobalActivarAlerta = 0;
        break;
    }
  }
}

void enviarAlertas(int dato, char* topic) {
  String dato1 = String(dato);
  char a[5];
  dato1.toCharArray(a, 5);  //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
  Serial.println(a);
  MQTT_CLIENT.publish(topic, a);  //Envia la informacion dentro del arreglo char

  fechaCultivo1 = "a";
  fechaCultivo2 = "b";
  fechaCultivo3 = "c";
  flagGlobalActivarAlerta = 0;
}

String concatenarfecha(String Fecha) {
  int dia, mes, anio;
  String var;
  String dato;

  var = Fecha.substring(0, 4);
  dia = var.toInt();
  var = Fecha.substring(4, 8);
  mes = var.toInt();
  var = Fecha.substring(8, 12);
  anio = var.toInt();

  dato.concat(dia);
  dato.concat(mes);
  dato.concat(anio);

  return dato;
}

//Envia la fecha de cosecha segun el cultivo elegido
void seleccion_cultivo(int eleccion, String Fecha, char* topic) {
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
      enviarDatosCosecha(fechaCosechada, topic);
      break;
    case 2:         //Eleccion de Cebollas
      cantMes = 4;  //Se cosecha luego de los 4 meses
      Serial.println("Comenzando el calculo de la cosecha de Tomates...");
      Serial.println("  ");
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.print("Fecha de cosecha: ");
      Serial.println(fechaCosechada);
      enviarDatosCosecha(fechaCosechada, topic);
      break;

    case 3:         //Eleccion de Lechuga
      cantMes = 3;  //Se cosecha luego de los 3 meses
      Serial.println("Comenzando el calculo de la cosecha de Tomates...");
      Serial.println("  ");
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.print("Fecha de cosecha: ");
      Serial.println(fechaCosechada);
      enviarDatosCosecha(fechaCosechada, topic);
      break;

    case 4:         //Eleccion de Zanahoria
      cantMes = 3;  //Se cosecha luego de los 3 meses
      Serial.println("Comenzando el calculo de la cosecha de Tomates...");
      Serial.println("  ");
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.print("Fecha de cosecha: ");
      Serial.println(fechaCosechada);
      enviarDatosCosecha(fechaCosechada, topic);
      break;

    case 5:         //Eleccion de Pimiento
      cantMes = 5;  //Se cosecha luego de los 5 meses
      Serial.println("Comenzando el calculo de la cosecha de Tomates...");
      Serial.println("  ");
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.print("Fecha de cosecha: ");
      Serial.println(fechaCosechada);
      enviarDatosCosecha(fechaCosechada, topic);
      break;
  }
}

void enviarDatosCosecha(String fechaCosecha, char* topic) {
  char a[12];
  fechaCosecha.toCharArray(a, 12);  //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
  Serial.print("Dato enviado: ");
  Serial.println(a);
  MQTT_CLIENT.publish(topic, a);  //Envia la informacion dentro del arreglo char
  delay(500);
}

//Separa y divide la fecha para sumarle la cantidad de dias corrrespondientes para su cosecha - Retorna el la fecha de cosecha
String fecha_cosecha(String Fecha, int cantMes) {
  int dia, mes, anio;
  String var;
  String dato;

  var = Fecha.substring(0, 4);
  dia = var.toInt();
  var = Fecha.substring(4, 8);
  mes = var.toInt();
  var = Fecha.substring(8, 12);
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

//Segun el flag activada se activan diferentes sensores a su correspondiente sector
void menu_sector(int dato) {
  float lectura_fotoresistencia, lectura_sensorTemp;
  int lectura_sensorSuelo1, lectura_sensorSuelo2, lectura_sensorSuelo3;

  char* topic_Sensor1;
  char* topic_Sensor2;
  char* topic_Sensor3;

  switch (dato) {
    case 1:  //Se habilita el sector N° 1
      //Habilitamos los sensores del sector N° 1
      lectura_fotoresistencia = (analogRead(fotoresistencia) / 4095) * 100;  // PIN 39
      lectura_sensorSuelo1 = analogRead(sensorSuelo1);
      lectura_sensorTemp = analogRead(sensorTemp) * 0.1;  //  10mv/°C PIN 33

      topic_Sensor1 = "Inf/SensorTemperatura";
      topic_Sensor2 = "Inf/SensoHumedad1";
      topic_Sensor3 = "Inf/CantLuz";

      enviar_datoSensor_MQTT(lectura_sensorTemp, lectura_fotoresistencia, lectura_sensorSuelo1, topic_Sensor1, topic_Sensor2, topic_Sensor3);
      break;
  }
}

//Envio de datos por MQTT a App Inventor
void enviar_datoSensor_MQTT(float lecturaTemp, float lecturaLuz, int lecturaSuelo, char* topic_Sensor1, char* topic_Sensor2, char* topic_Sensor3) {
  static unsigned long lastMillis_publish_1 = millis();  //Variable a guardar el tiempo de millis
  static unsigned long lastMillis_publish_2 = millis();  //Variable a guardar el tiempo de millis
  static unsigned long lastMillis_publish_3 = millis();  //Variable a guardar el tiempo de millis

  if (millis() - lastMillis_publish_1 >= 2000) {
    lastMillis_publish_1 = millis();
    String dato1 = String(lecturaTemp);  //Se convierte el tipo de variable de int a String
    char a[5];
    dato1.toCharArray(a, 5);  //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
    //Serial.print("Valores de Temperatura: ");
    //Serial.println(a);
    MQTT_CLIENT.publish(topic_Sensor1, a);  //Envia la informacion dentro del arreglo char
  }
  if (millis() - lastMillis_publish_2 >= 2500) {
    lastMillis_publish_2 = millis();
    String dato2 = String(lecturaSuelo);  //Se convierte el tipo de variable de int a String
    char b[1];
    dato2.toCharArray(b, 12);  //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
    //Serial.print("Valores de Humedad: ");
    //Serial.println(b);
    MQTT_CLIENT.publish(topic_Sensor2, b);  //Envia la informacion dentro del arreglo char
  }
  if (millis() - lastMillis_publish_3 >= 3000) {
    lastMillis_publish_3 = millis();
    String dato3 = String(lecturaLuz);  //Se convierte el tipo de variable de int a String
    char c[1];
    dato3.toCharArray(c, 12);  //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
    //Serial.print("Valores de Luz: ");
    //Serial.println(c);
    MQTT_CLIENT.publish(topic_Sensor3, c);  //Envia la informacion dentro del arreglo char
  }
}

// Reconecta con MQTT broker
void reconnect() {
  MQTT_CLIENT.setServer("broker.hivemq.com", 1883);  //IP y Puerto Utilizados
  MQTT_CLIENT.setClient(WIFI_CLIENT);

  // Intentando conectar con el broker
  while (!MQTT_CLIENT.connected()) {
    Serial.println("Intentando conectar con Broker MQTT.");
    MQTT_CLIENT.connect("LeoIsleno");

    MQTT_CLIENT.subscribe("State/Sector1");  // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("State/Sector2");  // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("State/Sector3");  // Aca realiza la suscripcion

    //    Sector 1
    MQTT_CLIENT.subscribe("Tomates/Fecha1");    // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Cebollas/Fecha1");   // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Lechuga/Fecha1");    // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Zanahoria/Fecha1");  // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Pimiento/Fecha1");   // Aca realiza la suscripcion

    //    Sector 2
    MQTT_CLIENT.subscribe("Tomates/Fecha2");    // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Cebollas/Fecha2");   // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Lechuga/Fecha2");    // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Zanahoria/Fecha2");  // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Pimiento/Fecha2");   // Aca realiza la suscripcion

    //    Sector 3
    MQTT_CLIENT.subscribe("Tomates/Fecha3");    // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Cebollas/Fecha3");   // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Lechuga/Fecha3");    // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Zanahoria/Fecha3");  // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Pimiento/Fecha3");   // Aca realiza la suscripcion


    MQTT_CLIENT.subscribe("Comprobar/FechaHoy");  // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Comprobar/Cultivo1");  // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Comprobar/Cultivo2");  // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Comprobar/Cultivo3");  // Aca realiza la suscripcion

    // Espera para que conecte denuevo
    delay(3000);
  }
  Serial.println("Conectado a MQTT");
}