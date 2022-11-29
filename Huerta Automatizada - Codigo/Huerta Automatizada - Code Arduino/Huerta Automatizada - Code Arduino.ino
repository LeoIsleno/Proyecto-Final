/*
 * Autor: Leonardo Isleño - ITS VILLADA 
 * 
 * 20/11/22 - Funcionalidad de los 3 sectores correcta en el momento de cosecha.
 * 21/11/11 - Optimizacion del codigo, preparando para el uso de Reles.
 *          - Pruebas de EEPROM exitosas.
 * 22/11/22 - EEPROM para gestionar el tiempo de accionamiento de los reles para activar la LUZ, en sector uno funcionando - Falta desctivar el sector una vez cosechado

 * 25/11/22 - PROBLEMA: Al utilizar EEPROM el programa se desconfigura, no me deja comparar fechas (SOLUCIONADO)
            - Los 3 sectores funcionando correctamente con los reles de LUZ
            - Problemas solucionado: 
              - Error al comparar fecha
              - No devolvia la fecha del dia
              - Error con substring (al comparar la cantidad de caracteres las fechas de cosecha)

 * 29/11/22 - Testear sensores de humedad
            - Probar Electrovalvulas
            - Crear sistema que detecta la cantidad de luz solar que reciben las plantas por dia, en base a eso, realizar calculos para compensar la luz natural artificial con la natural.
            - FASE DE PRUEBAS
*/

#include <WiFi.h>          // Para el ESP32
#include <EEPROM.h>        // Libreria para utilizar EEPROM
#include <PubSubClient.h>  // Libreria para metodo MQTT
#include <MQTT.h>
WiFiClient WIFI_CLIENT;  // Libreria Wifi
PubSubClient MQTT_CLIENT;

#define EEPROM_SIZE 64

#define sensorAgua 36
#define fotoresistencia 39
#define sensorSuelo3 35
#define sensorSuelo2 34
#define sensorSuelo1 32
#define sensorTemp 33

#define relaySector1_LUZ 15
#define relaySector2_LUZ 2
#define relaySector3_LUZ 4

#define relaySector1_ELEC 23
#define relaySector2_ELEC 5
#define relaySector3_ELEC 21

const char* ssid = "Leouu";          //Nombre del Wifi
const char* password = "leo12345S";  //Contraseña del wifi

//const int relay = 19;

bool StateSectorActivate = 0, flagFechaComprobar = 0, flagActivarCFC1 = 0, flagActivarCFC2 = 0, flagActivarCFC3 = 0, flagActivarCFC = 0, flagGlobalActivarAlerta = 0, flagStateSector1 = 0, flagStateSector2 = 0, flagStateSector3 = 0;
String fechaHoy = "a", fechaCultivo1 = "b", fechaCultivo2 = "c", fechaCultivo3 = "d";

bool relayStateSectorLuz[3] = { 0, 0, 0 };  //Sectores de Luz desactivados al inicializar - (Sector 1, Sector 2, Sector 3)

static bool luzDiaSectores[3] = { 0, 0, 0 }, flagDesactivarSectores[3] = { 0, 0, 0 };
static int cantHorasActivado[4] = { 0, 0, 0, 0 }, cantHorasDesactivado[3] = { 0, 0, 0 }, tiempoCosecha[3] = { 0, 0, 0 }, cantHumedad[3] = { 0, 0, 0 };

int eleccion = 0;
bool estado = 0;

//void recived(String topic, String valor);

