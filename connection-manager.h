#include "config.h"

WiFiManager_NINA_Lite *wiFiManager;

struct ServerDetails {
    char *host;
    int port;
    boolean useSSL;
    char *username;
    char *password;
};

struct StoredData {
    ServerDetails serverDetails;
};

class ConnectionManager {
   private:
    boolean connected = false;
    boolean mInit = true;
    StoredData mStoredData;
    unsigned long int lastCheck = 0;
    int wifiCheckInterval = 30000;
    void (*mConnectedCallback)(StoredData);
    void (*mStatusCheckCallback)(bool isConnected);
    void (*mDisconnectedCallback)();
    void checkConnection();

   public:
    ConnectionManager();
    void onConnected(void (*connectedCallback)(StoredData));
    void onDisconnected(void (*disconnectedCallback)());
    void onStatusCheck(void (*statusCheckCallback)(bool isConnected));
    void loop();
};

ConnectionManager::ConnectionManager() {
    wiFiManager = new WiFiManager_NINA_Lite();
    wiFiManager->setConfigPortalChannel(0);
    wiFiManager->setConfigPortal("Intercom", "intercom");
    wiFiManager->begin(HOST_NAME);
}

void ConnectionManager::checkConnection() {
    unsigned long int time = millis();
    if (time < lastCheck) {
        lastCheck = time;
    }
    if (time - lastCheck < wifiCheckInterval && connected) {
        return;
    }
    lastCheck = time;
    int status = WiFi.status();
    if (mStatusCheckCallback != NULL) {
        mStatusCheckCallback(status == WL_CONNECTED);
    }
    if (status == WL_CONNECTED && !connected) {
        connected = true;
        if (mInit) {
            mInit = false;
            for (int i = 0; i < NUM_MENU_ITEMS; i++) {
                char *entry = myMenuItems[i].id;
                if (strcmp(entry, "wsh") == 0) {
                    mStoredData.serverDetails.host = myMenuItems[i].pdata;
                } else if (strcmp(entry, "wsp") == 0) {
                    mStoredData.serverDetails.port = atoi(myMenuItems[i].pdata);
                } else if (strcmp(entry, "wsssl") == 0) {
                    mStoredData.serverDetails.useSSL = (strcmp(myMenuItems[i].pdata, "true") == 0);
                } else if (strcmp(entry, "wsusr") == 0) {
                    mStoredData.serverDetails.username = myMenuItems[i].pdata;
                } else if (strcmp(entry, "wspas") == 0) {
                    mStoredData.serverDetails.password = myMenuItems[i].pdata;
                }
            }
        }
        if (mConnectedCallback != NULL) {
            mConnectedCallback(mStoredData);
        }
    } else if (status != WL_CONNECTED && connected) {
        connected = false;
        if (mDisconnectedCallback != NULL) {
            mDisconnectedCallback();
        }
    }
}

void ConnectionManager::onConnected(void (*connectedCallback)(StoredData)) {
    mConnectedCallback = connectedCallback;
}

void ConnectionManager::onDisconnected(void (*disconnectedCallback)()) {
    mDisconnectedCallback = disconnectedCallback;
}

void ConnectionManager::onStatusCheck(void (*statusCheckCallback)(bool isConnected)) {
    mStatusCheckCallback = statusCheckCallback;
}

void ConnectionManager::loop() {
    wiFiManager->run();
    checkConnection();
}