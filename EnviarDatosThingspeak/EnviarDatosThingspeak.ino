//Enviar datos de los sensores testeados a ThingsPeak

#include <WiFi.h>
#include <ThingSpeak.h>

const char* ssid="Leou"; //Nombre de la red
const char* password="leo12345A"; //Contraseña de la Red

unsigned long channelID = 1769679; //Channel ID de Thinkspeak
const char* WriteAPIKey ="5OL9UJB4HL5NF7BT"; //APY Key de Thinspeak

WiFiClient cliente;

void setup() {
  //Inicializamos el puerto serial en 115200
  Serial.begin(115200);
   /*Iniciamos la conexión a la red WiFi, y se imprimirán caracteres indicando el tiempo que tarda la conexión*/
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  /*Una vez conectado, se imprimirá una frase y se iniciará la conexión a la Plataforma usando el cliente definido anteriormente*/
  Serial.println("Conectado al WiFi");
  ThingSpeak.begin(cliente);
  delay(5000);
  /*Hacemos la conexión y envío de datos a la plataforma, utilizando las credenciales definidas anteriormente*/
  ThingSpeak.writeFields(channelID,WriteAPIKey);
  /*Imprimimos una frase indicando el envío, y agregamos un retardo de 2 segundos*/
  delay(2000);
}

void loop() {
  /*Usamos un retardo de 5 segundos, y utilizamos la función Medición para la lectura de los sensores*/
  delay(5000);
  medicion();
  /*Hacemos la conexión y envío de datos a la plataforma, utilizando las credenciales definidas anteriormente*/
  ThingSpeak.writeFields(channelID,WriteAPIKey);
  /*Imprimimos una frase indicando el envío, y agregamos un retardo de 10 segundos*/
  Serial.println("Datos enviados a ThingSpeak!");
  delay(1000);
}

/*Definimos la función Medición*/
void medicion(){
  const int potenciometro = 34;
  int lectura;
  /*Realizamos la lectura de Temperatura y Humedad del sensor*/
  lectura = analogRead(potenciometro); //Lectura del potenciometro por ADC
  Serial.println(lectura); //Muestreo de los valores por el Serial
  delay(350); //Delay de 350 ms
  Serial.println("-----------------------------------------");
  /*Indicamos el orden de envío por campos o Field, en el orden definido de la plataforma, junto a los valores del sensor*/
  ThingSpeak.setField(1,lectura); 
}
