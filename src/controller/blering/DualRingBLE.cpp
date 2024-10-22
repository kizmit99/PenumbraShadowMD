#include <Arduino.h>
#include <NimBLEDevice.h>
#include "DualRingBLE.h"
#include "Ring.h"
#include "debug.h"
#include "../../LocalPreferences.h"

static const char HID_SERVICE[] = "1812";
static const char HID_REPORT_MAP[] = "2A4B";
static const char HID_REPORT_DATA[] = "2A4D";
static const char HID_PROTOCOL_MODE[] = "2A4E";
static const char BATTERY_SERVICE[] = "180F";
static const char BATTERY_LEVEL[] = "2A19";

static NimBLEUUID HID_REPORT_DATA_UUID = NimBLEUUID(HID_REPORT_DATA);
static NimBLEUUID HID_PROTOCOL_MODE_UUID = NimBLEUUID(HID_PROTOCOL_MODE);

void scanEndedCB(NimBLEScanResults results);
static uint32_t scanTime = 0; /** 0 = scan forever */

#define DRIVEMAC_KEY "DriveMAC"
#define DOMEMAC_KEY "DomeMAC"

char DriveMAC[RingAddressMaxLen] = "XX:XX:XX:XX:XX:XX";
char DomeMAC[RingAddressMaxLen]  = "XX:XX:XX:XX:XX:XX";

/** Define a class to handle the client callbacks */
class ClientCallbacks : public NimBLEClientCallbacks {

    void onConnect(NimBLEClient* pClient) {
        const char* peerAddress = pClient->getPeerAddress().toString().c_str();
        DBG_printf("Connected to: %s\r\n", peerAddress);
        pClient->updateConnParams(120,120,0,60);
        Ring* driveRing = rings.getRing(DualRingBLE::Drive);
        Ring* domeRing = rings.getRing(DualRingBLE::Dome);
        if (!strncmp(domeRing->address, peerAddress, sizeof(domeRing->address))) {
            domeRing->onConnect();
        }
        if (!strncmp(driveRing->address, peerAddress, sizeof(driveRing->address))) {
            driveRing->onConnect();
        }
    };

    void onDisconnect(NimBLEClient* pClient) {
        const char* peerAddress = pClient->getPeerAddress().toString().c_str();
        DBG_printf("%s Disconnected\r\n", peerAddress);
        Ring* driveRing = rings.getRing(DualRingBLE::Drive);
        Ring* domeRing = rings.getRing(DualRingBLE::Dome);
        if (!strncmp(domeRing->address, peerAddress, sizeof(domeRing->address))) {
            //DBG_printf("onDisconnect dome(%s, %s)\r\n",domeRing->address, peerAddress);
            domeRing->onDisconnect();
            domeRing->waitingFor = true;
        }
        if (!strncmp(driveRing->address, peerAddress, sizeof(driveRing->address))) {
            //DBG_printf("onDisconnect drive(%s, %s)\r\n",driveRing->address, peerAddress);
            driveRing->onDisconnect();
            driveRing->waitingFor = true;
        }
        if (!NimBLEDevice::getScan()->isScanning()) {
            DBG_println("Restarting scan");
            NimBLEDevice::getScan()->start(scanTime, scanEndedCB);
        }
    };

    /** Pairing process complete, we can check the results in ble_gap_conn_desc */
    void onAuthenticationComplete(ble_gap_conn_desc* desc){
        DBG_println(__func__);
        if(!desc->sec_state.encrypted) {
            DBG_println("Encrypt connection failed - disconnecting");
            /** Find the client with the connection handle provided in desc */
            NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
            return;
        }
    };
};

/** Create a single global instance of the callback class to be used by all clients */
static ClientCallbacks clientCB;