void setup() {
  /*Iniciamos el terminal Serial a una velocidad de 115200, junto a un retardo de 1 segundo y definimos los pines a utilizar*/
  Serial.begin(115200);  //Inicializa el serial begin a 115200

  EEPROM.begin(EEPROM_SIZE);  //Se inicializa EEPROM
                              /*                
  for(int i=0; i<=50;i++){
    EEPROM.write(i,0);
    Serial.println("Numero: ");
    Serial.print(i);
    Serial.println(" eliminado.");
  }*/

  //Asignamos las variables cargadas anteriormente
  luzDiaSectores[1] = EEPROM.read(0);
  luzDiaSectores[2] = EEPROM.read(1);
  luzDiaSectores[3] = EEPROM.read(2);

  //Asignamos las variables cargadas anteriormente
  flagDesactivarSectores[1] = EEPROM.read(3);
  flagDesactivarSectores[2] = EEPROM.read(4);
  flagDesactivarSectores[3] = EEPROM.read(5);

  //Asignamos las variables cargadas anteriormente
  cantHorasActivado[1] = EEPROM.read(6);
  cantHorasActivado[2] = EEPROM.read(7);
  cantHorasActivado[3] = EEPROM.read(8);

  //Asignamos las variables cargadas anteriormente
  cantHorasDesactivado[1] = EEPROM.read(9);
  cantHorasDesactivado[2] = EEPROM.read(10);
  cantHorasDesactivado[3] = EEPROM.read(11);

  //Sectores Activados - Activar Reles
  relayStateSectorLuz[1] = EEPROM.read(20);
  relayStateSectorLuz[2] = EEPROM.read(25);
  relayStateSectorLuz[3] = EEPROM.read(30);

  //Asignamos los valores de tiempo de cosecha para cada sector
  tiempoCosecha[1] = EEPROM.read(15);
  tiempoCosecha[2] = EEPROM.read(16);
  tiempoCosecha[3] = EEPROM.read(17);

  //Contador de Cantidad de Luz natural que recibe la Huerta
  cantHorasActivado[4] = EEPROM.read(18);

  Serial.println("Estado de Sector 1: ");
  Serial.println(relayStateSectorLuz[1]);
  Serial.println("Estado de Sector 2: ");
  Serial.println(relayStateSectorLuz[2]);
  Serial.println("Estado de Sector 3: ");
  Serial.println(relayStateSectorLuz[3]);

  //Se define los pines de los reles como salidas
  pinMode(relaySector1_LUZ, OUTPUT);
  pinMode(relaySector2_LUZ, OUTPUT);
  pinMode(relaySector3_LUZ, OUTPUT);

  pinMode(relaySector1_ELEC, OUTPUT);
  pinMode(relaySector2_ELEC, OUTPUT);
  pinMode(relaySector3_ELEC, OUTPUT);

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

  Serial.println("   ");
  Serial.print("Message received: ");
  Serial.print(recibido);

  String recibidoStr = String(recibido);

  for (int i = 0; i < length; i++) {
    receivedInt = (char)payload[i] - '0';  //Convierte de Char to int
    (var).concat(receivedInt);             // Une todo el codigo segmentado en uno mismo y lo guarda como Int, de momento no se utiliza al no tener gran variedad de plantas a utilizar
  }

  Serial.print(" Codigo concatenado: ");
  Serial.print(var);
  Serial.println("  ");

  recived(recibidoStr, var);  //Topic - Valor recibido
}

//Main
void loop() {
  if (!MQTT_CLIENT.connected()) {
    reconnect();
  }
  /*
  digitalWrite(5, 0);
  Serial.println("Rele Sector 2 activado");
  delay(2000);
  digitalWrite(5, 1);
  Serial.println("Rele Sector 2 desactivado");
  delay(2000);
*/


  menu_sector(StateSectorActivate);
  MQTT_CLIENT.loop();  // Testea la suscripcion*/
}

