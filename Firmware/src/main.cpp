// =====================
//   EWHRA
//   ESP32-C3 SuperMini 
// =====================

// --- Librerías BLE ---
// Se encargan de crear el servidor BLE, servicio y características.
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --- Objetos BLE globales ---

BLEServer *pServer = NULL; // servidor BLE del ESP32
BLECharacteristic *pADCCharacteristic = NULL; // envía el RMS suavizado (en mV por bytes)
BLECharacteristic *pNivelCharacteristic = NULL; // envía el nivel ALTO / MEDIO / BAJO

// --- Estados de conexión BLE ---

bool deviceConnected = false; // indica si hay un dispositivo conectado
bool oldDeviceConnected = false; // detecta cambios de estado de conexión

// --- Pines ---

#define SIGNAL_PIN 0 // pin del ADC
#define ledEstado 1 // indicador de actividad general
#define ledBLE 2 // indicador de conexión BLE

// --- UUIDs BLE ---

// Identifican el servicio y las características
#define SERVICE_UUID              "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define ADC_CHARACTERISTIC_UUID   "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define NIVEL_CHARACTERISTIC_UUID "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"

// --- Parámetros de procesamiento ---

const int muestras = 200; // cantidad de lecturas por cálculo RMS
float rmsSuave = 0.0; // valor RMS filtrado
float alpha = 0.10; // nivel de suavizado (más chico = más lento)

// Valores máximos del ADC del ESP32-C3

const float ADC_MAX_VOLT = 3.3;
const int   ADC_MAX_COUNT = 4095;

// =================================================
//   CALLBACKS BLE
//   Detectan conexión y desconexión del teléfono
// =================================================
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    digitalWrite(BLE, HIGH);
    Serial.println("✓ Cliente conectado");
  }
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    digitalWrite(BLE, LOW);
    Serial.println("✗ Cliente desconectado");
  }
};

// ==================================
//   SETUP
//   Configura pines, ADC y BLE
// ==================================
void setup() {
  Serial.begin(115200);

  pinMode(ledEstado, OUTPUT);
  pinMode(ledBLE, OUTPUT);
  pinMode(SIGNAL_PIN, INPUT);

  // Configuración ADC del ESP32-C3
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);  // permite lectura 0-3.3V reales

  // --- BLE inicialización ---
  BLEDevice::init("EWHRA");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Crear servicio
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Característica del RMS (en bytes)
  pADCCharacteristic = pService->createCharacteristic(
    ADC_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pADCCharacteristic->addDescriptor(new BLE2902());

  // Característica del nivel en texto
  pNivelCharacteristic = pService->createCharacteristic(
    NIVEL_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pNivelCharacteristic->addDescriptor(new BLE2902());
  pNivelCharacteristic->setValue("BAJO"); // valor inicial

  pService->start();

  // Publicar el servicio BLE
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();

  Serial.println("✓ Servicio BLE iniciado");
  Serial.println("→ Esperando conexión...\n");
}

// =========================================
//   CÁLCULO DE RMS (ANTI-SATURACIÓN)
//   Lee  el ADC, calcula RMS y lo suaviza
// =========================================
float calcularRMS() {

  uint64_t sumaCuadrados = 0; // 64 bits evita saturarse

  for (int i = 0; i < muestras; i++) {
    int raw = analogRead(SIGNAL_PIN); // lectura ADC

    sumaCuadrados += (uint64_t)raw * (uint64_t)raw;

    delayMicroseconds(200); // controla frecuencia de muestreo
  }

  // RMS del ADC sin convertir (0–4095)
  double rmsRaw = sqrt((double)sumaCuadrados / (double)muestras);

  // Conversión a voltaje real (0–3.3V)
  float voltRMS = (rmsRaw / ADC_MAX_COUNT) * ADC_MAX_VOLT;

  // Filtro exponencial 
  rmsSuave = (alpha * voltRMS) + (1 - alpha) * rmsSuave;

  return rmsSuave;
}

// ======================================================
//   LOOP PRINCIPAL
//   Envía RMS, nivel, y controla LEDs
// ======================================================
void loop() {

  digitalWrite(ledEstado, HIGH); // indicador de actividad

  // Si hay conexión BLE, mandar datos
  if (deviceConnected) {

    float valorRMS = calcularRMS();  // obtiene RMS filtrado

    // Clasificación por nivel de voltaje
    String nivel;
    if (valorRMS >= 2.2) nivel = "ALTO";
    else if (valorRMS >= 1.1) nivel = "MEDIO";
    else nivel = "BAJO";

    // Escribe en el monitor serial
    Serial.print("RMS = ");
    Serial.print(valorRMS, 3);
    Serial.print(" V  | Nivel = ");
    Serial.println(nivel);

    // Convertir RMS a milivoltios (mV)
    uint16_t valorInt = (uint16_t)(valorRMS * 1000.0);

    // Empaquetar en 2 bytes (LSB + MSB)
    uint8_t buffer[2];
    buffer[0] = valorInt & 0xFF;
    buffer[1] = (valorInt >> 8) & 0xFF;

    // Enviar por BLE
    pADCCharacteristic->setValue(buffer, 2);
    pADCCharacteristic->notify();

    // Enviar nivel (string)
    pNivelCharacteristic->setValue(nivel.c_str());
    pNivelCharacteristic->notify();
  }

  // Manejo automático de reconexión BLE
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising(); // reinicia advertising
    Serial.println("→ Advertising reiniciado");
  }

  if (deviceConnected && !oldDeviceConnected) {
    Serial.println("→ Cliente conectado, enviando datos...\n");
  }

  oldDeviceConnected = deviceConnected;
}
