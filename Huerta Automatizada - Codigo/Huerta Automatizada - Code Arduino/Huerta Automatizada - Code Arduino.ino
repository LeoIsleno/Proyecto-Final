/*
 * Autor: Leonardo Isleño - ITS VILLADA 
 * 
 * 20/11/22 - Funcionalidad de los 3 sectores correcta en el momento de cosecha.
 * 21/11/11 - Optimizacion del codigo, preparando para el uso de Reles.
 *          - Pruebas de EEPROM exitosas.
 * 22/11/22 - EEPROM para gestionar el tiempo de accionamiento de los reles para activar la LUZ, en sector uno funcionando - Falta desctivar el sector una vez cosechado

 PROBLEMA: Al utilizar EEPROM el programa se desconfigura, no me deja comparar fechas.
*/

#include <WiFi.h>          // Para el ESP32
#include <EEPROM.h>        // Libreria para utilizar EEPROM
#include <PubSubClient.h>  // Libreria para metodo MQTT
#include <MQTT.h>
WiFiClient WIFI_CLIENT;  // Libreria Wifi
PubSubClient MQTT_CLIENT;

#define EEPROM_SIZE 64

#define fotoresistencia 39
#define sensorSuelo3 35
#define sensorSuelo2 34
#define sensorSuelo1 32
#define sensorTemp 33

#define relaySector1_LUZ 15
#define relaySector2_LUZ 2
#define relaySector3_LUZ 4

const char* ssid = "Leouu";          //Nombre del Wifi
const char* password = "leo12345S";  //Contraseña del wifi

//const int relay = 19;

bool StateSectorActivate = 0, flagFechaComprobar = 0, flagActivarCFC1 = 0, flagActivarCFC2 = 0, flagActivarCFC3 = 0, flagActivarCFC = 0, flagGlobalActivarAlerta = 0, flagStateSector1 = 0, flagStateSector2 = 0, flagStateSector3 = 0;
String fechaHoy = "a", fechaCultivo1 = "b", fechaCultivo2 = "c", fechaCultivo3 = "d";

bool relayStateSectorLuz[3] = { 0, 0, 0 };  //Sectores de Luz desactivados al inicializar - (Sector 1, Sector 2, Sector 3)

static bool luzDiaSectores[3] = { 0, 0, 0 }, flagDesactivarSectores[3] = { 0, 0, 0 };
static int cantHorasActivado[3] = { 0, 0, 0 }, cantHorasDesactivado[3] = { 0, 0, 0 };

bool estado = 0;

//void recived(String topic, String valor);

void setup() {
  /*Iniciamos el terminal Serial a una velocidad de 115200, junto a un retardo de 1 segundo y definimos los pines a utilizar*/
  Serial.begin(115200);  //Inicializa el serial begin a 115200

  EEPROM.begin(EEPROM_SIZE);  //Se inicializa EEPROM

  //Asignamos las variables cargadas anteriormente
  luzDiaSectores[1] = EEPROM.read(0);
  //luzDiaSectores[2] = EEPROM.read(1);
  //luzDiaSectores[3] = EEPROM.read(2);

  //Asignamos las variables cargadas anteriormente
  flagDesactivarSectores[1] = EEPROM.read(3);
  //flagDesactivarSectores[2] = EEPROM.read(4);
  //flagDesactivarSectores[3] = EEPROM.read(5);

  //Asignamos las variables cargadas anteriormente
  cantHorasActivado[1] = EEPROM.read(6);
  //cantHorasActivado[2] = EEPROM.read(7);
  //cantHorasActivado[3] = EEPROM.read(8);

  //Asignamos las variables cargadas anteriormente
  cantHorasDesactivado[1] = EEPROM.read(9);
  //cantHorasDesactivado[2] = EEPROM.read(10);
  //cantHorasDesactivado[3] = EEPROM.read(11);

  //Sectores Activados - Activar Reles
  relayStateSectorLuz[1] = EEPROM.read(12);
  //relayStateSectorLuz[2] = EEPROM.read(13);
  //relayStateSectorLuz[3] = EEPROM.read(14);

  //Se define los pines de los reles como salidas
  pinMode(relaySector1_LUZ, OUTPUT);
  pinMode(relaySector2_LUZ, OUTPUT);
  pinMode(relaySector3_LUZ, OUTPUT);

  //Inicializamos los reles con el ultimo estado
  digitalWrite(relaySector1_LUZ, luzDiaSectores[1]);
  digitalWrite(relaySector2_LUZ, luzDiaSectores[2]);
  digitalWrite(relaySector3_LUZ, luzDiaSectores[3]);

  delay(1000);  // 1 segundo
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
  menu_sector(StateSectorActivate);
  MQTT_CLIENT.loop();  // Testea la suscripcion
}

