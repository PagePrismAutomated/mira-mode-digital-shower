
//basic wireless bluetooth on/off control for Mira Mode shower/bath
//tested on Mira Mode dual outlet digital shower/bath purchased 2023 in UK
//tested on old ESP32-WROOM-32 and ESP32-WROOM-32E (FireBeetle 2 dev. board)
//Mira Mode requires pairing before you can write to characteristics
//hold shower on button for 6 seconds until light flashes, then send 'p' over serial to pair
//send commands from serial to test then reduce to a device with buttons etc. 
//inspired by this library https://github.com/alexpilotti/python-miramode but comms different
//included code to receive notifications so can be developed to get feedback etc.

#include "NimBLEDevice.h"

static NimBLEUUID serviceUUID("267f0001-eb15-43f5-94c3-67d2221188f7"); // The remote service
static NimBLEUUID    char2UUID("267f0002-eb15-43f5-94c3-67d2221188f7"); // The write characteristic
static NimBLEUUID    char3UUID("267f0003-eb15-43f5-94c3-67d2221188f7"); // The notify characteristic
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static NimBLERemoteCharacteristic* pRemoteCharacteristic;
static NimBLERemoteCharacteristic* cRemoteCharacteristic;
static NimBLEAdvertisedDevice* myDevice;
uint16_t myConnId;
int incomingByte = 0; //for serial monitor
byte myCommandData [4][10] = {
  {0xaa, 0x55, 0x00, 0xab, 0x04, 0x01, 0x7c, 0x50, 0x01, 0x84},  //0 bath on
  {0xaa, 0x55, 0x00, 0xab, 0x04, 0x01, 0x7c, 0x50, 0x02, 0x83},  //1 shower on
  {0xaa, 0x55, 0x00, 0xab, 0x04, 0x01, 0x7c, 0x58, 0x02, 0x7b},  //2 default shower on
  {0xaa, 0x55, 0x00, 0xab, 0x04, 0x01, 0x7c, 0x50, 0x00, 0x85}   //3 off
};

static void notifyCallback(
  NimBLERemoteCharacteristic* pNimBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pNimBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println(String((char *)pData));
    //Serial.println((char*)pData);
}


