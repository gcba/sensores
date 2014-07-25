/*
   Cliente ethernet para postear datos a la API de SENSORES GCBA
   Compilado en:    IDE v1.0.5
   Placa:           Arduino Uno R3
   Dispositivo:     w5100/w5200
   Version:         1.0.0
   Fecha:           Julio 2014
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

// Direccion MAC para conexion DHCP
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// URL del servidor
char serverName[] = "sensores.hml.gcba.gob.ar";

// Puerto del servidor
int serverPort = 80;

/* pageName es el parametro donde se pasa el metodo que acompaña a la URL del servidor, contiente:
user:       user1
pass :      p4RmoU7q
timestamp:  year()-month()-day()%20hour():minute():second()
El parametro timestamp toma los valores del sistema, de esa forma nunca se repite una fecha y horario.
*/
char pageName[] = "/api/data/create?user=user1&pass=p4RmoU7q&timestamp=year()-month()-day()%20hour():minute():second()";

// Cliente ethernet
EthernetClient client;

// Params debe ser lo suficientemente grande para contener todas las variables
int sizeArrayParams=3;
int paramsBuffer=64;
char params[3];

//char params[64];
//char params2[64];
//char params3[64];
//char params4[64];

//=======================================================
/// Set-Up inicial del programa
//=======================================================
void setup() {
  Serial.begin(9600);  // Comienza comunicacion serial
  dht.begin(); //Se inicia el sensor
  Serial.println("LAB GCBA - Soft para lectura de datos de pluviometro");
  Serial.println();
  Serial.println();

for(int i=0;i<sizeArrayParams;i++){
  params[i]=paramsBuffer;
  Serial.println(params[i]);
}
  // Comienza la comunicacion Ethernet
  Serial.print(F("Iniciando ethernet..."));
  if(!Ethernet.begin(mac)) Serial.println(F("Fallo"));
  else Serial.println(Ethernet.localIP());

  delay(2000);
  Serial.println(F("Listo"));
}


//=======================================================
// Bucle principal
//=======================================================
void loop()
{
    int hum1 = dht.readHumidity(); //Se lee la humedad
    int temp1 = dht.readTemperature(); //Se lee la temperatura
    int hum2 = 2*hum1; //Se lee la humedad
    int temp2 = 2*temp1; //Se lee la temperatura
    // Los parametros deben estar encondeados para la URL.
    
    //Signal 1
    sprintf(params,"id=61&data=%i&datatype=temp1",temp1);
    if(!postPage(serverName,serverPort,pageName,params)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));
    
    //Signal 2
    sprintf(params2,"id=61&data=%i&datatype=temp2",temp2);
    if(!postPage(serverName,serverPort,pageName,params2)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));
    
    //Signal 3
    sprintf(params3,"id=61&data=%i&datatype=hum1",hum1);
    if(!postPage(serverName,serverPort,pageName,params3)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));
    
    //Signal 4
    sprintf(params4,"id=61&data=%i&datatype=hum2",hum2);
    if(!postPage(serverName,serverPort,pageName,params4)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));
    
}

/* Funcion:  postPage

Recibe:    char domainBuffer
           int thisPort
           char page
           char thisData
         
Devuelve:  byte

Es la funcion encargada de realizar el post, 

*/

byte postPage(char* domainBuffer,int thisPort,char* page,char* thisData)
{
  int inChar;
  char outBuf[256]; //El buffer debe ser de 64*n, donde n es la cantidad de señales a enviasr

  // La funcion se conecta al servidor a traver de la URL domainBuffer y del puerto thisPort
  Serial.print(F("Conectando..."));
  if(client.connect(domainBuffer,thisPort))
  {
    Serial.println(F("Conectado"));

    // Enviar el encabezado del post
    sprintf(outBuf,"POST %s HTTP/1.1",page);
    client.println(outBuf);
    sprintf(outBuf,"Host: %s",domainBuffer);
    client.println(outBuf);
    client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
    sprintf(outBuf,"Content-Length: %u\r\n",strlen(thisData));
    client.println(outBuf);

    // Envia el cuerpo del ost (las variables)
    client.print(thisData);
  } 
  else
  {
    Serial.println(F("Fallo"));
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
  Serial.println(F("Desconectando..."));
  client.stop();
  return 1;
}
