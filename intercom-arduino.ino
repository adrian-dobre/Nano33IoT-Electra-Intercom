#include "credentials.h"
#include "communication-manager.h"
#include "intercom-device.h"
#include "wifi-manager.h"

int checkRing = 2;
int checkTalk = 3;
int checkOpen = 4;

int doRing = 7;
int doTalk = 8;
int doOpen = 9;

IntercomDevice intercom = IntercomDevice(checkRing, checkTalk, checkOpen, doRing, doTalk, doOpen);
CommunicationManager communicationManager = CommunicationManager(WEB_SOCKET_SERVER_HOST, WEB_SOCKET_SERVER_PORT, WEB_SOCKET_SERVER_USE_SSL);

std::map<int, IntercomStatus> commandCodeToIntercomStatus;
std::map<IntercomStatus, int> intercomStatusToEventCode;

void setup() {
    Serial.begin(115200);

    std::map<int, IntercomStatus>::iterator it;
    for (it = codeToIntercomStatus.begin(); it != codeToIntercomStatus.end(); ++it) {
        commandCodeToIntercomStatus.insert(std::make_pair(it->first, it->second));
        intercomStatusToEventCode.insert(std::make_pair(it->second, it->first));
    }

    while (!Serial) {
        // wait for serial
    }
    wiFiManagerSetup();

    intercom.onStatusChange([](IntercomStatus status) {
        Serial.print("Intercom reported status change: ");
        Serial.println(intercomStatusNameMap[status]);
        communicationManager.sendEvent(intercomStatusToEventCode[status]);
    });
    communicationManager.onCommandReceived([](int command) {
        IntercomStatus status = commandCodeToIntercomStatus[MessageType::Command];
        Serial.print("Communication manager requested status change: ");
        Serial.println(intercomStatusNameMap[status]);
        intercom.changeStatus(status);
    });
}

void loop() {
    intercom.loop();
    communicationManager.loop();
}