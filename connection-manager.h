#include "config.h"

WiFiManager_NINA_Lite *wiFiManager;

struct ServerDetails {
    char *host;
    int port;
    boolean useSSL;
    char *username;
    char *password;
};

class ConnectionManager {
   private:
    boolean connected = false;
    boolean mInit = true;
    ServerDetails mServerDetails;
    unsigned long int lastCheck = 0;
    void (*mConnectedCallback)(ServerDetails);
    void (*mDisconnectedCallback)();
    void checkConnection();

   public:
    ConnectionManager();
    void onConnected(void (*connectedCallback)(ServerDetails));
    void onDisconnected(void (*disconnectedCallback)());
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
    if (time - lastCheck < 30000 && connected) {
        return;
    }
    lastCheck = time;
    int status = WiFi.status();
    if (status == WL_CONNECTED && !connected) {
        connected = true;
        if (mInit) {
            mInit = false;
            for (int i = 0; i < NUM_MENU_ITEMS; i++) {
                char *entry = myMenuItems[i].id;
                if (strcmp(entry, "wsh") == 0) {
                    mServerDetails.host = myMenuItems[i].pdata;
                } else if (strcmp(entry, "wsp") == 0) {
                    mServerDetails.port = atoi(myMenuItems[i].pdata);
                } else if (strcmp(entry, "wsssl") == 0) {
                    mServerDetails.useSSL = (strcmp(myMenuItems[i].pdata, "true") == 0);
                } else if (strcmp(entry, "wsusr") == 0) {
                    mServerDetails.username = myMenuItems[i].pdata;
                } else if (strcmp(entry, "wspas") == 0) {
                    mServerDetails.password = myMenuItems[i].pdata;
                }
            }
        }
        if (mConnectedCallback != NULL) {
            mConnectedCallback(mServerDetails);
        }
    } else if (status != WL_CONNECTED && connected) {
        connected = false;
        if (mDisconnectedCallback != NULL) {
            mDisconnectedCallback();
        }
    }
}

void ConnectionManager::onConnected(void (*connectedCallback)(ServerDetails)) {
    mConnectedCallback = connectedCallback;
}

void ConnectionManager::onDisconnected(void (*disconnectedCallback)()) {
    mDisconnectedCallback = disconnectedCallback;
}

void ConnectionManager::loop() {
    wiFiManager->run();
    checkConnection();
}