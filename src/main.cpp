// --- Librerías BLE ---
// Estas librerías permiten crear el servidor BLE, enviar características y notificar cambios.
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --- Objetos globales BLE ---
BLEServer *pServer = NULL;              // Objeto del servidor BLE principal
BLECharacteristic *pADCCharacteristic = NULL;    // Característica donde enviamos los bytes del ADC
BLECharacteristic *pNivelCharacteristic = NULL;  // Característica donde enviamos "ALTO/MEDIO/BAJO"

// --- Estados de conexión ---
bool deviceConnected = false;      // True cuando un cliente BLE (tu app) está conectado
bool oldDeviceConnected = false;   // Se usa para detectar cambios de estado

// --- Pines ---
// Pines que vos definiste: señal EEG, LED de estado general y LED de conexión BLE
#define SIGNAL_PIN 0
#define ledEstado 10
#define ledBLE 9

// --- UUIDs ---
// Identificadores únicos del servicio y características BLE.
// Los UUIDs distinguen cada característica para que tu app pueda leerlos.
#define SERVICE_UUID              "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define ADC_CHARACTERISTIC_UUID   "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define NIVEL_CHARACTERISTIC_UUID "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"


// --- Callbacks BLE ---
// Esta clase detecta cuándo un cliente se conecta o desconecta.
// Se ejecuta automáticamente cuando ocurre el evento.
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("✓ Cliente conectado");
  }
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("✗ Cliente desconectado");
  }
};


void setup() {
  Serial.begin(115200);  // Para monitorear desde serial

  // Configuración de los LEDs
  pinMode(ledEstado, OUTPUT);
  pinMode(ledBLE, OUTPUT);

  // Configuración del ADC
  analogReadResolution(12);    // Configura resolución de 0 a 4095
  pinMode(SIGNAL_PIN, INPUT);

  // Inicializar BLE con el nombre del dispositivo
  BLEDevice::init("EWHRA");

  // Crear servidor BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());  // activar callbacks

  // Crear el servicio BLE usando su UUID
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // --- Característica del ADC (dos bytes) ---
  // Esta característica será leída y notificada.
  pADCCharacteristic = pService->createCharacteristic(
    ADC_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  // Descriptor obligatorio para permitir notificaciones
  pADCCharacteristic->addDescriptor(new BLE2902());

  // --- Característica del Nivel (texto ALTO/MEDIO/BAJO) ---
  pNivelCharacteristic = pService->createCharacteristic(
    NIVEL_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pNivelCharacteristic->addDescriptor(new BLE2902());
  pNivelCharacteristic->setValue("BAJO");  // valor inicial

  // Iniciar servicio
  pService->start();

  // Hacer visible el servicio para que la app pueda conectarse
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();

  Serial.println("✓ Servicio BLE iniciado");
  Serial.println("→ Esperando conexión...\n");
}



void loop() {

  digitalWrite(ledEstado, HIGH);  // LED principal siempre encendido

  // Indicar actividad BLE parpadeando si NO hay conexión
  if (!deviceConnected) {
    digitalWrite(ledBLE, HIGH); delay(200);
    digitalWrite(ledBLE, LOW);  delay(200);
  }

  // --- Si hay cliente BLE conectado ---
  if (deviceConnected) {
    digitalWrite(ledBLE, LOW);  // LED fijo indicando conexión activa

    // Leer valor del ADC (0–4095)
    int adcValue = analogRead(SIGNAL_PIN);
    String nivel;

    // --- Clasificación en ALTO/MEDIO/BAJO ---
    // Vos podés ajustar los umbrales cuando quieras.
    if (adcValue >= 3000) nivel = "ALTO";
    else if (adcValue >= 1500) nivel = "MEDIO";
    else nivel = "BAJO";

    // Debug por serial
    Serial.print("ADC = ");
    Serial.print(adcValue);
    Serial.print(" | Nivel = ");
    Serial.println(nivel);

    // ----------- ENVIAR ADC COMO DOS BYTES -----------

    uint16_t adc16 = adcValue;  // Lo convertimos a entero de 16 bits
    uint8_t buffer[2];

    // LSB (byte menos significativo) → buffer[0]
    buffer[0] = adc16 & 0xFF;

    // MSB (byte más significativo) → buffer[1]
    buffer[1] = (adc16 >> 8) & 0xFF;

    // Enviar array de 2 bytes
    pADCCharacteristic->setValue(buffer, 2);
    pADCCharacteristic->notify();

    // ----------- ENVIAR NIVEL COMO TEXTO -----------

    pNivelCharacteristic->setValue(nivel.c_str());
    pNivelCharacteristic->notify();

    delay(100);  // velocidad de actualización
  }

  // Manejo automático del advertising al desconectarse
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("→ Advertising reiniciado");
  }

  // Se ejecuta al pasar de desconectado → conectado
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println("→ Cliente conectado, enviando datos...\n");
  }

  oldDeviceConnected = deviceConnected;
}
