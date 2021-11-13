#define WEBSOCKETS_NETWORK_TYPE NETWORK_WIFININA
#include <WebSocketsClient_Generic.h>
#include <stdlib.h>
#include <map>

enum MessageType {
    Command = 1,
    Event = 2,
    Status = 3,
};

enum Status {
    Connected = 1
};

std::map<int, MessageType> codeToMessageType;
std::map<MessageType, int> messageTypeToCode;

class CommunicationManager {
   private:
    char *mServer;
    int mPort;
    bool mUseSSL;
    bool mConnected;
    WebSocketsClient webSocket;
    void (*mCommandCallback)(int command);
    void handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length);
    void handleMessageType(char *message);
    void sendMessage(MessageType message, int data);

   public:
    CommunicationManager(char *server, int port, bool useSSL);
    void onCommandReceived(void (*callback)(int command));
    void sendEvent(int event);
    void loop();
};

CommunicationManager::CommunicationManager(char *server, int port, bool useSSL) {
    mServer = server;
    mPort = port;
    mUseSSL = useSSL;
    mConnected = false;
    if (mUseSSL) {
        webSocket.beginSSL(mServer, mPort);
    } else {
        webSocket.begin(mServer, mPort);
    }
    std::function<void(WStype_t type, uint8_t * payload, size_t length)> handleWebSocketEventCb = std::bind(
        &CommunicationManager::handleWebSocketEvent,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3);
    webSocket.onEvent(handleWebSocketEventCb);

    codeToMessageType.insert(std::make_pair(1, MessageType::Command));
    codeToMessageType.insert(std::make_pair(2, MessageType::Event));
    codeToMessageType.insert(std::make_pair(3, MessageType::Status));

    std::map<int, MessageType>::iterator it;
    for (it = codeToMessageType.begin(); it != codeToMessageType.end(); ++it) {
        messageTypeToCode.insert(std::make_pair(it->second, it->first));
    }
}

void CommunicationManager::handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            if (mConnected) {
                Serial.println("[WSc] Disconnected!");
                mConnected = false;
            }

            break;
        case WStype_CONNECTED: {
            mConnected = true;

            Serial.print("[WSc] Connected to url: ");
            Serial.println((char *)payload);

            // send message to server when Connected
            sendMessage(MessageType::Status, Status::Connected);
        } break;
        case WStype_TEXT:
            Serial.print("[WSc] get text: ");
            Serial.println((char *)payload);
            handleMessageType((char *)payload);
            break;

        default:
            break;
    }
}

void CommunicationManager::handleMessageType(char *message) {
    char *component;
    char *delim = ":";
    component = strtok(message, delim);
    int results[] = {};
    unsigned int resultsIndex = 0;
    while (component != NULL) {
        results[resultsIndex] = atoi(component);
        resultsIndex++;
        component = strtok(NULL, delim);
    }
    if (resultsIndex == 0) {
        return;
    }

    Serial.print("Received valid message ");
    Serial.println(message);

    switch (results[0]) {
        case MessageType::Command:
            mCommandCallback(results[1]);
            break;

        default:
            break;
    }
}

void CommunicationManager::onCommandReceived(void (*callback)(int command)) {
    mCommandCallback = callback;
}

void CommunicationManager::sendMessage(MessageType type, int data) {
    char messagePayload[10];
    std::sprintf(messagePayload, "%d:%d", messageTypeToCode[type], data);

    Serial.print("Sending payload ");
    Serial.println(messagePayload);

    webSocket.sendTXT(messagePayload);
}

void CommunicationManager::sendEvent(int event) {
    sendMessage(MessageType::Event, event);
}

void CommunicationManager::loop() {
    webSocket.loop();
}