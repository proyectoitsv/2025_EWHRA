#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define ADC_PIN A0   // Cambiá esto al pin que uses para tu señal EEG

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
uint32_t lastSendTime = 0;

// UUIDs personalizados (podés mantener los del ejemplo si querés)
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Inicializando BLE...");

  BLEDevice::init("EWHRA_EEG"); // Nombre del dispositivo BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  pServer->getAdvertising()->start();

  Serial.println("BLE listo, esperando conexión...");
}

void loop() {
  if (deviceConnected) {
    // Leer valor del ADC
    int sensorValue = analogRead(ADC_PIN);

    // Convertirlo a string para enviar por BLE
    String dataToSend = String(sensorValue);

    // Enviar el valor
    pCharacteristic->setValue(dataToSend.c_str());
    pCharacteristic->notify();

    // Mostrar también por Serial
    Serial.print("Valor EEG enviado: ");
    Serial.println(sensorValue);

    delay(100); // cada 100 ms (ajustable)
  } else {
    // Esperando conexión
    delay(500);
  }
}