/** Define a class to handle the callbacks when advertisments are received */
class AdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {

    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        uint8_t advType = advertisedDevice->getAdvType();
        if ((advType == BLE_HCI_ADV_TYPE_ADV_DIRECT_IND_HD) ||
            (advType == BLE_HCI_ADV_TYPE_ADV_DIRECT_IND_LD) ||
            (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(NimBLEUUID(HID_SERVICE)))) {
            DBG_printf("Advertised HID Device found: %s\r\n", advertisedDevice->toString().c_str());
            DBG_printf("Name = %s\r\n", advertisedDevice->getName().c_str());
            DBG_printf("Name we're looking for = %s\r\n", "Magicsee R1");

            Ring* driveRing = rings.getRing(DualRingBLE::Drive);
            Ring* domeRing = rings.getRing(DualRingBLE::Dome);

            if (strstr(advertisedDevice->getName().c_str(), "Magicsee R1") != NULL) {
                DBG_printf("Match found! (type=%d)\r\n", advertisedDevice->getAddress().getType());
                if (!strncmp(DriveMAC, advertisedDevice->getAddress().toString().c_str(), sizeof(DriveMAC))) {
                    //Found Drive Ring by saved MAC
                    if (driveRing->waitingFor) {
                        DBG_println("Reassigning to Drive");
                        driveRing->waitingFor = false;
                        driveRing->advertisedDevice = advertisedDevice;
                        driveRing->connectTo = true;
                    } else {
                        DBG_println("Drive Ring already assigned!");
                    }
                } else if (!strncmp(DomeMAC, advertisedDevice->getAddress().toString().c_str(), sizeof(DomeMAC))) {
                    //Found Dome Ring by saved MAC
                    if (domeRing->waitingFor) {
                        DBG_println("Reassigning to Dome");
                        domeRing->waitingFor = false;
                        domeRing->advertisedDevice = advertisedDevice;
                        domeRing->connectTo = true;
                    } else {
                        DBG_println("Dome Ring already assigned!");
                    }
                } else {
                    //We have an unknown Ring
                    if ((driveRing->waitingFor) &&
                        (DriveMAC[0] == 'X')) {
                        //Unrecognized Ring and Drive Ring doesn't have an assigned address yet
                        DBG_println("Assigning to Drive");
                        driveRing->waitingFor = false;
                        driveRing->advertisedDevice = advertisedDevice;
                        driveRing->connectTo = true;
                    } else if ((domeRing->waitingFor) &&
                               (DomeMAC[0] == 'X')) {
                        //Unrecognized Ring and Dome Ring doesn't have an assigned address yet
                        DBG_println("Assigning to Dome");
                        domeRing->waitingFor = false;
                        domeRing->advertisedDevice = advertisedDevice;
                        domeRing->connectTo = true;
                    } else {
                        DBG_println("Neither ring claimed the connection");
                    }
                }
            } else {
                DBG_println("Not a Match");
            }
            if ((!driveRing->waitingFor && !domeRing->waitingFor) && 
                NimBLEDevice::getScan()->isScanning()) {
                DBG_printf("Stopping Scan in AdvDeviceCallback driveWait=%d, domeWait=%d\r\n", driveRing->waitingFor, domeRing->waitingFor);
                NimBLEDevice::getScan()->stop();
            }
        }
    };
};

/** Notification / Indication receiving handler callback */
void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic,
              uint8_t* pData, size_t length, bool isNotify) {
    if (pRemoteCharacteristic->getUUID() == HID_REPORT_DATA_UUID) {
        NimBLEAddress peerAddress = pRemoteCharacteristic->getRemoteService()->getClient()->getPeerAddress();
        Ring* driveRing = rings.getRing(DualRingBLE::Drive);
        Ring* domeRing = rings.getRing(DualRingBLE::Dome);
        if (peerAddress == driveRing->bleAddress) {
            driveRing->onReport(pData, length);
        } else if (peerAddress == domeRing->bleAddress) {
            domeRing->onReport(pData, length);
        } else {
            DBG_printf("Unexpected report from peer: %s\r\n", peerAddress);
        }
    }
};

