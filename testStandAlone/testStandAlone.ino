/*
   Cliente ethernet para postear datos a la API de SENSORES GCBA
   Compilado en:    IDE v1.0.5
   Placa:           Arduino Uno R3
   Dispositivo:     w5100/w5200
   Version:         2.1.0
   Fecha:           Agosto 2014
   Funcion:         Postear 4 señales de datos: temperatura y humedad (DHT11), ruido (electret) y luz (LDR)
   Direccion General de Innovacion y Gobierno Abierto
*/

// Librerias
#include <SPI.h>
#include "DHT.h" //cargamos la librería DHT

// Variables
byte Offset=0;
#define DHTPIN 2 //Seleccionamos el pin en el que se //conectará el sensor
#define DHTTYPE DHT11 //Se selecciona el DHT11 (hay //otros DHT)
DHT dht(DHTPIN, DHTTYPE); //Se inicia una variable que será usada por Arduino para comunicarse con el sensor
unsigned long time;
// Variables Luz
int lightPin = A3;  //Pin de la foto-resistencia
int light=0;     //Variable light
int light0=0;
float Res0=10.0;
//int min = 0;       //valor mínimo que da la foto-resistencia
//int max = 1000;       //valor máximo que da la foto-resistencia
// Variables ruido
int electret = 0;
int lect = 0;
int noise = 0;   
int threshold=450; //Valor medidio para el amplificador LM741


//=======================================================
/// Set-Up inicial del programa
//=======================================================
void setup() {
  Serial.begin(9600);  // Comienza comunicacion serial
  dht.begin(); //Se inicia el sensor
}


//=======================================================
// Bucle principal
//=======================================================
void loop()
{
    //Temperatura
    int temp = dht.readTemperature()-6;
    //Humedad
    int hum = dht.readHumidity()+7;
    //Ruido
    int lect = analogRead(electret);    
    noise = lect-threshold;
    //Luz
    light0 = analogRead(lightPin);   // Read the analogue pin
    float Vout0=light0*0.0048828125;      // calculate the voltage
    light=500/(Res0*((5-Vout0)/Vout0));
    
   Serial.print(temp, DEC);
  Serial.print(",");
  Serial.print(hum, DEC);
  Serial.print(",");
  Serial.print(noise, DEC);
  Serial.print(",");
  Serial.print(light, DEC);
  Serial.print(",");
  Serial.println();
  delay(50);
}
