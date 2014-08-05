int electret = 0; // Pin A0
int sensorReading = 0;  // Inicializo la variable   
int sensorMax = 0;  // Valor maximo
int sensorMin = 1023; // Valor minimo
int threshold;  // Umbral

void setup() {
  Serial.begin(9600); // Inicio serial

  // Calibro
  while (millis() < 3000) {
    threshold = analogRead(electret);
    if (threshold > sensorMax) {
      sensorMax = threshold;
    }
  }
  threshold = sensorMax;
}

void loop() {
  sensorReading = analogRead(electret); // Leo el valor del sensor
  if ((sensorReading >= threshold)) {
    Serial.println(sensorReading-threshold); //Valores positivos y absolutos de la onda         
  }
}
