#include "connection-manager.h"
#include "communication-manager.h"
#include "credentials.h"
#include "intercom-device.h"

int checkRing = 2;
int checkTalk = 3;
int checkOpen = 4;

int doRing = 7;
int doTalk = 8;
int doOpen = 9;

static IntercomDevice *intercom;
static ConnectionManager *connectionManager;
static CommunicationManager *communicationManager;

std::map<int, IntercomStatus> commandCodeToIntercomStatus;
std::map<IntercomStatus, int> intercomStatusToEventCode;

void setup() {
    Serial.begin(115200);

    // while (!Serial) {
    //     // wait for serial
    // }

    intercom = new IntercomDevice(checkRing, checkTalk, checkOpen, doRing, doTalk, doOpen);
    connectionManager = new ConnectionManager();

    connectionManager->onConnected([](ServerDetails serverDetails) {
        Serial.println("Connected to WiFi");
        Serial.println(serverDetails.host);
        communicationManager = new CommunicationManager(
            serverDetails.host,
            serverDetails.port,
            serverDetails.useSSL,
            serverDetails.username,
            serverDetails.password
        );
        if (commandCodeToIntercomStatus.empty()) {
            std::map<int, IntercomStatus>::iterator it;
            for (it = codeToIntercomStatus.begin(); it != codeToIntercomStatus.end(); ++it) {
                commandCodeToIntercomStatus.insert(std::make_pair(it->first, it->second));
                intercomStatusToEventCode.insert(std::make_pair(it->second, it->first));
            }
        }

        communicationManager->onCommandReceived([](int command) {
                IntercomStatus status = commandCodeToIntercomStatus[MessageType::Command];
                Serial.print("Communication manager requested status change: ");
                Serial.println(intercomStatusNameMap[status]);
                intercom->changeStatus(status);
            });

        communicationManager->onConfigReceived([](int config[], int configLength) {
            intercom->updateConfig(config, configLength);
        });

        intercom->onStatusChange([](IntercomStatus status) {
            Serial.print("Intercom reported status change: ");
            Serial.println(intercomStatusNameMap[status]);
            if (communicationManager != NULL) {
                communicationManager->sendEvent(intercomStatusToEventCode[status]);
            }
        });

        intercom->onConfigUpdated([](int config[], int configLenght) {
            Serial.println("Intercom reported config update:");
            if (communicationManager != NULL) {
                communicationManager->sendConfig(config, configLenght);
            }
        });

        communicationManager->onConnected([](){
            intercom->getCurrentConfig([](int config[], int configLenght){
                if (communicationManager != NULL) {
                    communicationManager->sendConfig(config, configLenght);
                }
            });
        });
    });

    connectionManager->onDisconnected([]() {
        delete communicationManager;
    });
}

void loop() {
    connectionManager->loop();
    intercom->loop();
    if (communicationManager != NULL) {
        communicationManager->loop();
    }
}