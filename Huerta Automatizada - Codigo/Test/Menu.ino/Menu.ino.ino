/*
  * Se puede calcular la fecha de cosecha segun la planta elegida
  * Activacion de sectores segun los cultivos a cosechar listo
  * 
*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  int dato = 1;  //Variable a recibit desde App Inventor
  menu_sector(dato);
}

void menu_sector(int dato) {
  String fecha = "27/10/22";  //Variable a recibir desde App Inventor
  int eleccion = 004;         //Variable a recibir desde App Inventor

  float lectura_fotoresistencia, lectura_sensorTemp;
  int lectura_sensorSuelo1, lectura_sensorSuelo2, lectura_sensorSuelo3;
  String topic_Sensor1, topic_Sensor2, topic_Sensor3;

  switch (dato) {
    case 1:  //Se habilita el sector N° 1
      //Habilitamos los sensores del sector N° 1
      lectura_fotoresistencia = analogRead(fotoresistencia)/ 4095) * 100; // PIN 39
      lectura_sensorSuelo1 = analogRead(sensorSuelo1);
      lectura_sensorTemp = analogRead(sensorTemp) * 0.1;  //  10mv/°C PIN 33

      topic_Sensor1 = "Inf/Fotoresistencia";
      topic_Sensor2 = "Inf/SensorSuelo2";
      topic_Sensor3 = "Inf/SensorTemp";
      enviar_datoSensor_MQTT(lectura_fotoresistencia, lectura_sensorTemp, lectura_sensorSuelo1, topic_Sensor1, topic_Sensor2, topic_Sensor3);
      break;
    case 2:                                                               //Se habilita el sector N° 2
      lectura_fotoresistencia = analogRead(fotoresistencia)/ 4095) * 100; // PIN 39
      lectura_sensorSuelo2 = analogRead(sensorSuelo2);
      lectura_sensorTemp = analogRead(sensorTemp) * 0.1;  //  10mv/°C PIN 33

      topic_Sensor1 = "Inf/Fotoresistencia";
      topic_Sensor2 = "Inf/SensorSuelo2";
      topic_Sensor3 = "Inf/SensorTemp";
      enviar_datoSensor_MQTT(lectura_fotoresistencia, lectura_sensorTemp, lectura_sensorSuelo1, topic_Sensor1, topic_Sensor2, topic_Sensor3);
      break;
    case 3:                                                               //Se habilita el sector N° 3
      lectura_fotoresistencia = analogRead(fotoresistencia)/ 4095) * 100; // PIN 39
      lectura_sensorSuelo3 = analogRead(sensorSuelo3);
      lectura_sensorTemp = analogRead(sensorTemp) * 0.1;  //  10mv/°C PIN 33
      topic_Sensor1 = "Inf/Fotoresistencia";
      topic_Sensor2 = "Inf/SensorSuelo2";
      topic_Sensor3 = "Inf/SensorTemp";
      enviar_datoSensor_MQTT(lectura_fotoresistencia, lectura_sensorTemp, lectura_sensorSuelo1, topic_Sensor1, topic_Sensor2, topic_Sensor3);
      break;
    case 4:                                                                  //Sectores 1 y 2 Activados
      lectura_fotoresistencia = (analogRead(fotoresistencia) / 4095) * 100;  // PIN 39
      lectura_sensorSuelo1 = analogRead(sensorSuelo1);
      lectura_sensorTemp = analogRead(sensorTemp) * 0.1;  //  10mv/°C PIN 33

      topic_Sensor1 = "Inf/Fotoresistencia";
      topic_Sensor2 = "Inf/SensorSuelo2";
      topic_Sensor3 = "Inf/SensorTemp";
      enviar_datoSensor_MQTT(lectura_fotoresistencia, lectura_sensorTemp, lectura_sensorSuelo1, topic_Sensor1, topic_Sensor2, topic_Sensor3);

      delay(500);

      lectura_fotoresistencia = (analogRead(fotoresistencia) / 4095) * 100;  // PIN 39
      lectura_sensorSuelo2 = analogRead(sensorSuelo2);
      lectura_sensorTemp = analogRead(sensorTemp) * 0.1;  //  10mv/°C PIN 33

      topic_Sensor1 = "Inf/Fotoresistencia";
      topic_Sensor2 = "Inf/SensorSuelo2";
      topic_Sensor3 = "Inf/SensorTemp";
      enviar_datoSensor_MQTT(lectura_fotoresistencia, lectura_sensorTemp, lectura_sensorSuelo1, topic_Sensor1, topic_Sensor2, topic_Sensor3);
      break;

    default:

      break;
  }
  seleccion_cultivo(eleccion, fecha);
}

void enviar_datoSensor_MQTT(float lecturaLuz, float lecturaTemp, int lecturaSuelo, String topic_Sensor1, String topic_Sensor2, String topic_Sensor3) {
  String dato1 = String(lecturaLuz);  //Se convierte el tipo de variable de int a String
  char a[1];
  dato1.toCharArray(a, 12);  //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
  Serial.println(a);
  MQTT_CLIENT.publish(topic_Sensor1, a);  //Envia la informacion dentro del arreglo char
  delay(500);

  String dato1 = String(lecturaTemp);  //Se convierte el tipo de variable de int a String
  char b[1];
  dato1.toCharArray(b, 12);  //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
  Serial.println(b);
  MQTT_CLIENT.publish(topic_Sensor2, b);  //Envia la informacion dentro del arreglo char
  delay(500);

  String dato1 = String(lecturaSuelo);  //Se convierte el tipo de variable de int a String
  char c[1];
  dato1.toCharArray(c, 12);  //Se convierte el tipo de variable de String a Char ( Variable, cantidad de bytes a trabajar )
  Serial.println(c);
  MQTT_CLIENT.publish(topic_Sensor3, c);  //Envia la informacion dentro del arreglo char
  delay(500);
}

void seleccion_cultivo(int eleccion, String Fecha) {
  int cantMes;
  String fechaCosechada;
  switch (eleccion) {
    case 001:       //Eleccion de Tomates
      cantMes = 4;  //Se cosecha luego de los 4 meses
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.println(fechaCosechada);
      break;
    case 002:       //Eleccion Cebollas
      cantMes = 4;  //Se cosecha luego de los 4 meses
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.println(fechaCosechada);
      break;
    case 003:       //Eleccion Pimiento
      cantMes = 5;  //Se cosecha luego de los 5 meses
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.println(fechaCosechada);
      break;
    case 004:       //Eleccion Lechuga
      cantMes = 3;  //Se cosecha luego de los 3 meses
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.println(fechaCosechada);
      break;
    case 005:       //Eleccion Zanahoria
      cantMes = 3;  //Se cosecha luego de los 3 meses
      fechaCosechada = fecha_cosecha(Fecha, cantMes);
      Serial.println(fechaCosechada);
      break;
    default:
      Serial.print("No se eligio nada");
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