/** Handles the provisioning of clients and connects / interfaces with the server */
bool connectToServer(Ring* ring) {
    NimBLEClient* pClient = nullptr;
    bool reconnected = false;

    DBG_println(__func__);

    /** Check if we have a client we should reuse first **/
    if (NimBLEDevice::getClientListSize()) {
        pClient = NimBLEDevice::getClientByPeerAddress(ring->advertisedDevice->getAddress());
        if (pClient && (pClient->getPeerAddress() == ring->bleAddress)) {
            if (!pClient->connect(ring->advertisedDevice, false)) {
                DBG_println("Reconnect failed");
                return false;
            }
            DBG_println("Reconnected client");
            reconnected = true;
        } else {
            // We don't already have a client that knows this device,
            //  we will check for a client that is disconnected that we can use.
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }

    /** No client to reuse? Create a new one. */
    if (pClient == NULL) {
        if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
            DBG_println("Max clients reached - no more connections available");
            return false;
        }

        pClient = NimBLEDevice::createClient();
        DBG_println("New client created");

        pClient->setClientCallbacks(&clientCB, false);
        pClient->setConnectionParams(12,12,0,51);
        pClient->setConnectTimeout(5);


        if (!pClient->connect(ring->advertisedDevice)) {
            /** Created a client but failed to connect, don't need to keep it as it has no data */
            NimBLEDevice::deleteClient(pClient);
            DBG_println("Failed to connect, deleted client");
            return false;
        }
    }

    if (!pClient->isConnected()) {
        if (!pClient->connect(ring->advertisedDevice)) {
            DBG_println("Failed to connect");
            return false;
        }
    }

    ring->bleAddress = pClient->getPeerAddress();
    strncpy(ring->address, pClient->getPeerAddress().toString().c_str(), sizeof(ring->address));
    DBG_print("Connected to: ");
    DBG_println(ring->address);

    NimBLERemoteService *hidService = pClient->getService(HID_SERVICE);

    if (hidService != NULL) {
        std::vector<NimBLERemoteCharacteristic*>*charvector;
        charvector = hidService->getCharacteristics(true);
        // For each characteristic
        for (auto &it: *charvector) {
            if (it->getUUID() == HID_PROTOCOL_MODE_UUID) {
                if (it->canRead()) {
                    it->readValue();
                }
            } else if (it->getUUID() == HID_REPORT_DATA_UUID) {
                if (it->canNotify()) {
                    if (it->subscribe(true, notifyCB)) {
                        DBG_println("subscribe notification OK");
                    } else {
                        /** Disconnect if subscribe failed */
                        DBG_println("subscribe notification failed");
                        pClient->disconnect();
                        return false;
                    }
                }
            }
        }
    }

    DBG_println("Done with this device!");
    return true;
}

/** Callback to process the results of the last scan */
void scanEndedCB(NimBLEScanResults results){
    DBG_println("Scan Ended");
}

void DualRingBLE::clearMACMap() {
    preferences.clear();
}

void DualRingBLE::init(const char *name) {

    this->name = name;
    driveRing.setOtherRing(&domeRing);
    domeRing.setOtherRing(&driveRing);
    driveRing.init();
    domeRing.init();

    DBG_printf("Before loading Prefs, Drive: %s, Dome: %s\r\n", DriveMAC, DomeMAC);
    if (preferences.isKey(DRIVEMAC_KEY)) {
        DBG_printf("Key Found: %s\r\n", DRIVEMAC_KEY);
        preferences.getString(DRIVEMAC_KEY, DriveMAC, sizeof(DriveMAC));
    }
    if (preferences.isKey(DOMEMAC_KEY)) {
        DBG_printf("Key Found: %s\r\n", DRIVEMAC_KEY);
        preferences.getString(DOMEMAC_KEY, DomeMAC, sizeof(DomeMAC));
    }
    DBG_printf("After  loading Prefs, Drive: %s, Dome: %s\r\n", DriveMAC, DomeMAC);

    NimBLEDevice::init(name);
    //Begin listening for advertisements

    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
    NimBLEScan* pScan = NimBLEDevice::getScan();

    if ((DriveMAC[0] != 'X') &&
        (DomeMAC[0] != 'X')) {      //We have two MACs, enabled whitelist for scan
        DBG_printf("Two MACs defined, enabling scan whitelist; drive: %s, dome: %s\r\n", DriveMAC, DomeMAC);
        NimBLEDevice::whiteListAdd(NimBLEAddress(DriveMAC));
        NimBLEDevice::whiteListAdd(NimBLEAddress(DomeMAC));
        pScan->setFilterPolicy(BLE_HCI_SCAN_FILT_USE_WL);
    } else {
        DBG_printf("Not enabling scan whitelist; drive: %s, dome: %s\r\n", DriveMAC, DomeMAC);
    }
    
    /** create a callback that gets called when advertisers are found */
    pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());

    /** Set scan interval (how often) and window (how long) in milliseconds */
    pScan->setInterval(22);
    pScan->setWindow(11);

    pScan->setActiveScan(false);

    DBG_println("Scanning");
    pScan->start(scanTime, scanEndedCB);
}

