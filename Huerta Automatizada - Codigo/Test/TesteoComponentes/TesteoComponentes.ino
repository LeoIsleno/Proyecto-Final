//Testear todos los componentes a utilizar y revisar que le ADC del Modulo ESP-32 funcionen correctamente.
//Al utilizar el modulo WiFi se deshabilitan varios pines ADC (ADC2) por lo tanto nos permite usar una serie de pines limitados (ADC1)(32,33,34,35,36,39)

#include <dummy.h>
#include <WiFi.h>
#include <ThingSpeak.h>

//const int potenciometro = 33;
const int fotoresistencia = 33;
//const int sensorTemperatura = 35;
//const int sensorSuelo = 34;
//const int relay = 19;
int lectura;

/*Iniciamos la función Setup()*/
void setup() {
  /*Iniciamos el terminal Serial a una velocidad de 115200, junto a un retardo de 1 segundo y definimos los pines a utilizar*/
 // pinMode(33,INPUT); //Configuramos Potenciometro como entrada o INPUT
  pinMode(15,INPUT); //Configuramos fotoresistencia como entrada o INPUT
 // pinMode(35,INPUT); //Configuramos sensor Temperatura como entrada o INPUT
  //pinMode(34,INPUT); //Configuramos sensor de Suelo como entrada o INPUT
 //// pinMode(relay, OUTPUT); // Configurar relay como salida o OUTPUT
  
  Serial.begin(115200); // Abrir el puerto serie a la velocidad de 9600bps para trasnmicion de datos.
  delay(1000); // 1 segundo
  Serial.println("Sensores Instalados y listos");
}
void loop() {
  //Lectura y prueba de sensores 
  
 // lectura = analogRead(potenciometro);
 // Serial.println("====================================");
 // Serial.print("Valor del potenciometro: ");
 // Serial.println(lectura);
  lectura = analogRead(fotoresistencia);
  Serial.print("Valor de la fotoresistencia: ");
  Serial.println(lectura);
 // lectura = analogRead(sensorTemperatura);
 // Serial.print("Valor del sensor de temperatura: ");
 // Serial.println(lectura);
 // lectura = analogRead(sensorSuelo);
 // Serial.print("Valor del sensor de suelo: ");
 // Serial.println(lectura);
  //delay(2000);

  //Prueba de Modulos Relay
  
  //digitalWrite(relay, HIGH); // envia señal alta al relay
  //Serial.println("Relay accionado");
  //delay(1000);           // 1 segundo
  
  //digitalWrite(relay, LOW);  // envia señal baja al relay
  //Serial.println("Relay no accionado");
  //delay(1000);           // 1 segundo

}