//Segun lo recibido desde la App segun el topic y mensaje recibido actua
void recived(String topic, String valor) {
  int flagStateTomates;

  char* enviarTopic1 = "Fecha/cosecha1";
  char* enviarTopic2 = "Fecha/cosecha2";
  char* enviarTopic3 = "Fecha/cosecha3";

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


  if (flagActivarCFC3) {

    flagActivarCFC = 0;
    flagActivarCFC1 = 0;
    flagActivarCFC2 = 0;
    flagActivarCFC3 = 0;

    comprobarFechasCosechas(fechaHoy, fechaCultivo1, fechaCultivo2, fechaCultivo3);
  }

  if (topic == "Tomates/Fecha1") {
    eleccion = 1;
    tiempoCosecha[1] = seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[1] = 60;
  }

  if (topic == "Tomates/Fecha2") {
    eleccion = 1;
    tiempoCosecha[2] = seleccion_cultivo(eleccion, valor, enviarTopic2);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[2] = 60;
  }

  if (topic == "Tomates/Fecha3") {
    eleccion = 1;
    tiempoCosecha[3] = seleccion_cultivo(eleccion, valor, enviarTopic3);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[3] = 60;
  }

  if (topic == "Cebollas/Fecha1") {
    eleccion = 2;
    tiempoCosecha[1] = seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[1] = 45;
  }

  if (topic == "Cebollas/Fecha2") {
    eleccion = 2;
    tiempoCosecha[2] = seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[2] = 45;
  }

  if (topic == "Cebollas/Fecha3") {
    eleccion = 2;
    tiempoCosecha[3] = seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[3] = 45;
  }

  if (topic == "Lechuga/Fecha1") {
    eleccion = 3;
    tiempoCosecha[1] = seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[1] = 55;
  }

  if (topic == "Lechuga/Fecha2") {
    eleccion = 3;
    tiempoCosecha[2] = seleccion_cultivo(eleccion, valor, enviarTopic2);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[2] = 55;
  }

  if (topic == "Lechuga/Fecha3") {
    eleccion = 3;
    tiempoCosecha[3] = seleccion_cultivo(eleccion, valor, enviarTopic3);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[3] = 55;
  }

  if (topic == "Zanahoria/Fecha1") {
    eleccion = 4;
    tiempoCosecha[1] = seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[1] = 65;
  }

  if (topic == "Zanahoria/Fecha2") {
    eleccion = 4;
    tiempoCosecha[2] = seleccion_cultivo(eleccion, valor, enviarTopic2);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[2] = 65;
  }

  if (topic == "Zanahoria/Fecha3") {
    eleccion = 4;
    tiempoCosecha[3] = seleccion_cultivo(eleccion, valor, enviarTopic3);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[3] = 65;
  }

  if (topic == "Pimiento/Fecha1") {
    eleccion = 5;
    tiempoCosecha[1] = seleccion_cultivo(eleccion, valor, enviarTopic1);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[1] = 70;
  }

  if (topic == "Pimiento/Fecha2") {
    eleccion = 5;
    tiempoCosecha[2] = seleccion_cultivo(eleccion, valor, enviarTopic2);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[2] = 70;
  }

  if (topic == "Pimiento/Fecha3") {
    eleccion = 5;
    tiempoCosecha[3] = seleccion_cultivo(eleccion, valor, enviarTopic3);  //Eleccion de planta a cultivar - Fecha de cultivo a calcular cosecha
    cantHumedad[3] = 70;
  }

  if (topic == "Reles/Sectores1") {
    if (valor == "1") {
      relayStateSectorLuz[1] = 1;
      Serial.println("Rele Sector 1 activado");
      Serial.println("Estado de Sector 1: ");
      Serial.println(valor);
      guardarEEPROM(20, relayStateSectorLuz[1]);
    } else if (valor == "0") {
      relayStateSectorLuz[1] = 0;
      Serial.println("Rele Sector 1 desactivado");
      guardarEEPROM(20, relayStateSectorLuz[1]);
    }
  }

  if (topic == "Reles/Sectores2") {

    if (valor == "1") {
      relayStateSectorLuz[2] = 1;
      Serial.println("Rele Sector 2 ACTIVADO");
      Serial.println("Estado de Sector 2: ");
      Serial.println(valor);
      guardarEEPROM(25, relayStateSectorLuz[2]);
    } else if (valor == "0") {
      relayStateSectorLuz[2] = 0;
      Serial.println("Rele Sector 2 DESACTIVADO");
      guardarEEPROM(25, relayStateSectorLuz[2]);
    }
  }

  if (topic == "Reles/Sectores3") {
    if (valor == "1") {
      relayStateSectorLuz[3] = 1;
      Serial.println("Rele Sector 3 ACTIVADO");
      Serial.println("Estado de Sector 3: ");
      Serial.println(valor);
      guardarEEPROM(30, relayStateSectorLuz[3]);
    } else if (valor == "0") {
      relayStateSectorLuz[3] = 0;
      Serial.println("Rele Sector 3 desactivado");
      guardarEEPROM(30, relayStateSectorLuz[3]);
    }
  }
}

