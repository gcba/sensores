/*
   Cliente ethernet para postear datos a la API de SENSORES GCBA
   Compilado en:    IDE v1.0.5
   Placa:           Arduino Uno R3
   Dispositivo:     ENC28J80
   Version:         1.0.0
   Fecha:           Septiembre 2014
   Direccion General de Innovacion y Gobierno Abierto
*/

// Librerias
#include <SPI.h>
#include <UIPEthernet.h>

// Direccion MAC para conexion DHCP
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// URL del servidor
char serverName[] = "sensores.hml.gcba.gob.ar";

// Puerto del servidor
int serverPort = 80;

/* pageName es el parametro donde se pasa el metodo que acompaña a la URL del servidor, contiente:
user:       ----------
pass :      ----------
timestamp:  year()-month()-day()%20hour():minute():second()
El parametro timestamp toma los valores del sistema, de esa forma nunca se repite una fecha y horario.
*/
char pageName[] = "/api/data/create?user=----------&pass=----------&timestamp=year()-month()-day()%20hour():minute():second()";

// Cliente ethernet
EthernetClient client;

// Params debe ser lo suficientemente grande para contener a la variable
char params[64];

//=======================================================
/// Set-Up inicial del programa
//=======================================================
void setup() {
  Serial.begin(9600);  // Comienza comunicacion serial
  Serial.println("LAB GCBA - Soft para lectura de datos de pluviometro");
  Serial.println();
  Serial.println();
  // Comienza la comunicacion Ethernet
  Serial.print(F("Iniciando ethernet..."));
  if(!Ethernet.begin(mac)) Serial.println(F("Fallo"));
  else Serial.println(Ethernet.localIP());
  delay(2000);
  Serial.println(F("Listo"));}

//=======================================================
// Bucle principal
//=======================================================
void loop(){
    int variable = 10;
    sprintf(params,"id=ID_SENSOR&data=%i&datatype=NOMBRE_TIPO_DATO",variable);
    if(!postPage(serverName,serverPort,pageName,params)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));}

/* Funcion:  postPage

Recibe:    char domainBuffer
           int thisPort
           char page
           char thisData
         
Devuelve:  byte

Es la funcion encargada de realizar el post.

*/

byte postPage(char* domainBuffer,int thisPort,char* page,char* thisData){
  int inChar;
  char outBuf[64]; //El buffer debe ser de 64*n, donde n es la cantidad de señales a enviar
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
  return 1;}
