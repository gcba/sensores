/*
   Cliente ethernet para postear datos a la API de SENSORES GCBA
   Compilado en:    IDE v1.0.5
   Placa:           Arduino Uno R3
   Dispositivo:     w5100/w5200
   Version:         2.1.0
   Fecha:           Agosto 2014
   Funcion:         Envio serial de 13 datos: temperatura y humedad (DHT11), ruido (electret) y luz (LDR), accX, accY,
                    accZ, gyrX, gyrY, gyrZ, roll, pitch y temperatura.
   Direccion General de Innovacion y Gobierno Abierto
*/

// Librerias
#include <SPI.h>
#include "DHT.h" //cargamos la librería DHT
#include <Wire.h>
#include "kalman.h"

// Offset
byte Offset = 0;

// Variables DHT22
#define DHTPIN 2 //Seleccionamos el pin en el que se //conectará el sensor
#define DHTTYPE DHT22 //Se selecciona el DHT11 (hay //otros DHT)
DHT dht(DHTPIN, DHTTYPE); //Se inicia una variable que será usada por Arduino para comunicarse con el sensor
unsigned long time;

// Variables Luz
int lightPin = A3;  //Pin de la foto-resistencia
int light = 0;   //Variable light
int light0 = 0;
float Res0 = 0.4;

// Variables ruido
int electret = 0;
int lect = 0;
int noise = 0;
int threshold = 450; //Valor medidio para el amplificador LM741

// Variables gyro
#define RESTRICT_PITCH // Borrar para que roll sea solo ±90°, leer: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf
Kalman kalmanX; // Instancias Kalman
Kalman kalmanY;
/* IMU Data */
double accX, accY, accZ;
double gyroX, gyroY, gyroZ;
int16_t tempRaw;
double gyroXangle, gyroYangle; // Angulo del gyro
double compAngleX, compAngleY; // Angulo complementario
double kalAngleX, kalAngleY; // Angulo de Kalman
uint32_t timer;
uint8_t i2cData[14]; // Buffer I2C

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
  dht.begin(); // Se inicia DHT22
  Wire.begin(); // Se inicia IMU
  
  //Setup del gyro
  TWBR = ((F_CPU / 400000L) - 16) / 2; // Seteo la frecuencia de I2C a 400kHz

  i2cData[0] = 7; // Seteo el sample rate a 1000Hz - 8kHz/(7+1) = 1000Hz
  i2cData[1] = 0x00; // Desabilito FSYNC y seteo a 260 Hz el filtro Acc, a 256 Hz el filtro del Gyro y a 8 KHz el sampleo
  i2cData[2] = 0x00; // Seteo el Gyro con un rango total de ±250deg/s
  i2cData[3] = 0x00; // Seteo el acelerometro con un rango total a ±2g
  while (i2cWrite(0x19, i2cData, 4, false)); // Escrito todos los registros en rafaga al mismo tiempo
  while (i2cWrite(0x6B, 0x01, true)); // PLL con el eje X del gyro como referencia y salgo del modo sleep

  while (i2cRead(0x75, i2cData, 1));
  if (i2cData[0] != 0x68) { // Leo registo WHO_I_AM
    Serial.print(F("Error en lectura"));
    while (1);
  }

  delay(100); // Espero estabilizacion del sensor

  /* Seteo el angulo de comienzo del gyro y del kalman */
  while (i2cRead(0x3B, i2cData, 6));
  accX = (i2cData[0] << 8) | i2cData[1];
  accY = (i2cData[2] << 8) | i2cData[3];
  accZ = (i2cData[4] << 8) | i2cData[5];

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 y 26
  // atan2 da como salida valores de -pi a pi (radianes)
  // Convierto de radianes a grados
  #ifdef RESTRICT_PITCH
  double roll  = atan2(accY, accZ) * RAD_TO_DEG;
  double pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
  #else // Eq. 28 y 29
  double roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
  double pitch = atan2(-accX, accZ) * RAD_TO_DEG;
  #endif

  kalmanX.setAngle(roll); // Set angulo de comienzo
  kalmanY.setAngle(pitch);
  gyroXangle = roll;
  gyroYangle = pitch;
  compAngleX = roll;
  compAngleY = pitch;

  timer = micros();
}