void comprobarFechasCosechas(String fechaHoy, String fechaCultivo1, String fechaCultivo2, String fechaCultivo3) {
  char* topicAlertaSector1 = "alerta/Sector1";
  char* topicAlertaSector2 = "alerta/Sector2";
  char* topicAlertaSector3 = "alerta/Sector3";

  fechaCultivo1 = concatenarfecha(fechaCultivo1);
  fechaCultivo2 = concatenarfecha(fechaCultivo2);
  fechaCultivo3 = concatenarfecha(fechaCultivo3);
  fechaHoy = concatenarfecha(fechaHoy);

  Serial.print("Fecha de cultivo de Sector 1: ");
  Serial.println(fechaCultivo1);
  Serial.print("Fecha de cultivo de Sector 2: ");
  Serial.println(fechaCultivo2);
  Serial.print("Fecha de cultivo de Sector 3: ");
  Serial.println(fechaCultivo3);
  Serial.print("Fecha de hoy: ");
  Serial.println(fechaHoy);

  if (fechaCultivo1 == fechaHoy) {
    Serial.println("Alerta de Cosecha al sector 1");
    alertaCosecha(1, topicAlertaSector1);
  }
  if (fechaCultivo2 == fechaHoy) {
    Serial.println("Alerta de Cosecha al sector 2");
    alertaCosecha(2, topicAlertaSector2);
  }
  if (fechaCultivo3 == fechaHoy) {
    Serial.println("Alerta de Cosecha al sector 3");
    alertaCosecha(3, topicAlertaSector3);
  }
}

void alertaCosecha(int i, char* topic) {
  switch (i) {
    case 1:
      enviarAlertas(1, topic);
      break;
    case 2:
      enviarAlertas(1, topic);
      break;
    case 3:
      enviarAlertas(1, topic);
      break;
  }
}

void enviarAlertas(int dato, char* topic) {
  String dato1 = String(dato);
  char a[5];
  dato1.toCharArray(a, 5);  //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
  Serial.println(a);
  MQTT_CLIENT.publish(topic, a);  //Envia la informacion dentro del arreglo char
  /*
  fechaCultivo1 = "a";
  fechaCultivo2 = "b";
  fechaCultivo3 = "c";
  flagGlobalActivarAlerta = 0;*/
}

String concatenarfecha(String Fecha) {
  Serial.print("Fecha a concatenar: ");
  Serial.println(Fecha);

  int dia, mes, anio, cantCaracteres;
  String var;
  String dato;

  cantCaracteres = Fecha.length();
  Serial.print("Cantidad de caracteres: ");
  Serial.println(cantCaracteres);

  if (cantCaracteres == 10) {
    var = Fecha.substring(0, 4);
    dia = var.toInt();
    var = Fecha.substring(4, 8);
    mes = var.toInt();
    var = Fecha.substring(8, 10);
    anio = var.toInt();

    dato.concat(dia);
    dato.concat(mes);
    dato.concat(anio);
  } else if (cantCaracteres == 9) {

    var = Fecha.substring(0, 4);
    dia = var.toInt();
    var = Fecha.substring(4, 7);
    mes = var.toInt();
    var = Fecha.substring(7, 9);
    anio = var.toInt();

    dato.concat(dia);
    dato.concat(mes);
    dato.concat(anio);
  }


  return dato;
}

