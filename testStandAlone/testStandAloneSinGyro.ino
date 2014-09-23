/*
   Cliente ethernet para postear datos a la API de SENSORES GCBA
   Compilado en:    IDE v1.0.5
   Placa:           Arduino Uno R3
   Version:         2.1.0
   Fecha:           Agosto 2014
   Funcion:         Leer 4 señales de datos: temperatura y humedad (DHT11), ruido (electret) y luz (LDR)
   Direccion General de Innovacion y Gobierno Abierto
*/

// Librerias
#include "DHT.h" //cargamos la librería DHT

// Offset
byte Offset = 0;

// Variables DHT22
#define DHTPIN 2 //Seleccionamos el pin en el que se //conectará el sensor
#define DHTTYPE DHT22 //Se selecciona el DHT11 (hay //otros DHT)
DHT dht(DHTPIN, DHTTYPE); //Se inicia una variable que será usada por Arduino para comunicarse con el sensor

// Variables Luz
int lightPin = A3;  //Pin de la foto-resistencia
int light = 0;   //Variable light
int light0 = 0;
float Res0 = 0.4;

// Variables ruido
int electret = A0;
int lect = 0;
int noise = 0;
int threshold = 450; //Valor medidio para el amplificador LM741

// Rangos
int minTemp = 0;
int maxTemp = 40;
int minHum = 0;
int maxHum = 100;
int minNoise = 40;
int maxNoise = 120;
int minLight = 0;
int maxLight = 4000;


//=======================================================
/// Set-Up inicial del programa
//=======================================================
void setup() {
  Serial.begin(115200);  // Comienza comunicacion serial
  dht.begin(); //Se inicia el sensor
}


//=======================================================
// Bucle principal
//=======================================================
void loop()
{
  //Temperatura
  int temp = dht.readTemperature() -5;
  if(temp<minTemp)
  {
    temp=minTemp;
  }
  else if(temp>maxTemp)
  {
    temp=maxTemp;
  }
  //Humedad
  int hum = dht.readHumidity() + 11;
  if(hum<minHum)
  {
    hum=minHum;
  }
  else if(hum>maxHum)
  {
    hum=maxHum;
  }
  //Ruido
  int lect = analogRead(electret);
  noise = lect - threshold;
  if(noise<minNoise)
  {
    noise=minNoise;
  }
  else if(noise>maxNoise)
  {
    noise=maxNoise;
  }
  //Luz
  light0 = analogRead(lightPin);   // Read the analogue pin
  float Vout0 = light0 * 0.0048828125;  // calculate the voltage
  light = 500 / (Res0 * ((5 - Vout0) / Vout0));
    if(light<minLight)
  {
    light=minLight;
  }
  else if(light>maxLight)
  {
    temp=maxLight;
  }

  Serial.print(temp,DEC);
  Serial.print(",");
  Serial.print(hum,DEC);
  Serial.print(",");
  Serial.print(noise, DEC);
  Serial.print(",");
  Serial.print(light, DEC);
  Serial.print(",");
  Serial.println();
  delay(50);
}
