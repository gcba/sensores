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
#include <Ethernet.h>

// URL del servidor
char dataServer[] = "sensores.hml.gcba.gob.ar";
char pageName[] = "/api/data/create?user=----------&pass=----------&timestamp=year()-month()-day()%20hour():minute():second()";

// Parametros de actualizacion
#define UPDATE_TIME 60000 // INtervalo de 1 minuto para mandar datos al servidor.
#define TIMEOUT 1000 // Timeout de 1 segundo

// Conexion DHCP
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xAE, 0xCD };
EthernetClient client;

// Timers
unsigned long timer1 = 0;
unsigned long timer2 = 0;
int failedResponse = 0;

//=======================================================
/// Set-Up inicial del programa
//=======================================================
void setup()
{
  //Initiallize the serial port.
  Serial.begin(9600);
  Serial.println("LAB GCBA - Cliente de datos para sensores");
  
  // Comienza la comunicacion Ethernet
  Serial.print(F("Iniciando ethernet..."));
  if(!Ethernet.begin(mac)) Serial.println(F("Fallo"));
  else Serial.println(Ethernet.localIP());
  Serial.print("LocalIP:\t\t");
  Serial.println(Ethernet.localIP());
  Serial.print("SubnetMask:\t\t");
  Serial.println(Ethernet.subnetMask());
  Serial.print("GatewayIP:\t\t");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("dnsServerIP:\t\t");
  Serial.println(Ethernet.dnsServerIP());
  delay(2000);
  Serial.println(F("Conexion establecida."));
}

//=======================================================
// Bucle principal
//=======================================================
void loop()
{
  // Actualizar el dato cada 60 segudos
  if(millis() > timer1 + UPDATE_TIME)
  {
    timer1 = millis();  // Actualizo el timer1 con el tiempo actual en milisegundos.
    sendTodataServer(); // Envio la data al servidor.
  }
}

//=======================================================
// Envio de datos
//=======================================================
void sendTodataServer()
{
  // Establezco una coneccion TCP con el servidor donde esta alojada la base de datos de sensores.
  if (client.connect(dataServer, 80))
  {
    // Si el cliente esta conectado al servidor, envio los datos.
    if(client.connected())
    {
      Serial.println("Enviando datos...\n");
      client.print("POST %s HTTP/1.1",pageName);
      client.println("Host: sensores.hml.gcba.gob.ar");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.println("Connection: close");
      client.print("Content-Length: ");
      client.print("&id=");
      client.print("ID_SENSOR");
      client.print("&data=");
      client.print("VARIABLE");
      client.print("&datatype=");
      client.print("TIPO_DE_DATO");
      delay(1000);
      timer2 = millis();
      while((client.available() == 0)&&(millis() < timer2 + TIMEOUT)); // Espero hasta que el servidor responda o hasta que timer2 expire.
      // Mientras existan bytes entrantes del servidor, mostrarlos.
      while(client.available() > 0)
      {
        char inData = client.read();
        Serial.print(inData);
      }
      Serial.println("\n");
      client.stop(); // Desconecto el cliente del servidor
    }
  }
}