//Envia la fecha de cosecha segun el cultivo elegido
int seleccion_cultivo(int eleccion, String Fecha, char* topic) {
  int cantMes;
  int tiempoCosecha = 0;
  String fechaCosechada;

  switch (eleccion) {
    case 1:         //Eleccion de Tomates
      cantMes = 4;  //Se cosecha luego de los 4 meses
      tiempoCosecha = 7;
      Serial.println("Comenzando el calculo de la cosecha de Tomates...");
      Serial.println("  ");
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.print("Fecha de cosecha: ");
      Serial.println(fechaCosechada);
      enviarDatosCosecha(fechaCosechada, topic);

      setValoresEEPROM();

      return tiempoCosecha;
      break;
    case 2:               //Eleccion de Cebollas
      cantMes = 4;        //Se cosecha luego de los 4 meses
      tiempoCosecha = 5;  //En horas
      Serial.println("Comenzando el calculo de la cosecha de Cebollas...");
      Serial.println("  ");
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.print("Fecha de cosecha: ");
      Serial.println(fechaCosechada);
      enviarDatosCosecha(fechaCosechada, topic);

      setValoresEEPROM();

      return tiempoCosecha;
      break;

    case 3:         //Eleccion de Lechuga
      cantMes = 3;  //Se cosecha luego de los 3 meses
      tiempoCosecha = 4;
      Serial.println("Comenzando el calculo de la cosecha de Lechuga...");
      Serial.println("  ");
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.print("Fecha de cosecha: ");
      Serial.println(fechaCosechada);
      enviarDatosCosecha(fechaCosechada, topic);

      setValoresEEPROM();

      return tiempoCosecha;
      break;

    case 4:         //Eleccion de Zanahoria
      cantMes = 3;  //Se cosecha luego de los 3 meses
      tiempoCosecha = 5;
      Serial.println("Comenzando el calculo de la cosecha de Zanahoria...");
      Serial.println("  ");
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.print("Fecha de cosecha: ");
      Serial.println(fechaCosechada);
      enviarDatosCosecha(fechaCosechada, topic);

      setValoresEEPROM();

      return tiempoCosecha;
      break;

    case 5:         //Eleccion de Pimiento
      cantMes = 5;  //Se cosecha luego de los 5 meses
      tiempoCosecha = 7;
      Serial.println("Comenzando el calculo de la cosecha de Pimiento...");
      Serial.println("  ");
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.print("Fecha de cosecha: ");
      Serial.println(fechaCosechada);
      enviarDatosCosecha(fechaCosechada, topic);

      setValoresEEPROM();

      return tiempoCosecha;
      break;
  }
}

void setValoresEEPROM() {

  guardarEEPROM(13, relayStateSectorLuz[1]);
  guardarEEPROM(12, relayStateSectorLuz[2]);
  guardarEEPROM(14, relayStateSectorLuz[3]);

  cantHorasActivado[1] = 0;
  guardarEEPROM(6, cantHorasActivado[1]);

  flagDesactivarSectores[1] = 0;
  //Bandera para el contador de tiempo encendido
  guardarEEPROM(3, flagDesactivarSectores[1]);

  luzDiaSectores[1] = 1;
  //Bandera de encendido para el los reles sector 1
  guardarEEPROM(0, luzDiaSectores[1]);
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
  Serial.print("Fecha a concatenar: ");
  Serial.println(Fecha);

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
      //Serial.println("Activado 1 ");
      topic_Sensor1 = "Inf/SensorTemperatura";
      topic_Sensor2 = "Inf/SensoHumedad1";
      topic_Sensor3 = "Inf/CantLuz";

      //activarRelesSector(sensores[1], sensores[2], sensores[0]);
      enviar_datoSensor_MQTT(sensores[0], sensores[1], sensores[2], topic_Sensor1, topic_Sensor2, topic_Sensor3);
      break;
  }
  activarRelesSector(sensores);
}

void activarRelesSector(int array[]) {
  int temp = *(array + 1);       //Sensor Temperatura
  int Fres = *(array + 2);       //Sensor Fotoresistencia
  int senSuelo1 = *(array + 3);  //Sensor de Suelo 1
  int senSuelo2 = *(array + 4);  //Sensor de Suelo 2
  int senSuelo3 = *(array + 5);  //Sensor de Suelo 3

  senSuelo1 = ((senSuelo1 * 100) / 4095);
  senSuelo2 = ((senSuelo2 * 100) / 4095);
  senSuelo3 = ((senSuelo3 * 100) / 4095);

  Fres = ((Fres * 100) / 4095);

  activarRelesLuz(Fres);

  activarRelesElectrovalvulas(senSuelo1, senSuelo2, senSuelo3);
}

