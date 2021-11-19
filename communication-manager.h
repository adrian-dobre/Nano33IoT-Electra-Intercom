#define WEBSOCKETS_NETWORK_TYPE NETWORK_WIFININA
#include <WebSocketsClient_Generic.h>
#include <stdlib.h>

#include <map>

enum MessageType {
    Command = 1,
    Event = 2,
    Status = 3,
    Configuration = 4
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
    char *mUsername;
    char *mPassword;
    bool mConnected;
    int lastPing;
    WebSocketsClient webSocket;
    void (*mCommandCallback)(int command);
    void (*mConnectedCallback)();
    void (*mConfigCallback)(int config[], int configLength);
    void handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length);
    void handleMessageType(char *message);
    void sendMessage(MessageType type, int data[], int dataLength);
    void ping();

   public:
    CommunicationManager(char *server, int port, bool useSSL, char *username, char *password);
    void onCommandReceived(void (*callback)(int command));
    void onConnected(void (*callback)());
    void onConfigReceived(void (*callback)(int config[], int configLength));
    void sendEvent(int event);
    void sendConfig(int config[], int configLength);
    void loop();
};

CommunicationManager::CommunicationManager(char *server, int port, bool useSSL, char *username, char *password) {
    mServer = server;
    mPort = port;
    mUseSSL = useSSL;
    mUsername = username;
    mPassword = password;
    mConnected = false;

    if (mUseSSL) {
        webSocket.beginSSL(mServer, mPort);
    } else {
        webSocket.begin(mServer, mPort);
    }
    webSocket.setAuthorization(mUsername, mPassword);
    char extraHeaders[80];
    sprintf(extraHeaders, "device-id: %s", DEVICE_ID);
    webSocket.setExtraHeaders(extraHeaders);
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
    codeToMessageType.insert(std::make_pair(4, MessageType::Configuration));

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
            int messageData[] = {Status::Connected};
            sendMessage(MessageType::Status, messageData, 1);
            if (mConnectedCallback != NULL) {
                mConnectedCallback();
            }
        } break;
        case WStype_PONG: {
            Serial.print("[WSc] Got pong: ");
            Serial.println((char *)payload);
        } break;
        case WStype_TEXT: {
            Serial.print("[WSc] get text: ");
            Serial.println((char *)payload);
            handleMessageType((char *)payload);
        } break;

        default:
            break;
    }
}

void CommunicationManager::handleMessageType(char *message) {
    Serial.println("Handling message");
    Serial.println(message);
    char *component;
    char *delim = ":";
    component = strtok(message, delim);
    int data[10] = {};
    int messageType;
    unsigned int resultsIndex = 0;
    while (component != NULL && resultsIndex <= 10) {
        if (resultsIndex == 0) {
            messageType = atoi(component);
        } else {
            data[resultsIndex - 1] = atoi(component);
        }
        resultsIndex++;
        component = strtok(NULL, delim);
    }
    if (resultsIndex == 0) {
        return;
    }

    Serial.print("Received valid message ");
    Serial.println(message);

    switch (messageType) {
        case MessageType::Command:
            if (mCommandCallback != NULL) {
                mCommandCallback(data[0]);
            }
            break;
        case MessageType::Configuration:
            if (mConfigCallback != NULL) {
                mConfigCallback(data, resultsIndex - 1);
            }
            break;
        default:
            break;
    }
    delay(1000);
    Serial.println("Done message handling ");
}

void CommunicationManager::onCommandReceived(void (*callback)(int command)) {
    mCommandCallback = callback;
}

void CommunicationManager::onConnected(void (*callback)()) {
    mConnectedCallback = callback;
}

void CommunicationManager::onConfigReceived(void (*callback)(int config[], int configLength)) {
    mConfigCallback = callback;
}

void CommunicationManager::sendMessage(MessageType type, int data[], int dataLength) {
    char messagePayload[30];
    char *pos = messagePayload;

    pos += std::sprintf(messagePayload, "%d", messageTypeToCode[type]);
    for (int i = 0; i < dataLength; i++) {
        pos += std::sprintf(pos, ":%d", data[i]);
    }

    Serial.print("Sending payload ");
    Serial.println(messagePayload);

    webSocket.sendTXT(messagePayload);
}

void CommunicationManager::sendEvent(int event) {
    int eventData[] = {event};
    sendMessage(MessageType::Event, eventData, 1);
}

void CommunicationManager::sendConfig(int config[], int configLength) {
    sendMessage(MessageType::Configuration, config, configLength);
}

void CommunicationManager::ping() {
    unsigned long int time = millis();
    if (time < lastPing) {
        lastPing = time;
    }
    if (time - lastPing > 30000) {
        webSocket.sendPing();
        lastPing = time;
    }
}

void CommunicationManager::loop() {
    ping();
    webSocket.loop();
}