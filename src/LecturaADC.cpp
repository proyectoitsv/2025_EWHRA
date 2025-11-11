// Defino pin analógico - GPIO 0, 1, 2, 3 o 4
#define PIN_SENAL 0 // Cambia a 0, 1, 2, 3 o 4 según tu conexión

// Variables globales
int valorPin = 0;
float voltaje = 0.0;

void setup() {
    Serial.begin(115200);
    
    // Pequeña pausa para estabilizar la comunicación serial
    delay(1000);
    
    // Configurar resolución ADC (opcional, por defecto ya es 12 bits)
    analogReadResolution(12); // 12 bits = 0-4095
    
    // Configurar atenuación del ADC para rango completo 0-3.3V
    analogSetAttenuation(ADC_11db); // Permite lectura de 0-3.3V
    
    Serial.println("=== Lectura de Pin Analógico ESP32-C3 ===");
    Serial.println("Iniciando lecturas...\n");
}

void loop() {
    // Lectura del pin analógico (corregido)
    valorPin = analogRead(PIN_SENAL);
    
    // Conversión a voltaje (corregido el cálculo)
    voltaje = valorPin * (3.3 / 4095.0);
    
    // Mostrar resultados en el puerto serie
    Serial.print("Entrada ADC: ");
    Serial.print(valorPin);
    Serial.print(" | Voltaje: ");
    Serial.print(voltaje, 3); // 3 decimales
    Serial.println(" V");
    
    // Pequeña pausa entre lecturas para no saturar el serial
    delay(500);
}