void activarRelesElectrovalvulas(int senSuelo1, int senSuelo2, int senSuelo3) {
  static unsigned long tiempo = millis();  //Variable a guardar el tiempo de millis
  int cont = 0;

  cantHumedad[1] = 100 - cantHumedad[1];
  cantHumedad[2] = 100 - cantHumedad[2];
  cantHumedad[3] = 100 - cantHumedad[3];

  if (millis() - tiempo >= 1000) {
    tiempo = millis();
    cont = cont + 1;
    /*
    Serial.println("Sensor de Suelo 1:");
    Serial.print(senSuelo1);
    Serial.println("%");

    Serial.println("Sensor de Suelo 2:");
    Serial.print(senSuelo2);
    Serial.println("%");

    Serial.println("Sensor de Suelo 3:");
    Serial.print(senSuelo3);
    Serial.println("%");
*/
    if (cantHumedad[1] > senSuelo1) {
      digitalWrite(relaySector1_ELEC, 0);
      //Serial.println("Electrovalvula 1 encendida");

    } else if (cantHumedad[1] < senSuelo1) {
      digitalWrite(relaySector1_ELEC, 1);
      //Serial.println("Electrovalvula 1 apagada");
    }
    if (cantHumedad[2] > senSuelo2) {
      digitalWrite(relaySector2_ELEC, 0);
      Serial.println("Electrovalvula 2 encendida");
    } else if (cantHumedad[2] < senSuelo2) {
      digitalWrite(relaySector2_ELEC, 1);

      //Serial.println("Electrovalvula 2 apagada");
    }
    if (cantHumedad[3] > senSuelo3) {
      digitalWrite(relaySector3_ELEC, 0);
      //Serial.println("Electrovalvula 3 encendida");
    } else if (cantHumedad[3] < senSuelo3) {
      digitalWrite(relaySector3_ELEC, 1);
      //Serial.println("Electrovalvula 3 apagada");
    }
  }
}

