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
    void (*mStatusChangeCallback)(IntercomStatus);
    void checkIntercomStatus();

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
    pinMode(checkTalk, INPUT_PULLDOWN);
    pinMode(checkOpen, INPUT_PULLDOWN);

    pinMode(doRing, OUTPUT);
    pinMode(doTalk, OUTPUT);
    pinMode(doOpen, OUTPUT);

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
            digitalWrite(mDoTalk, LOW);
            digitalWrite(mDoOpen, LOW);
            break;
        case Talk:
            digitalWrite(mDoRing, HIGH);
            digitalWrite(mDoTalk, HIGH);
            digitalWrite(mDoOpen, LOW);
            break;
        case Open:
            digitalWrite(mDoRing, HIGH);
            digitalWrite(mDoTalk, LOW);
            digitalWrite(mDoOpen, HIGH);
            break;
        case Listen:
            digitalWrite(mDoRing, HIGH);
            digitalWrite(mDoTalk, LOW);
            digitalWrite(mDoOpen, LOW);
            break;
        default:
            digitalWrite(mDoRing, LOW);
            digitalWrite(mDoTalk, LOW);
            digitalWrite(mDoOpen, LOW);
            break;
    }
}

void IntercomDevice::onStatusChange(void (*callback)(IntercomStatus)) {
    mStatusChangeCallback = callback;
}

void IntercomDevice::checkIntercomStatus() {
    IntercomStatus intercomStatus = lastIntercomStatus;

    if (lastIntercomStatus == IntercomStatus::Ready && digitalRead(mCheckRing) == HIGH) {
        intercomStatus = IntercomStatus::Ring;

    } else if ((lastIntercomStatus == IntercomStatus::Listen || lastIntercomStatus == IntercomStatus::Talk) &&
               digitalRead(mCheckOpen) == HIGH) {
        intercomStatus = IntercomStatus::Open;

    } else if ((lastIntercomStatus == IntercomStatus::Ring || lastIntercomStatus == IntercomStatus::Listen) &&
               digitalRead(mCheckTalk) == HIGH) {
        intercomStatus = IntercomStatus::Talk;

    } else if (lastIntercomStatus == IntercomStatus::Talk && digitalRead(mCheckTalk) == LOW) {
        intercomStatus = IntercomStatus::Listen;

    } else if (digitalRead(mCheckRing) == LOW) {
        intercomStatus = IntercomStatus::Ready;
    }
    if (intercomStatus != lastIntercomStatus) {
        lastIntercomStatus = intercomStatus;
        mStatusChangeCallback(intercomStatus);
    }
}

void IntercomDevice::loop() {
    checkIntercomStatus();
}