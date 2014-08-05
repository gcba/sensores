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
#include <Ethernet.h>
#include "DHT.h" //cargamos la librería DHT

// Variables
byte Offset=0;
#define DHTPIN 2 //Seleccionamos el pin en el que se //conectará el sensor
#define DHTTYPE DHT11 //Se selecciona el DHT11 (hay //otros DHT)
DHT dht(DHTPIN, DHTTYPE); //Se inicia una variable que será usada por Arduino para comunicarse con el sensor
unsigned long time;
// Variables Luz
int lightPin = 5;  //Pin de la foto-resistencia
int light=0;     //Variable light
int min = 0;       //valor mínimo que da la foto-resistencia
int max = 1000;       //valor máximo que da la foto-resistencia
// Variables ruido
int electret = 0;
int noise = 0;   
int sensorMax = 0;
int sensorMin = 1023;
int threshold;

// Direccion MAC para conexion DHCP
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// URL del servidor
char serverName[] = "sensores.hml.gcba.gob.ar";

// Puerto del servidor
int serverPort = 80;

/* pageName es el parametro donde se pasa el metodo que acompaña a la URL del servidor, contiente:
user:       qwerty
pass :      qwerty
timestamp:  year()-month()-day()%20hour():minute():second()
El parametro timestamp toma los valores del sistema, de esa forma nunca se repite una fecha y horario.
*/
char pageName[] = "/api/data/create?user=qwerty&pass=qwerty&timestamp=year()-month()-day()%20hour():minute():second()";

// Cliente ethernet
EthernetClient client;

// Params debe ser lo suficientemente grande para contener todas las variables
char params[64];
char params2[64];
char params3[64];
char params4[64];

//=======================================================
/// Set-Up inicial del programa
//=======================================================
void setup() {
  Serial.begin(9600);  // Comienza comunicacion serial
  dht.begin(); //Se inicia el sensor
  Serial.println("LAB GCBA - Soft para lectura de datos de pluviometro");
  Serial.println();
  Serial.println();
  
  //Calibro Electret
  while (millis() < 3000) {
    threshold = analogRead(electret);
    if (threshold > sensorMax) {
      sensorMax = threshold;
    }
  }
  threshold = sensorMax;

  // Comienza la comunicacion Ethernet
  Serial.print(F("Starting ethernet..."));
  if(!Ethernet.begin(mac)) Serial.println(F("failed"));
  else Serial.println(Ethernet.localIP());

  delay(2000);
  Serial.println(F("Ready"));
}


//=======================================================
// Bucle principal
//=======================================================
void loop()
{
    //Temperatura
    int temp = dht.readTemperature();
    //Humedad
    int hum = dht.readHumidity();
    //Ruido
    noise = analogRead(electret);    
    if ((noise >= threshold)) {
      Serial.println(noise-threshold);}    
    //Luz
    int light = analogRead(lightPin);
    light = map(light, min, max, 0, 255);
    
    // Los parametros deben estarl encondeados para la URL.
    sprintf(params,"id=62&data=%i&datatype=temp",temp);
    if(!postPage(serverName,serverPort,pageName,params)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));
    sprintf(params2,"id=62&data=%i&datatype=hum",hum);
    if(!postPage(serverName,serverPort,pageName,params2)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));
    sprintf(params3,"id=62&data=%i&datatype=noise",noise);
    if(!postPage(serverName,serverPort,pageName,params3)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));
    sprintf(params4,"id=62&data=%i&datatype=light",light);
    if(!postPage(serverName,serverPort,pageName,params4)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));
    
}

byte postPage(char* domainBuffer,int thisPort,char* page,char* thisData)
{
  int inChar;
  char outBuf[300];

  Serial.print(F("connecting..."));
  if(client.connect(domainBuffer,thisPort))
  {
    Serial.println(F("connected"));
    sprintf(outBuf,"POST %s HTTP/1.1",page);
    client.println(outBuf);
    sprintf(outBuf,"Host: %s",domainBuffer);
    client.println(outBuf);
    client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
    sprintf(outBuf,"Content-Length: %u\r\n",strlen(thisData));
    client.println(outBuf);
    client.print(thisData);
  } 
  else
  {
    Serial.println(F("failed"));
    return 0;
  }

  int connectLoop = 0;

  while(client.connected())
  {
    while(client.available())
    {
      inChar = client.read();
      Serial.write(inChar);
      connectLoop = 0;
    }

    delay(1);
    connectLoop++;
    if(connectLoop > 10000)
    {
      Serial.println();
      Serial.println(F("Timeout"));
      client.stop();
    }
  }

  Serial.println();
  Serial.println(F("disconnecting."));
  client.stop();
  return 1;
}