void DualRingBLE::task() {
    if (driveRing.connectTo) {
        driveRing.connectTo = false;
        if (!connectToServer(&driveRing)) {
            driveRing.waitingFor = true;
        } else {
            //DBG_printf("After Drive Connect, before NVStore, DriveMAC: %s, DriveAddr: %s\r\n", DriveMAC, driveRing.address);
            if (strncmp(DriveMAC, driveRing.address, sizeof(DriveMAC))) {
                preferences.putString(DRIVEMAC_KEY, driveRing.address);
                preferences.getString(DRIVEMAC_KEY, DriveMAC, sizeof(DriveMAC));
                //DBG_printf("After Drive Connect, after  NVStore, DriveMAC: %s, DriveAddr: %s\r\n", DriveMAC, driveRing.address);
            } else {
                //DBG_println("Not storing to NVRAM");
            }
        }
    }
    if (domeRing.connectTo) {
        domeRing.connectTo = false;
        if (!connectToServer(&domeRing)) {
            domeRing.waitingFor = true;
        } else {
            //DBG_printf("After Dome Connect, before NVStore, DomeMAC: %s, DomeAddr: %s\r\n", DomeMAC, domeRing.address);
            if (strncmp(DomeMAC, domeRing.address, sizeof(DomeMAC))) {
                preferences.putString(DOMEMAC_KEY, domeRing.address);
                preferences.getString(DOMEMAC_KEY, DomeMAC, sizeof(DomeMAC));
                //DBG_printf("After Dome Connect, after  NVStore, DomeMAC: %s, DomeAddr: %s\r\n", DomeMAC, domeRing.address);
            } else {
                //DBG_println("Not storing to NVRAM");
            }
        }
    }
    if ((driveRing.waitingFor || domeRing.waitingFor) && 
        !NimBLEDevice::getScan()->isScanning()) {
        NimBLEDevice::getScan()->start(scanTime, scanEndedCB);
    }

    driveRing.task();
    domeRing.task();
}

Ring *DualRingBLE::getRing(Controller controller) {
    switch (controller) {
        case Dome:  return &domeRing;
        case Drive: return &driveRing;
        default:
            DBG_println("ERROR in DualRingBLE::getRing, invalid controller");
            return NULL;
    }
}

bool DualRingBLE::isModifierPressed(Controller controller, Modifier button) {
    switch (button) {
        case A:    return getRing(controller)->isButtonPressed(MagicseeR1::A);
        case B:    return getRing(controller)->isButtonPressed(MagicseeR1::B);
        case L2:   return getRing(controller)->isButtonPressed(MagicseeR1::L2);
        default:   return false;
    }
}

bool DualRingBLE::isButtonPressed(Controller controller, Button button) {
    switch (button) {
        case Up:     return getRing(controller)->isButtonPressed(MagicseeR1::UP);
        case Down:   return getRing(controller)->isButtonPressed(MagicseeR1::DOWN);
        case Left:   return getRing(controller)->isButtonPressed(MagicseeR1::LEFT);
        case Right:  return getRing(controller)->isButtonPressed(MagicseeR1::RIGHT);
        default:     return false;
    }
}

bool DualRingBLE::isButtonClicked(Controller controller, Clicker button) {
    switch (button) {
        case C:     return getRing(controller)->isButtonClicked(MagicseeR1::C);
        case D:     return getRing(controller)->isButtonClicked(MagicseeR1::D);
        case L1:    return getRing(controller)->isButtonClicked(MagicseeR1::L1);
        default:    return false;
    }
}

int8_t DualRingBLE::getJoystick(Controller controller, Axis axis) {
    switch (axis) {
        case X:     return getRing(controller)->getJoystick(MagicseeR1::X);
        case Y:     return getRing(controller)->getJoystick(MagicseeR1::Y);
        default:    return 0;
    }
}

void DualRingBLE::setNamespace(const char* nspace) {
    preferences.setNamespace(nspace);
}

bool DualRingBLE::isConnected() {
    if (driveRing.waitingFor || 
        driveRing.connectTo ||
        domeRing.waitingFor ||
        domeRing.connectTo) {
        return false;
    } else {
        return true;
    }
}

bool DualRingBLE::hasFault() {
    return false;
}

void DualRingBLE::printState() {
    DBG_print("Drive: ");
    driveRing.printState();
    DBG_print("Dome:  ");
    domeRing.printState();
}