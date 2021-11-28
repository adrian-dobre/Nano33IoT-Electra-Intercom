#include "device-monitor.h"
#include "connection-manager.h"
#include "communication-manager.h"
#include "intercom-device.h"

int checkRingPin = 2;
int checkTalkPin = 3;
int checkOpen = 4;

int doTalkPin = 8;
int doOpenPin = 9;

static IntercomDevice *intercom;
static ConnectionManager *connectionManager;
static CommunicationManager *communicationManager;
static DeviceMonitor deviceMonitor;

std::map<int, IntercomStatus> commandCodeToIntercomStatus;
std::map<IntercomStatus, int> intercomStatusToEventCode;

void setup() {
    intercom = new IntercomDevice(checkRingPin, checkTalkPin, checkOpen, doTalkPin, doOpenPin);
    connectionManager = new ConnectionManager();

    connectionManager->onStatusCheck([](bool connected){
         deviceMonitor.onNetworkCheck(connected);
    });

    connectionManager->onConnected([](StoredData storedData) {
        communicationManager = new CommunicationManager(
            storedData.serverDetails.host,
            storedData.serverDetails.port,
            storedData.serverDetails.useSSL,
            storedData.serverDetails.username,
            storedData.serverDetails.password
        );
        if (commandCodeToIntercomStatus.empty()) {
            std::map<int, IntercomStatus>::iterator it;
            for (it = codeToIntercomStatus.begin(); it != codeToIntercomStatus.end(); ++it) {
                commandCodeToIntercomStatus.insert(std::make_pair(it->first, it->second));
                intercomStatusToEventCode.insert(std::make_pair(it->second, it->first));
            }
        }

        communicationManager->onCommandReceived([](int command) {
                IntercomStatus status = commandCodeToIntercomStatus[command];
                intercom->changeStatus(status);
            });

        communicationManager->onConfigReceived([](int config[], int configLength) {
            intercom->updateConfig(config, configLength);
        });

        intercom->onStatusChange([](IntercomStatus status) {
            if (communicationManager != NULL) {
                communicationManager->sendIntercomStatusEvent(intercomStatusToEventCode[status]);
            }
        });

        intercom->onButtonStatusChange([](IntercomButton button, IntercomButtonState state) {
            if (communicationManager != NULL) {
                communicationManager->sendIntercomButtonStatusEvent(button, state);
            }
        });

        intercom->onConfigUpdated([](int config[], int configLenght) {
            if (communicationManager != NULL) {
                communicationManager->sendConfig(config, configLenght);
            }
        });

        communicationManager->onConnected([](){
            // on (re)connected send current configuration
            intercom->getCurrentConfig([](int config[], int configLenght){
                if (communicationManager != NULL) {
                    communicationManager->sendConfig(config, configLenght);
                }
            });
            // also send last known event
            if (communicationManager != NULL) {
                communicationManager->sendIntercomStatusEvent(intercomStatusToEventCode[intercom->lastIntercomStatus]);
            }
        });

        communicationManager->onPong([](){
            deviceMonitor.onServerPong();
        });
    });

    connectionManager->onDisconnected([]() {
        delete communicationManager;
    });
    deviceMonitor.startMonitoring();
}

void loop() {
    deviceMonitor.loop();
    connectionManager->loop();
    intercom->loop();
    if (communicationManager != NULL) {
        communicationManager->loop();
    }
}