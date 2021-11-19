enum IntercomStatus {
    Ready = 1,
    Ring = 2,
    Talk = 3,
    Listen = 4,
    Open = 5
};

std::map<int, IntercomStatus> codeToIntercomStatus;
std::map<IntercomStatus, int> intercomStatusToCode;
std::map<IntercomStatus, const char*> intercomStatusNameMap;

IntercomStatus lastIntercomStatus = IntercomStatus::Ready;

class IntercomDevice {
   private:
    int mCheckRing;
    int mCheckTalk;
    int mCheckOpen;
    int mDoRing;
    int mDoTalk;
    int mDoOpen;
    bool mAutoOpen = false;
    int mAutoOpenDelay = 0;
    unsigned long int mStatusChangedTime = 0;
    void (*mStatusChangeCallback)(IntercomStatus);
    void (*mConfigUpdatedCallback)(int config[], int configLength);
    void checkIntercomStatus();
    void handleAutoOpen();

   public:
    IntercomDevice(
        int checkRing,
        int checkTalk,
        int checkOpen,
        int doRing,
        int doTalk,
        int doOpen);

    void changeStatus(IntercomStatus status);
    void onStatusChange(void (*callback)(IntercomStatus));
    void updateConfig(int config[], int configLength);
    void onConfigUpdated(void (*callback)(int config[], int configLength));
    void getCurrentConfig(void (*callback)(int config[], int configLength));
    void loop();
};

IntercomDevice::IntercomDevice(
    int checkRing,
    int checkTalk,
    int checkOpen,
    int doRing,
    int doTalk,
    int doOpen) {
    mCheckRing = checkRing;
    mCheckTalk = checkTalk;
    mCheckOpen = checkOpen;
    mDoRing = doRing;
    mDoTalk = doTalk;
    mDoOpen = doOpen;

    pinMode(checkRing, INPUT_PULLDOWN);
    pinMode(checkTalk, INPUT_PULLUP);
    pinMode(checkOpen, INPUT_PULLUP);

    pinMode(doRing, OUTPUT);
    pinMode(doTalk, OUTPUT);
    pinMode(doOpen, OUTPUT);

    digitalWrite(doRing, LOW);
    digitalWrite(doTalk, HIGH);
    digitalWrite(doOpen, HIGH);

    intercomStatusNameMap.insert(std::make_pair(IntercomStatus::Ready, "Ready"));
    intercomStatusNameMap.insert(std::make_pair(IntercomStatus::Ring, "Ring"));
    intercomStatusNameMap.insert(std::make_pair(IntercomStatus::Talk, "Talk"));
    intercomStatusNameMap.insert(std::make_pair(IntercomStatus::Listen, "Listen"));
    intercomStatusNameMap.insert(std::make_pair(IntercomStatus::Open, "Open"));

    codeToIntercomStatus.insert(std::make_pair(1, IntercomStatus::Ready));
    codeToIntercomStatus.insert(std::make_pair(2, IntercomStatus::Ring));
    codeToIntercomStatus.insert(std::make_pair(3, IntercomStatus::Talk));
    codeToIntercomStatus.insert(std::make_pair(4, IntercomStatus::Listen));
    codeToIntercomStatus.insert(std::make_pair(5, IntercomStatus::Open));

    std::map<int, IntercomStatus>::iterator it;
    for (it = codeToIntercomStatus.begin(); it != codeToIntercomStatus.end(); ++it) {
        intercomStatusToCode.insert(std::make_pair(it->second, it->first));
    }
}

void IntercomDevice::changeStatus(IntercomStatus status) {
    switch (status) {
        case Ring:
            digitalWrite(mDoRing, HIGH);
            digitalWrite(mDoTalk, HIGH);
            digitalWrite(mDoOpen, HIGH);
            break;
        case Talk:
            digitalWrite(mDoRing, HIGH);
            digitalWrite(mDoTalk, LOW);
            digitalWrite(mDoOpen, HIGH);
            break;
        case Open:
            digitalWrite(mDoRing, HIGH);
            digitalWrite(mDoTalk, HIGH);
            digitalWrite(mDoOpen, LOW);
            break;
        case Listen:
            digitalWrite(mDoRing, HIGH);
            digitalWrite(mDoTalk, HIGH);
            digitalWrite(mDoOpen, HIGH);
            break;
        default:
            digitalWrite(mDoRing, LOW);
            digitalWrite(mDoTalk, HIGH);
            digitalWrite(mDoOpen, HIGH);
            break;
    }
}

void IntercomDevice::updateConfig(int config[], int configLength) {
    if (configLength < 2) {
        return;
    }
    mAutoOpen = (config[0] == 1);
    mAutoOpenDelay = config[1];
    if (mConfigUpdatedCallback != NULL) {
        int currentConfig[2] = {mAutoOpen ? 1 : 0, mAutoOpenDelay};
        mConfigUpdatedCallback(currentConfig, 2);
    }
}

void IntercomDevice::onConfigUpdated(void (*callback)(int config[], int configLength)) {
    mConfigUpdatedCallback = callback;
}

void IntercomDevice::getCurrentConfig(void (*callback)(int config[], int configLength)) {
    int currentConfig[2] = {mAutoOpen ? 1 : 0, mAutoOpenDelay};
    callback(currentConfig, 2);
}

void IntercomDevice::onStatusChange(void (*callback)(IntercomStatus)) {
    mStatusChangeCallback = callback;
}

void IntercomDevice::checkIntercomStatus() {
    IntercomStatus intercomStatus = lastIntercomStatus;

    int ring = digitalRead(mCheckRing);
    int open = digitalRead(mCheckOpen);
    int talk = digitalRead(mCheckTalk);

    // Serial.print("Ring: ");
    // Serial.println(ring);
    // Serial.print("Talk: ");
    // Serial.println(talk);
    // Serial.print("Open: ");
    // Serial.println(open);

    if (lastIntercomStatus == IntercomStatus::Ready && ring == HIGH) {
        intercomStatus = IntercomStatus::Ring;

    } else if ((lastIntercomStatus == IntercomStatus::Listen || lastIntercomStatus == IntercomStatus::Talk) &&
               open == LOW) {
        intercomStatus = IntercomStatus::Open;

    } else if ((lastIntercomStatus == IntercomStatus::Ring || lastIntercomStatus == IntercomStatus::Listen) &&
               talk == LOW) {
        intercomStatus = IntercomStatus::Talk;

    } else if (lastIntercomStatus == IntercomStatus::Talk && talk == HIGH) {
        intercomStatus = IntercomStatus::Listen;

    } else if (ring == LOW) {
        intercomStatus = IntercomStatus::Ready;
    }
    if (intercomStatus != lastIntercomStatus) {
        lastIntercomStatus = intercomStatus;
        mStatusChangedTime = millis();

        if (mStatusChangeCallback != NULL) {
            mStatusChangeCallback(intercomStatus);
        }
    }
}

void IntercomDevice::handleAutoOpen() {
    unsigned long int time = millis();
    if (
        mAutoOpen &&
        lastIntercomStatus == IntercomStatus::Ring) {
        if (time < mStatusChangedTime) {
            mStatusChangedTime = time;
        }
        if (time - mStatusChangedTime > (mAutoOpenDelay * 1000)) {
            changeStatus(Talk);
            delay(1000);
            changeStatus(Listen);
            delay(1000);
            changeStatus(Open);
        }
    }
}

void IntercomDevice::loop() {
    checkIntercomStatus();
    handleAutoOpen();
}