/*
  ESP32 BLE EMG Sensor Streamer
  
  This Arduino sketch sets up a Bluetooth Low Energy (BLE) server on an ESP32.
  It reads from 11 analog input pins (configured for MyoWare / BioAMP EMG sensors),
  formats the readings as a comma-separated string, and notifies the connected
  Web Bluetooth client (our HTML control center).
  
  Service UUID:        4fafc201-1fb5-459e-8fcc-c5c9c331914b
  Characteristic UUID: beb5483e-36e1-4688-b7f5-ea07361b26a8
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE UUID Configuration
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// 11 Analog Pins corresponding to the 11 EMG channels.
// ADJUST THESE PINS TO MATCH YOUR ACTUAL ESP32 WIRING.
// Note: ESP32 ADC1 pins are recommended (GPIO 32-39) as ADC2 cannot be used when WiFi/Bluetooth is active.
const int emgPins[11] = {
  32, // Channel 0: gastrocmed (Calf)
  33, // Channel 1: tibialisanterior (Shin)
  34, // Channel 2: soleus (Calf Base)
  35, // Channel 3: vastusmedialis (Quad)
  36, // Channel 4: vastuslateralis (Quad)
  39, // Channel 5: rectusfemoris (Quad)
  32, // Channel 6: bicepsfemoris (Hamstring) - Reuse or map to other ADC1 pins
  33, // Channel 7: semitendinosus (Hamstring)
  34, // Channel 8: gracilis (Inner Thigh)
  35, // Channel 9: gluteusmedius (Hip)
  36  // Channel 10: (Extra / Reference)
};

// BLE Connection Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Dashboard connected!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Dashboard disconnected.");
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESP32 EMG Streamer...");

  // Initialize analog pins as inputs
  for (int i = 0; i < 11; i++) {
    pinMode(emgPins[i], INPUT);
  }

  // Create the BLE Device
  BLEDevice::init("ESP32 Exoskeleton EMG");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create the BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Create a BLE Descriptor (Needed for Notifications)
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issues
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE Server is active. Ready to pair with the Dashboard!");
}

void loop() {
    // Only send data if a device is connected
    if (deviceConnected) {
        String dataPayload = "";
        
        // Read 11 EMG channels
        for (int i = 0; i < 11; i++) {
            // Read ADC value (0 - 4095 on ESP32)
            int rawAdc = analogRead(emgPins[i]);
            
            // Convert to voltage (0.0V - 3.3V) as standard sensor input format
            float voltage = (rawAdc / 4095.0) * 3.3;
            
            // Append to payload
            dataPayload += String(voltage, 3); // 3 decimal places
            if (i < 10) {
                dataPayload += ",";
            }
        }
        
        // Print payload to Serial Monitor for debugging
        Serial.println("Sending: " + dataPayload);

        // Update BLE characteristic value and notify client
        pCharacteristic->setValue(dataPayload.c_str());
        pCharacteristic->notify();
        
        // Send sample every 20ms (50 Hz sampling rate)
        // Adjust the delay to control throughput. Keep in mind BLE throughput limitations.
        delay(20); 
    }
    
    // Handle disconnection / reconnection
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("Restarted advertising...");
        oldDeviceConnected = deviceConnected;
    }
    
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff on connection
        oldDeviceConnected = deviceConnected;
    }
}