void activarRelesLuz(int Fres) {
  static unsigned long TactivoSector1 = millis();     //Variable a guardar el tiempo de millis
  static unsigned long TdesactivoSector1 = millis();  //Variable a guardar el tiempo de millis

  static unsigned long TactivoSector2 = millis();     //Variable a guardar el tiempo de millis
  static unsigned long TdesactivoSector2 = millis();  //Variable a guardar el tiempo de millis

  static unsigned long TactivoSector3 = millis();     //Variable a guardar el tiempo de millis
  static unsigned long TdesactivoSector3 = millis();  //Variable a guardar el tiempo de millis

  static unsigned long TactivoFotoresis = millis();  //Variable a guardar el tiempo de millis
  //static unsigned long TdesactivoSector3 = millis();  //Variable a guardar el tiempo de millis


  if (Fres == 100) {
    if (millis() - TactivoFotoresis >= 1000) {
      TactivoFotoresis = millis();
      cantHorasActivado[4] = cantHorasActivado[4] + 1;
      Serial.println("Cantidad de horas encendido sensor de Luz: ");
      Serial.println(cantHorasActivado[4]);

      guardarEEPROM(18, cantHorasActivado[4]);
    }
  }

  if (relayStateSectorLuz[1] == 1) {
    if (flagDesactivarSectores[1] == 0) {
      luzDiaSectores[1] = 1;
      if (millis() - TactivoSector1 >= 10000) {
        TactivoSector1 = millis();
        cantHorasActivado[1] = cantHorasActivado[1] + 1;
        //Serial.print("HORAS SECTOR 1 ACTIVADO: ");
        //Serial.println(cantHorasActivado[1]);

        //Contador de horas Activado
        guardarEEPROM(6, cantHorasActivado[1]);

        //Bandera para el contador de tiempo encendido
        guardarEEPROM(3, flagDesactivarSectores[1]);

        //Bandera de encendido para el los reles sector 1
        guardarEEPROM(0, luzDiaSectores[1]);

        Serial.println("RELE 1 ENCEDIDO");
        digitalWrite(relaySector1_LUZ, luzDiaSectores[1]);
      }
      if (cantHorasActivado[1] + (cantHorasActivado[4] * 3) >= tiempoCosecha[1]) {  //Multiplicamos por 3 debido a que 1 hora de luz natural equivalen a 3 de luz artificial

        Serial.print("Ciclo Cumplido");
        flagDesactivarSectores[1] = 1;
        cantHorasActivado[1] = 0;

        //Contador de horas Activado
        guardarEEPROM(6, cantHorasActivado[1]);

        //Bandera para el contador de tiempo encendido
        guardarEEPROM(4, flagDesactivarSectores[1]);

        //Bandera de encendido para el sector 1
        guardarEEPROM(0, luzDiaSectores[1]);
      }
    } else if (flagDesactivarSectores[1] == 1) {
      luzDiaSectores[1] = 0;
      if (millis() - TdesactivoSector1 >= 10000) {
        TdesactivoSector1 = millis();
        cantHorasDesactivado[1] = cantHorasDesactivado[1] + 1;
        //Serial.print("HORAS SECTOR 1 DESACTIVADO: ");
        //Serial.println(cantHorasDesactivado[1]);

        //Contador de horas desactivado
        guardarEEPROM(9, cantHorasDesactivado[1]);

        //Bandera para el contador de tiempo encendido
        guardarEEPROM(4, flagDesactivarSectores[1]);

        //Bandera de encendido para el sector 1
        guardarEEPROM(0, luzDiaSectores[1]);

        Serial.println("RELE APAGADO");
        digitalWrite(relaySector1_LUZ, luzDiaSectores[1]);
      }

      if (cantHorasDesactivado[1] == (24 - tiempoCosecha[1])) {
        Serial.println("Ciclo desactivado Cumplido");
        flagDesactivarSectores[1] = 0;  //Estado : Apagado
        cantHorasDesactivado[1] = 0;

        //Contador de horas desactivado
        guardarEEPROM(9, cantHorasDesactivado[1]);

        //Bandera para el contador de tiempo encendido/apagado
        guardarEEPROM(3, flagDesactivarSectores[1]);

        //Bandera de encendido para el sector 1
        guardarEEPROM(0, luzDiaSectores[1]);
      }
    }
  }

  if (relayStateSectorLuz[2] == 1) {
    if (flagDesactivarSectores[2] == 0) {
      luzDiaSectores[2] = 1;
      if (millis() - TactivoSector2 >= 10000) {
        TactivoSector2 = millis();
        cantHorasActivado[2] = cantHorasActivado[2] + 1;
        //Serial.print("HORAS SECTIR 2 ACTIVADO: ");
        //Serial.println(cantHorasActivado[2]);

        //Contador de horas desactivado
        guardarEEPROM(7, cantHorasActivado[2]);

        //Bandera para el contador de tiempo encendido/apagado
        guardarEEPROM(4, flagDesactivarSectores[2]);

        //Bandera de encendido para el sector 1
        guardarEEPROM(1, luzDiaSectores[2]);

        Serial.println("RELE 2 ENCEDIDO");
        digitalWrite(relaySector2_LUZ, luzDiaSectores[2]);
      }
      if (cantHorasActivado[2] + (cantHorasActivado[4] * 3) >= tiempoCosecha[2]) {

        Serial.print("Ciclo 2 Cumplido");
        flagDesactivarSectores[2] = 1;
        cantHorasActivado[2] = 0;

        //Contador de horas desactivado
        guardarEEPROM(7, cantHorasActivado[2]);

        //Bandera para el contador de tiempo encendido/apagado
        guardarEEPROM(4, flagDesactivarSectores[2]);

        //Bandera de encendido para el sector 1
        guardarEEPROM(1, luzDiaSectores[2]);
      }
    } else if (flagDesactivarSectores[2] == 1) {
      luzDiaSectores[2] = 0;
      if (millis() - TdesactivoSector2 >= tiempoCosecha[2]) {
        TdesactivoSector2 = millis();
        cantHorasDesactivado[2] = cantHorasDesactivado[2] + 1;
        //Serial.print("Horas 2 Desactivado: ");
        //Serial.println(cantHorasDesactivado[2]);

        //Contador de horas desactivado
        guardarEEPROM(10, cantHorasDesactivado[2]);

        //Bandera para el contador de tiempo encendido/apagado
        guardarEEPROM(4, flagDesactivarSectores[2]);

        //Bandera de encendido para el sector 1
        guardarEEPROM(1, luzDiaSectores[2]);

        Serial.println("RELE 2 APAGADO");
        digitalWrite(relaySector2_LUZ, luzDiaSectores[1]);
      }

      if (cantHorasDesactivado[2] == (24 - tiempoCosecha[2])) {
        Serial.println("Ciclo 2 desactivado Cumplido");
        flagDesactivarSectores[2] = 0;  //Estado : Apagado
        cantHorasDesactivado[2] = 0;

        //Contador de horas desactivado
        guardarEEPROM(10, cantHorasDesactivado[2]);

        //Bandera para el contador de tiempo encendido/apagado
        guardarEEPROM(4, flagDesactivarSectores[2]);

        //Bandera de encendido para el sector 1
        guardarEEPROM(1, luzDiaSectores[2]);
      }
    }
  }

  if (relayStateSectorLuz[3] == 1) {
    if (flagDesactivarSectores[3] == 0) {
      luzDiaSectores[3] = 1;
      if (millis() - TactivoSector3 >= 10000) {
        TactivoSector3 = millis();
        cantHorasActivado[3] = cantHorasActivado[3] + 1;
        //Serial.print("HORAS SECTOR 3 ACTIVADO: ");
        //Serial.println(cantHorasActivado[1]);

        //Contador de horas Activado
        guardarEEPROM(8, cantHorasActivado[3]);

        //Bandera para el contador de tiempo encendido
        guardarEEPROM(5, flagDesactivarSectores[3]);

        //Bandera de encendido para el los reles sector 1
        guardarEEPROM(2, luzDiaSectores[3]);

        Serial.println("RELE 3 ENCEDIDO");
        digitalWrite(relaySector3_LUZ, luzDiaSectores[3]);
      }
      if (cantHorasActivado[3] + (cantHorasActivado[4] * 3) >= tiempoCosecha[3]) {

        Serial.println("CICLO SECTOR 3 Cumplido");
        flagDesactivarSectores[3] = 1;
        cantHorasActivado[3] = 0;

        //Contador de horas Activado
        guardarEEPROM(8, cantHorasActivado[3]);

        //Bandera para el contador de tiempo encendido
        guardarEEPROM(5, flagDesactivarSectores[3]);

        //Bandera de encendido para el sector 1
        guardarEEPROM(2, luzDiaSectores[3]);
      }
    } else if (flagDesactivarSectores[3] == 1) {
      luzDiaSectores[1] = 0;
      if (millis() - TdesactivoSector3 >= 10000) {
        TdesactivoSector3 = millis();
        cantHorasDesactivado[3] = cantHorasDesactivado[3] + 1;
        //Serial.print("HORAS SECTOR 3 DESACTIVADO: ");
        //Serial.println(cantHorasDesactivado[3]);

        //Contador de horas desactivado
        guardarEEPROM(11, cantHorasDesactivado[3]);

        //Bandera para el contador de tiempo encendido
        guardarEEPROM(5, flagDesactivarSectores[3]);

        //Bandera de encendido para el sector 1
        guardarEEPROM(2, luzDiaSectores[3]);

        Serial.println("RELE APAGADO");
        digitalWrite(relaySector3_LUZ, luzDiaSectores[3]);
      }

      if (cantHorasDesactivado[3] == (24 - tiempoCosecha[3])) {
        Serial.println("CICLO SECTOR 3 desactivado - Cumplido");
        flagDesactivarSectores[3] = 0;  //Estado : Apagado
        cantHorasDesactivado[3] = 0;

        //Contador de horas desactivado
        guardarEEPROM(11, cantHorasDesactivado[3]);

        //Bandera para el contador de tiempo encendido/apagado
        guardarEEPROM(5, flagDesactivarSectores[3]);

        //Bandera de encendido para el sector 1
        guardarEEPROM(2, luzDiaSectores[3]);
      }
    }
  }
}

void guardarEEPROM(int direccion, int memoria) {
  EEPROM.put(direccion, memoria);  //Direccion - Variable
  EEPROM.commit();                 //Confirmar
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

    MQTT_CLIENT.subscribe("Reles/Sectores1");  // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Reles/Sectores2");  // Aca realiza la suscripcion
    MQTT_CLIENT.subscribe("Reles/Sectores3");  // Aca realiza la suscripcion

    // Espera para que conecte denuevo
    delay(3000);
  }
  Serial.println("Conectado a MQTT");
  Serial.println(" ");
}