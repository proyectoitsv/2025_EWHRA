#define ledBLE  13 // Pin del LED integrado en la placa
#define ledEstado 12 // Pin del LED de estado de conexión

void setup()
{
    pinMode(ledBLE, OUTPUT); //Definir como salida el pin del LED integrado
    pinMode(ledEstado, OUTPUT); //Definir como salida el pin del LED de estado
}

void loop()
{
    // Parpadear el LED integrado para indicar que el BLE está activo
    digitalWrite(ledBLE, HIGH);
    delay(500);
    digitalWrite(ledBLE, LOW);
    delay(500);
    
    // Parpadear el LED de estado para indicar que el dispositivo está activo
    digitalWrite(ledEstado, HIGH);
    delay(500);
    digitalWrite(ledEstado, LOW);
    delay(500);
    
}