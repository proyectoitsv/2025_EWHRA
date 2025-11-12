
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLE2901.h>

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLE2901 *descriptor_2901 = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

// Pin analógico - GPIO 0, 1, 2, 3 o 4 son las mejores opciones
#define SIGNAL_PIN 0  // Cambia a 0, 1, 2, 3 o 4 según tu conexión

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("Cliente conectado");
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("Cliente desconectado");
  }
};
#include <Arduino.h>

void setup() {
  Serial.begin(115200);

  // Configurar resolución del ADC a 12 bits (0-4095)
  analogReadResolution(12);

  pinMode(SIGNAL_PIN, INPUT);

  // Create the BLE Device
  BLEDevice::init("EWHRA");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);

  // Creates BLE Descriptor 0x2902: Client Characteristic Configuration Descriptor (CCCD)
  pCharacteristic->addDescriptor(new BLE2902());

  // Adds also the Characteristic User Description - 0x2901 descriptor
  descriptor_2901 = new BLE2901();
  descriptor_2901->setDescription("Analog sensor voltage reading");
  descriptor_2901->setAccessPermissions(ESP_GATT_PERM_READ);
  pCharacteristic->addDescriptor(descriptor_2901);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();

  Serial.println("Esperando conexión de cliente...");
}

void loop() {
  // notify changed value
  if (deviceConnected) {
    // Leer valor analógico (0-4095 con 12 bits)
    int adcValue = analogRead(SIGNAL_PIN);

    // Convertir a voltaje (ESP32-C3 tiene referencia de 3.3V)
    float voltaje = adcValue * (3.3 / 4095.0);

    Serial.print("ADC: ");
    Serial.print(adcValue);
    Serial.print(" | Voltaje: ");
    Serial.print(voltaje, 3);
    Serial.println(" V");

    // Convertir el voltaje a string y enviarlo por BLE
    char entrada[16];
    dtostrf(voltaje, 4, 2, entrada);  // Formato: 4 dígitos totales, 2 decimales

    // O si prefieres enviar ambos valores:
    // sprintf(entrada, "%d,%.2f", adcValue, voltaje);

    pCharacteristic->setValue(entrada);
    pCharacteristic->notify();
    Serial.print("nashe");

    delay(10);
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("Reiniciando advertising");
    oldDeviceConnected = deviceConnected;
  }

  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}