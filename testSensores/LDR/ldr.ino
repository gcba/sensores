int lightPin = 5; // Puerto A5
int valor=0;  // Inicializo la variable
int min = 0;  // Valos minimo de la foto-resistencia
int max = 390;  // Valor máximo de la foto-resistencia

void setup()
{
    Serial.begin(9600);  //Inicializa el serial
}

void loop()
{
    valor = analogRead(lightPin); // Leo el valor del sensor
    Serial.print("LDR = ");
    Serial.println(valor); 
    valor = map(valor, min, max, 0, 255); // Mapeo el valor leido, basado en el minimo y maximo, entre 0 y 255
    Serial.print("LDRmapped = "); 
    Serial.println(valor); 
    delay(1000);
}