//Segun lo recibido desde la App segun el topic y mensaje recibido actua
void recived(String topic, String valor) {
  int flagStateTomates;
  int eleccion = 0;
  char* enviarTopic1 = "Fecha/cosecha1";
  char* enviarTopic2 = "Fecha/cosecha2";
  char* enviarTopic3 = "Fecha/cosecha3";
/*
  if (topic = "Reles/Sector1") {
    if (valor == "1") {
      relayStateSectorLuz[1] = 1;

      EEPROM.put(12, relayStateSectorLuz[1]);  //Direccion - Variable
      EEPROM.commit();                         //Confirmar
    } else {
      relayStateSectorLuz[1] = 0;

      EEPROM.put(12, relayStateSectorLuz[1]);  //Direccion - Variable
      EEPROM.commit();
    }
  }

  if (topic = "Reles/Sector2") {
    if (valor == "1") {
      relayStateSectorLuz[2] = 1;

      EEPROM.put(13, relayStateSectorLuz[1]);  //Direccion - Variable
      EEPROM.commit();                         //Confirmar
    } else {
      relayStateSectorLuz[2] = 0;

      EEPROM.put(13, relayStateSectorLuz[1]);  //Direccion - Variable
      EEPROM.commit();
    }
  }

  if (topic = "Reles/Sector3") {
    if (valor == "1") {
      relayStateSectorLuz[3] = 1;

      EEPROM.put(14, relayStateSectorLuz[1]);  //Direccion - Variable
      EEPROM.commit();                         //Confirmar
    } else {
      relayStateSectorLuz[3] = 0;
      EEPROM.put(14, relayStateSectorLuz[1]);  //Direccion - Variable
      EEPROM.commit();
    }
  }
*/

  if (topic == "State/Sector1") {
    if (valor == "1") {
      StateSectorActivate = 1;  //Cambia el estado de Seccion - Activa el sector 1
    } else {
      StateSectorActivate = 0;  //Cambia el estado de Seccion - Desactiva el sector 1
    }
  }

  if (topic == "State/Sector2") {
    if (valor == "1") {
      StateSectorActivate = 2;  //Cambia el estado de Seccion - Activa el sector 1
    } else {
      StateSectorActivate = 0;  //Cambia el estado de Seccion - Desactiva el sector 1
    }
  }

  if (topic == "State/Sector3") {
    if (valor == "1") {
      StateSectorActivate = 3;  //Cambia el estado de Seccion - Activa el sector 1
    } else {
      StateSectorActivate = 0;  //Cambia el estado de Seccion - Desactiva el sector 1
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
    seleccion_cultivo(eleccion, valor, enviarTopic2);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
  }

  if (topic == "Tomates/Fecha3") {
    eleccion = 1;
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
  fechaCultivo2 = concatenarfecha(fechaHoy);
  fechaCultivo3 = concatenarfecha(fechaCultivo3);
  fechaHoy = concatenarfecha(fechaHoy);

  Serial.println(fechaCultivo1);
  Serial.println(fechaHoy);

  if (fechaCultivo1 == fechaHoy) {
    Serial.println("Alerta de Cosecha al sector 1");
    alertaCosecha(1, topicAlertaSector1);
    flagGlobalActivarAlerta = 1;
  } else if (fechaCultivo2 == fechaHoy) {
    Serial.println("Alerta de Cosecha al sector 2");
    alertaCosecha(2, topicAlertaSector2);
    flagGlobalActivarAlerta = 1;
  } else if (fechaCultivo3 == fechaHoy) {
    Serial.println("Alerta de Cosecha al sector 3");
    alertaCosecha(3, topicAlertaSector3);
    flagGlobalActivarAlerta = 1;
  }
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
  //int lectura_sensorSuelo1, lectura_sensorSuelo2, lectura_sensorSuelo3, lectura_fotoresistencia, lectura_sensorTemp;

  int sensores[5] = { 0, 0, 0, 0, 0 };  //Temperatura, fotoresistencia, Sensor suelo1, Sensor suelo2, Sensor suelo3,

  sensores[1] = analogRead(sensorTemp) * 0.1;                //  10mv/°C PIN 33
  sensores[2] = (analogRead(fotoresistencia) / 4095) * 100;  // PIN 39
  sensores[3] = analogRead(sensorSuelo1);
  sensores[4] = analogRead(sensorSuelo2);
  sensores[5] = analogRead(sensorSuelo3);

  char* topic_Sensor1 = "";
  char* topic_Sensor2 = "";
  char* topic_Sensor3 = "";

  switch (dato) {
    case 1:  //Se habilita el sector N° 1
      //Habilitamos los sensores del sector N° 1 para mostrarlos:
      Serial.print("Activado 1 ");
      topic_Sensor1 = "Inf/SensorTemperatura";
      topic_Sensor2 = "Inf/SensoHumedad1";
      topic_Sensor3 = "Inf/CantLuz";

      //activarRelesSector(sensores[1], sensores[2], sensores[0]);
      enviar_datoSensor_MQTT(sensores[0], sensores[1], sensores[2], topic_Sensor1, topic_Sensor2, topic_Sensor3);
      break;
  }

  //activarRelesSector(sensores);
}

void activarRelesSector(int array[]) {
  int temp = *(array + 1);       //Sensor Temperatura
  int Fres = *(array + 2);       //Sensor Fotoresistencia
  int senSuelo1 = *(array + 3);  //Sensor de Suelo 1
  int senSuelo2 = *(array + 4);  //Sensor de Suelo 2
  int senSuelo3 = *(array + 5);  //Sensor de Suelo 3

  static unsigned long TactivoSector1 = millis();     //Variable a guardar el tiempo de millis
  static unsigned long TdesactivoSector1 = millis();  //Variable a guardar el tiempo de millis

  // Tomates

  if (relayStateSectorLuz[1] == 1) {

    if (flagDesactivarSectores[1] == 0) {
      luzDiaSectores[1] = 1;
      if (millis() - TactivoSector1 >= 2000) {
        TactivoSector1 = millis();
        cantHorasActivado[1] = cantHorasActivado[1] + 1;
        Serial.print("Horas Activo: ");
        Serial.println(cantHorasActivado[1]);

        //Contador de horas Activado
        EEPROM.put(6, cantHorasActivado[1]);  //Direccion - Variable
        EEPROM.commit();                      //Confirmar

        //Bandera para el contador de tiempo encendido
        EEPROM.put(3, flagDesactivarSectores[1]);  //Direccion - Variable
        EEPROM.commit();                           //Confirmar

        //Bandera de encendido para el sector 1
        EEPROM.put(0, luzDiaSectores[1]);  //Direccion - Variable
        EEPROM.commit();                   //Confirmar
        digitalWrite(relaySector1_LUZ, luzDiaSectores[1]);
      }
      if (cantHorasActivado[1] >= 5) {

        Serial.print("Ciclo Cumplido");
        flagDesactivarSectores[1] = 1;
        cantHorasActivado[1] = 0;

        //Contador de horas Activado
        EEPROM.put(6, cantHorasActivado[1]);  //Direccion - Variable
        EEPROM.commit();                      //Confirmar

        //Bandera para el contador de tiempo encendido
        EEPROM.put(3, flagDesactivarSectores[1]);  //Direccion - Variable
        EEPROM.commit();                           //Confirmar

        //Bandera de encendido para el sector 1
        EEPROM.put(0, luzDiaSectores[1]);  //Direccion - Variable
        EEPROM.commit();                   //Confirmar
        digitalWrite(relaySector1_LUZ, luzDiaSectores[1]);
      }
    }

    if (flagDesactivarSectores[1] == 1) {
      luzDiaSectores[1] = 0;
      if (millis() - TdesactivoSector1 >= 2000) {
        TdesactivoSector1 = millis();
        cantHorasDesactivado[1] = cantHorasDesactivado[1] + 1;
        Serial.print("Horas Desactivado: ");
        Serial.println(cantHorasDesactivado[1]);

        //Contador de horas desactivado
        EEPROM.put(9, cantHorasDesactivado[1]);  //Direccion - Variable
        EEPROM.commit();                         //Confirmar

        //Bandera para el contador de tiempo encendido
        EEPROM.put(3, flagDesactivarSectores[1]);  //Direccion - Variable
        EEPROM.commit();                           //Confirmar

        //Bandera de encendido para el sector 1
        EEPROM.put(0, luzDiaSectores[1]);  //Direccion - Variable
        EEPROM.commit();                   //Confirmar

        digitalWrite(relaySector1_LUZ, luzDiaSectores[1]);
      }

      if (cantHorasDesactivado[1] == 5) {
        Serial.println("Ciclo desactivado Cumplido");
        flagDesactivarSectores[1] = 0;  //Estado : Apagado
        cantHorasDesactivado[1] = 0;

        //Contador de horas desactivado
        EEPROM.put(9, cantHorasDesactivado[1]);  //Direccion - Variable
        EEPROM.commit();                         //Confirmar

        //Bandera para el contador de tiempo encendido/apagado
        EEPROM.put(3, flagDesactivarSectores[1]);  //Direccion - Variable
        EEPROM.commit();                           //Confirmar

        //Bandera de encendido para el sector 1
        EEPROM.put(0, luzDiaSectores[1]);  //Direccion - Variable
        EEPROM.commit();                   //Confirmar

        digitalWrite(relaySector1_LUZ, luzDiaSectores[1]);
      }
    }
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
/*
    MQTT_CLIENT.subscribe("Reles/Sector1");  // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Reles/Sector2");  // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Reles/Sector3");  // Aca realiza la suscripcion
*/
    // Espera para que conecte denuevo
    delay(3000);
  }
  Serial.println("Conectado a MQTT");
}