//=======================================================
// Bucle principal
//=======================================================
void loop()
{
  delay(2000);
  
  //Gyro
  /* Actualizo todos los valores */
  while (i2cRead(0x3B, i2cData, 14));
  accX = ((i2cData[0] << 8) | i2cData[1]);
  accY = ((i2cData[2] << 8) | i2cData[3]);
  accZ = ((i2cData[4] << 8) | i2cData[5]);
  tempRaw = (i2cData[6] << 8) | i2cData[7];
  gyroX = (i2cData[8] << 8) | i2cData[9];
  gyroY = (i2cData[10] << 8) | i2cData[11];
  gyroZ = (i2cData[12] << 8) | i2cData[13];

  double dt = (double)(micros() - timer) / 1000000; // Calculo tiempo delta
  timer = micros();

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 y 26
  // atan2 da como salida valores de -pi a pi (radianes)
  // Convierto de radianes a grados
  #ifdef RESTRICT_PITCH // Eq. 25 y 26
  double roll  = atan2(accY, accZ) * RAD_TO_DEG;
  double pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
  #else // Eq. 28 y 29
  double roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
  double pitch = atan2(-accX, accZ) * RAD_TO_DEG;
  #endif

  double gyroXrate = gyroX / 131.0; // Convierto a deg/s
  double gyroYrate = gyroY / 131.0; // Convierto a deg/s

  #ifdef RESTRICT_PITCH
  // Esto arregla el problema de transicion cuando el angulo del acelerometro salta entre -180 y 180 grados (estabilidad)
  if ((roll < -90 && kalAngleX > 90) || (roll > 90 && kalAngleX < -90)) {
    kalmanX.setAngle(roll);
    compAngleX = roll;
    kalAngleX = roll;
    gyroXangle = roll;
  } else
    kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt); // Calculo el angulo (rollX) usando el filtro de Kalman

  if (abs(kalAngleX) > 90)
    gyroYrate = -gyroYrate; // Invierto el rate asi cumplo con la restriccion de lecturas del acelerometro
  kalAngleY = kalmanY.getAngle(pitch, gyroYrate, dt);
#else
  // De nuevo, esto arregla el problema de transicion cuando el angulo del acelerometro salta entre -180 y 180 grados (estabilidad)
  if ((pitch < -90 && kalAngleY > 90) || (pitch > 90 && kalAngleY < -90)) {
    kalmanY.setAngle(pitch);
    compAngleY = pitch;
    kalAngleY = pitch;
    gyroYangle = pitch;
  } else
    kalAngleY = kalmanY.getAngle(pitch, gyroYrate, dt); // Calculo el angulo (pitchY) usando el filtro de Kalman

  if (abs(kalAngleY) > 90)
    gyroXrate = -gyroXrate; // Invierto el rate asi cumplo con la restriccion de lecturas del acelerometro
  kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt);
#endif

  gyroXangle += gyroXrate * dt; // Calculo el angulo del gyro sin filtro
  gyroYangle += gyroYrate * dt;

  compAngleX = 0.93 * (compAngleX + gyroXrate * dt) + 0.07 * roll; // Calculo el angulo usando el filtro complementario
  compAngleY = 0.93 * (compAngleY + gyroYrate * dt) + 0.07 * pitch;

  // Reseteo el angulo del gyro cuando se ha desviado mucho
  if (gyroXangle < -180 || gyroXangle > 180)
    gyroXangle = kalAngleX;
  if (gyroYangle < -180 || gyroYangle > 180)
    gyroYangle = kalAngleY;

 
  /* Muestro toda la data de la placa: temperatura y humedad del (DHT22), luz (LDR), ruido (Electret), temperatura y todos los angulos calculados por el IMU que contiene el gyro,
  el acelerometro y el sensor de temperatura CMOS (integrado en la placa GY-521)*/
  
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
    noise=mxNoise;
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
  #if 1 // Seteo en 1 para mostrar todos los daatos de este IF
  Serial.print(accX,DEC);
  Serial.print(",");
  Serial.print(accY,DEC);
  Serial.print(",");
  Serial.print(accZ,DEC);
  Serial.print(",");
  Serial.print(gyroX,DEC);
  Serial.print(",");
  Serial.print(gyroY,DEC);
  Serial.print(",");
  Serial.print(gyroZ,DEC);
  Serial.print(",");
#endif

  Serial.print(roll,DEC);
  Serial.print(",");
  //Serial.print(gyroXangle,DEC);
  //Serial.print(",");
  //Serial.print(compAngleX,DEC);
  //Serial.print(",");
  //Serial.print(kalAngleX,DEC);
  //Serial.print(",");

  Serial.print(pitch,DEC);
  Serial.print(",");
  //Serial.print(gyroYangle,DEC);
  //Serial.print(",");
  //Serial.print(compAngleY,DEC);
  //Serial.print(",");
  //Serial.print(kalAngleY,DEC);
  //Serial.print(",");

#if 1 // Seteo en 1 para mostrar el dato de temperatura del integrado IMU
  double temperature = (double)tempRaw / 340.0 + 36.53;
  Serial.print(temperature-5,DEC);
  Serial.print(",");
#endif

  Serial.println();
  delay(50);
}
