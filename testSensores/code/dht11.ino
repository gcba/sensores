#include "DHT.h" //cargamos la librería DHT

// Variables
#define DHTPIN 2 // Seleccionamos el pin en el que se conectará el sensor
#define DHTTYPE DHT11 // Se selecciona el DHT11 (hay otros DHT)
DHT dht(DHTPIN, DHTTYPE); // Se inicia una variable que será usada por Arduino para comunicarse con el sensor

void setup() {
  Serial.begin(9600);  // Comienza comunicacion serial
  dht.begin(); //Se inicia el sensor
}

void loop()
{
    int temp = dht.readTemperature(); //  Leo la temperatura
    int hum = dht.readHumidity(); // Leo la humedad
    Serial.print("Temperatura = ");
    Serial.println(temp);
    Serial.print("Humedad = ");
    Serial.println(hum);
}
