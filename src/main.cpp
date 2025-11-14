// --- Inclusión de librerías BLE del ESP32 ---
#include <BLEDevice.h>  // Manejo del dispositivo BLE (inicialización, nombre, etc.)
#include <BLEServer.h>  // Permite crear un servidor BLE (ESP32 actúa como periférico)
#include <BLEUtils.h>   // Utilidades auxiliares para BLE
#include <BLE2902.h>    // Descriptor 0x2902: permite al cliente activar NOTIFY/INDICATE

// --- Declaraciones globales de objetos BLE ---
BLEServer *pServer = NULL;                     // puntero al servidor BLE
BLECharacteristic *pADCCharacteristic = NULL;  // característica para el valor ADC crudo

// --- Flags de estado de conexión ---
bool deviceConnected = false;     // true si hay un cliente BLE conectado actualmente
bool oldDeviceConnected = false;  // estado anterior (usado para detectar cambios)

#define SIGNAL_PIN 0  // Pin analógico usado para leer la señal (elegir 0..4 en ESP32-C3)

#define ledEstado 10  // Pin del LED de estado de conexión
#define ledBLE 9      // Pin del LED de estado de BLE

// --- UUIDs del servicio y características (COMPATIBLES CON LA APP EWHRA) ---
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define ADC_CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// --- Clase para manejar eventos de conexión/desconexión del servidor BLE ---
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {  // Se llama cuando un cliente se conecta
    deviceConnected = true;             // actualizar flag
    Serial.println("✓ Cliente conectado a EWHRA");
  };
  void onDisconnect(BLEServer *pServer) {  // Se llama cuando un cliente se desconecta
    deviceConnected = false;               // actualizar flag
    Serial.println("✗ Cliente desconectado");
  }
};

void setup() {
  Serial.begin(115200);  // Iniciar monitor serie a 115200 baudios

  pinMode(ledEstado, OUTPUT);  //Definir como salida el pin del LED de estado
    pinMode(ledBLE, OUTPUT);  //Definir como salida el pin del LED de BLE

  Serial.println("\n=== EWHRA - Sistema de Monitoreo EEG ===");

  analogReadResolution(12);    // Establecer resolución ADC a 12 bits => lecturas 0..4095
  pinMode(SIGNAL_PIN, INPUT);  // Configurar el pin analógico como entrada

  BLEDevice::init("EWHRA");  // Inicializar stack BLE y poner nombre visible "EWHRA"

  pServer = BLEDevice::createServer();             // Crear el servidor BLE
  pServer->setCallbacks(new MyServerCallbacks());  // Asignar callbacks de conexión

  BLEService *pService = pServer->createService(SERVICE_UUID);  // Crear servicio con UUID

  // --- Característica única: Valor ADC (0-4095) ---
  pADCCharacteristic = pService->createCharacteristic(
    ADC_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pADCCharacteristic->addDescriptor(new BLE2902());  // Añadir descriptor 0x2902 para NOTIFY
  pADCCharacteristic->setValue("0");                 // Valor inicial

  pService->start();  // Iniciar el servicio BLE (hace disponible las características)

  // --- Advertising (publicidad) para que clientes encuentren el servicio ---
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);  // Incluir el UUID del servicio en el advertising
  pAdvertising->setScanResponse(true);         // Habilitar respuesta de escaneo
  pAdvertising->setMinPreferred(0x06);         // Configurar intervalo mínimo
  pAdvertising->setMaxPreferred(0x12);         // Configurar intervalo máximo
  BLEDevice::startAdvertising();               // Empezar a anunciarse

  Serial.println("✓ Servicio BLE iniciado");
  Serial.println("✓ Dispositivo: EWHRA");
  Serial.print("✓ Servicio UUID: ");
  Serial.println(SERVICE_UUID);
  Serial.print("✓ ADC Characteristic UUID: ");
  Serial.println(ADC_CHARACTERISTIC_UUID);
  Serial.println("→ Esperando conexión de la app...\n");
}

void loop() {

  digitalWrite(ledEstado, HIGH);

  if (!deviceConnected) {
    digitalWrite(ledBLE, HIGH);
    delay(500);
    digitalWrite(ledBLE, LOW);
    delay(500);
  }
  // --- Si hay un cliente conectado, leer ADC y notificar ---
  if (deviceConnected) {
    digitalWrite(ledBLE, LOW);
    delay(500);
    int adcValue = analogRead(SIGNAL_PIN);      // Leer ADC: 0..4095 (12 bits)
    float voltaje = adcValue * (3.3 / 4095.0);  // Convertir a voltaje (solo para debug local)

    // Imprimir en monitor serie para depuración local
    Serial.print("ADC: ");
    Serial.print(adcValue);
    Serial.print(" (");
    Serial.print(voltaje, 2);
    Serial.print("V) | Nivel: ");

    // Mostrar nivel según umbrales (igual que en la app)
    if (adcValue >= 3000) {
      Serial.println("🟢 ALTO");
    } else if (adcValue >= 1500) {
      Serial.println("🟡 MEDIO");
    } else {
      Serial.println("🔴 BAJO");
    }

    // Preparar string para enviar por BLE
    char adcStr[8];                   // buffer para el valor ADC como texto
    sprintf(adcStr, "%d", adcValue);  // convertir entero ADC a string (ej. "2048")

    // Enviar notificación BLE
    pADCCharacteristic->setValue(adcStr);  // asignar valor a la característica ADC
    pADCCharacteristic->notify();          // notificar a clientes suscritos

    delay(100);  // Intervalo de 100ms = 10Hz (óptimo para monitoreo en tiempo real)
  }

  // --- Manejo de re-advertising si el cliente se desconectó ---
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // breve espera para que el stack BLE estabilice
    pServer->startAdvertising();  // volver a anunciar el servicio (aceptar nuevas conexiones)
    Serial.println("→ Reiniciando advertising...");
    oldDeviceConnected = deviceConnected;  // sincronizar estado previo
  }

  // --- Actualizar estado 'oldDeviceConnected' cuando se detecta nueva conexión ---
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;  // sincronizar estado previo
    Serial.println("→ Iniciando transmisión de datos EEG...\n");
  }
}