class MyClientCallback : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* pclient) {
  }

  void onDisconnect(NimBLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer(bool getReferences) {   
  myScan(); //scan for device
  if (doConnect == true) { //doConnect means the shower was found in the scan
    NimBLEClient*  pClient  = NimBLEDevice::createClient();
    Serial.println(" - Created client");
    pClient->setClientCallbacks(new MyClientCallback());
    // Connect to the remote Server
    if(pClient->connect(myDevice)){  // if you pass NimBLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
      Serial.println(" - Connected to server");
      }else{
        Serial.println(" - Failed to connect");
        NimBLEDevice::deleteClient(pClient);
        return false;
        }
    myConnId = pClient->getConnId();
    pClient->secureConnection();
    connected = true;
   
   if(getReferences){ //if we are not pairing then continue to find the service and characteristics
     // Obtain a reference to the service we are after in the remote server.
      NimBLERemoteService* pRemoteService = pClient->getService(serviceUUID);
      if (pRemoteService == nullptr) {
        Serial.print(" - Failed to find our service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        myDisconnect();
        return false;
      }else{Serial.println(" - Found our service");}
  
      // Obtain a reference to the 2 characteristic
      pRemoteCharacteristic = pRemoteService->getCharacteristic(char2UUID);
      if (pRemoteCharacteristic == nullptr) {
        Serial.print(" - Failed to find char 2 UUID: ");
        myDisconnect();
        return false;
      }else{Serial.println(" - Found 2 characteristic");}
  
      // Obtain a reference to the 3 'notify' characteristic, information is returned
      // after writing requests to the 2 characteristic.    
      cRemoteCharacteristic = pRemoteService->getCharacteristic(char3UUID);
      if (cRemoteCharacteristic == nullptr) {
        Serial.print(" - Failed to find char 3 UUID: ");
        myDisconnect();
        return false;
      }else{Serial.println(" - Found 3 characteristic");}
  
       if(cRemoteCharacteristic->canNotify()){
        cRemoteCharacteristic->registerForNotify(notifyCallback);
        Serial.println(" - Characteristic 3 registered for callback");
      }
   } //getReferences
   return true; //success, connected and got services and characteristics
  }else{return false;} //do connect, device not found in the scan
}
/**
 * Scan for NimBLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising NimBLE server.
   */
  void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
    Serial.print("NimBLE Advertised Device found: ");
    Serial.println(advertisedDevice->toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(serviceUUID)) {

      NimBLEDevice::getScan()->stop();
      myDevice = advertisedDevice;
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks

void myCommand(int myRow){
    if (NimBLEDevice::getNumBonds() == 0){  
      Serial.println(" - Pairing is required"); //server needs to be placed into pairing mode then run pair function
      return; //comment out this line to see it will connect but does not allow writing unless paired
    }
    if (connectToServer(true)) {      //connect, send true to also get the services and characterisitcs
      pRemoteCharacteristic->writeValue(myCommandData[myRow], sizeof(myCommandData[myRow]), false); //writes to characteristic    
      myDisconnect();
    }else{
      Serial.println(" - Write failed");
      doConnect = false;
    }
}

void myDisconnect(){
    NimBLEClient*  pClient  = NimBLEDevice::getClientByID(myConnId);
    pClient->disconnect();
    NimBLEDevice::deleteClient(pClient);
    doConnect = false;
}

void myDeleteBonds(){
  Serial.printf(" - Number of bonds: %d\n", NimBLEDevice::getNumBonds());
  if (NimBLEDevice::getNumBonds() > 0){NimBLEDevice::deleteAllBonds();  //clears any past information
    Serial.printf(" - Delete all bonds\n - Number of bonds: %d\n", NimBLEDevice::getNumBonds());
  }
}

void myShowBonds(){
  Serial.printf(" - Bond Status: %s\n", NimBLEDevice::isBonded(myDevice->getAddress()) ? "bonded" : "not bonded");
  Serial.printf(" - Number of bonds: %d\n", NimBLEDevice::getNumBonds());
}

 bool myDoPairing(){
  if (NimBLEDevice::getNumBonds() > 0){NimBLEDevice::deleteAllBonds();}  //clears all bonds
    if (connectToServer(false)){ //send false we don't need to get services and characteristics
      unsigned long x = millis();
      while(!NimBLEDevice::isBonded(myDevice->getAddress()) && millis() - x < 5000){
        //wait for bonding to complete
      }
      myDisconnect();
  }
  Serial.printf(" - Bond Status: %s\n", NimBLEDevice::isBonded(myDevice->getAddress()) ? "bonded" : "not bonded");
  return NimBLEDevice::isBonded(myDevice->getAddress());    
}

void myScan(){
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the scan to run for 5 seconds.
  NimBLEScan* pNimBLEScan = NimBLEDevice::getScan();
  pNimBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pNimBLEScan->setInterval(1349);
  pNimBLEScan->setWindow(449);
  pNimBLEScan->setActiveScan(true);
  pNimBLEScan->start(5, false);
}

void setup() { 

  Serial.begin(115200);
  delay(500);
  Serial.println(" - Enter command");
  
  NimBLEDevice::init("");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setMTU(247); //Mira will request this so setup now
  NimBLEDevice::setSecurityAuth(true, false, true); //bonding, MITM, SC (false for legacy pair)
  myScan();
}


void loop() {
  
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
        switch (incomingByte) {
      case '0':                 //bath on
      case '1':                 //shower on
      case '2':                 //default shower on
      case '3':                 //off
        myCommand(incomingByte-48);
        break;
      case 'p':					//send 'p' to pair and bond
        myDoPairing();
        break;
      case 's':
        myShowBonds();
        break;
      case 'd':
        myDeleteBonds();
        break;
    }
  }
}