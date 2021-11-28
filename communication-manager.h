#define WEBSOCKETS_NETWORK_TYPE NETWORK_WIFININA
#include <WebSocketsClient_Generic.h>
#include <stdlib.h>

#include <map>

enum MessageType {
    IntercomCommand = 1,
    IntercomStatusEvent = 2,
    DeviceStatus = 3,
    IntercomConfiguration = 4,
    IntercomButtonStatusEvent = 5
};

enum DeviceStatus {
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
    int mLastPing;
    int mPingInterval = 30000;
    WebSocketsClient webSocket;
    void (*mCommandCallback)(int command);
    void (*mPongCallback)();
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
    void onPong(void (*callback)());
    void onConfigReceived(void (*callback)(int config[], int configLength));
    void sendIntercomStatusEvent(int event);
    void sendIntercomButtonStatusEvent(int intercomButton, int intercomButtonState);
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

    codeToMessageType.insert(std::make_pair(1, MessageType::IntercomCommand));
    codeToMessageType.insert(std::make_pair(2, MessageType::IntercomStatusEvent));
    codeToMessageType.insert(std::make_pair(3, MessageType::DeviceStatus));
    codeToMessageType.insert(std::make_pair(4, MessageType::IntercomConfiguration));
    codeToMessageType.insert(std::make_pair(5, MessageType::IntercomButtonStatusEvent));

    std::map<int, MessageType>::iterator it;
    for (it = codeToMessageType.begin(); it != codeToMessageType.end(); ++it) {
        messageTypeToCode.insert(std::make_pair(it->second, it->first));
    }
}

void CommunicationManager::handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            if (mConnected) {
                mConnected = false;
            }

            break;
        case WStype_CONNECTED: {
            mConnected = true;
            // send message to server when Connected
            int messageData[] = {DeviceStatus::Connected};
            sendMessage(MessageType::DeviceStatus, messageData, 1);
            if (mConnectedCallback != NULL) {
                mConnectedCallback();
            }
        } break;
        case WStype_TEXT: {
            handleMessageType((char *)payload);
        } break;

        case WStype_PONG: {
            if (mPongCallback != NULL) {
                mPongCallback();
            }
        } break;

        default:
            break;
    }
}

void CommunicationManager::handleMessageType(char *message) {
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

    switch (messageType) {
        case MessageType::IntercomCommand:
            if (mCommandCallback != NULL) {
                mCommandCallback(data[0]);
            }
            break;
        case MessageType::IntercomConfiguration:
            if (mConfigCallback != NULL) {
                mConfigCallback(data, resultsIndex - 1);
            }
            break;
        default:
            break;
    }
}

void CommunicationManager::onCommandReceived(void (*callback)(int command)) {
    mCommandCallback = callback;
}

void CommunicationManager::onConnected(void (*callback)()) {
    mConnectedCallback = callback;
}

void CommunicationManager::onPong(void (*callback)()) {
    mPongCallback = callback;
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

    webSocket.sendTXT(messagePayload);
}

void CommunicationManager::sendIntercomStatusEvent(int event) {
    int eventData[] = {event};
    sendMessage(MessageType::IntercomStatusEvent, eventData, 1);
}

void CommunicationManager::sendIntercomButtonStatusEvent(int intercomButton, int intercomButtonState) {
    int eventData[] = {intercomButton, intercomButtonState};
    sendMessage(MessageType::IntercomButtonStatusEvent, eventData, 2);
}

void CommunicationManager::sendConfig(int config[], int configLength) {
    sendMessage(MessageType::IntercomConfiguration, config, configLength);
}

void CommunicationManager::ping() {
    unsigned long int time = millis();
    if (time < mLastPing) {
        mLastPing = time;
    }
    if (time - mLastPing > mPingInterval) {
        webSocket.sendPing();
        mLastPing = time;
    }
}

void CommunicationManager::loop() {
    ping();
    webSocket.loop();
}