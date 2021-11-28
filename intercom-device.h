enum IntercomStatus {
    Ready = 1,
    Ring = 2,
    Talk = 3,
    Listen = 4,
    Open = 5
};

enum IntercomButton {
    TalkButton = 1,
    OpenButton = 2
};

enum IntercomButtonState {
    On = 1,
    Off = 2
};

std::map<int, IntercomStatus> codeToIntercomStatus;
std::map<IntercomStatus, int> intercomStatusToCode;
std::map<IntercomStatus, const char*> intercomStatusNameMap;
std::map<int, IntercomButton> codeToIntercomButton;
std::map<int, IntercomButtonState> codeToIntercomButtonState;

class IntercomDevice {
   private:
    int mCheckRing;
    int mCheckTalk;
    int mCheckOpen;
    int mDoTalk;
    int mDoOpen;
    bool mAutoOpen = false;
    int mAutoOpenDelay = 0;
    int mDelayForAutoActions = 100;
    bool mReportButtonStatus = false;
    unsigned long int mStatusChangedTime = 0;
    bool mHandledByAutoOpen = false;
    void performPreOpeningSequence();
    void (*mStatusChangeCallback)(IntercomStatus);
    std::map<IntercomButton, IntercomButtonState> mIntercomButtonStateMap;
    void (*mButtonStateChangeCallback)(IntercomButton, IntercomButtonState);
    void (*mConfigUpdatedCallback)(int config[], int configLength);
    void checkIntercomStatus();
    void handleAutoOpen();
    void checkButtonStatusChange(int talk, int open);

   public:
    IntercomDevice(
        int checkRingPin,
        int checkTalkPin,
        int checkOpen,
        int doTalkPin,
        int doOpenPin);
    void changeStatus(IntercomStatus status);
    void onStatusChange(void (*callback)(IntercomStatus));
    void onButtonStatusChange(void (*callback)(IntercomButton, IntercomButtonState));
    void updateConfig(int config[], int configLength);
    void onConfigUpdated(void (*callback)(int config[], int configLength));
    void getCurrentConfig(void (*callback)(int config[], int configLength));
    void loop();
    IntercomStatus lastIntercomStatus = IntercomStatus::Ready;
};

IntercomDevice::IntercomDevice(
    int checkRingPin,
    int checkTalkPin,
    int checkOpen,
    int doTalkPin,
    int doOpenPin) {
    mCheckRing = checkRingPin;
    mCheckTalk = checkTalkPin;
    mCheckOpen = checkOpen;
    mDoTalk = doTalkPin;
    mDoOpen = doOpenPin;

    pinMode(doTalkPin, OUTPUT);
    pinMode(doOpenPin, OUTPUT);

    // Talk & Open pins are HIGH in stand-by mode
    digitalWrite(doTalkPin, HIGH);
    digitalWrite(doOpenPin, HIGH);

    mIntercomButtonStateMap[IntercomButton::TalkButton] = IntercomButtonState::Off;
    mIntercomButtonStateMap[IntercomButton::OpenButton] = IntercomButtonState::Off;

    // Ring pin is LOW on stand-by mode
    pinMode(checkRingPin, INPUT_PULLDOWN);
    // Talk & Open pins are HIGH in stand-by mode
    pinMode(checkTalkPin, INPUT_PULLUP);
    pinMode(checkOpen, INPUT_PULLUP);

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

void IntercomDevice::performPreOpeningSequence() {
    changeStatus(IntercomStatus::Talk);
    // we need to manually set the status here, since this series of actions is blocking
    lastIntercomStatus = IntercomStatus::Talk;
    delay(mDelayForAutoActions);
    changeStatus(IntercomStatus::Listen);
    // we need to manually set the status here, since this series of actions is blocking
    lastIntercomStatus = IntercomStatus::Listen;
    delay(mDelayForAutoActions);
}

void IntercomDevice::changeStatus(IntercomStatus status) {
    switch (status) {
        case Talk:
            digitalWrite(mDoTalk, LOW);
            digitalWrite(mDoOpen, HIGH);
            break;
        case Open:
            // Intercom needs to go through talk -> listen sequence before opening
            if (lastIntercomStatus == IntercomStatus::Ring) {
                performPreOpeningSequence();
            }
            digitalWrite(mDoTalk, HIGH);
            digitalWrite(mDoOpen, LOW);
            // allow intercom device enough time to send the "OPEN" signal
            delay(mDelayForAutoActions);
            break;
        default:
            digitalWrite(mDoTalk, HIGH);
            digitalWrite(mDoOpen, HIGH);
            break;
    }
}

void IntercomDevice::updateConfig(int config[], int configLength) {
    if (configLength > 0) {
        mAutoOpen = (config[0] == 1);
    }
    if (configLength > 1) {
        mAutoOpenDelay = config[1];
    }
    if (configLength > 2) {
        mDelayForAutoActions = config[2];
    }
    if (configLength > 3) {
        mReportButtonStatus = (config[3] == 1);
    }
    if (mConfigUpdatedCallback != NULL) {
        getCurrentConfig(mConfigUpdatedCallback);
    }
}

void IntercomDevice::onConfigUpdated(void (*callback)(int config[], int configLength)) {
    mConfigUpdatedCallback = callback;
}

void IntercomDevice::getCurrentConfig(void (*callback)(int config[], int configLength)) {
    int currentConfig[4] = {mAutoOpen ? 1 : 0, mAutoOpenDelay, mDelayForAutoActions, mReportButtonStatus ? 1 : 0};
    callback(currentConfig, 4);
}

void IntercomDevice::onStatusChange(void (*callback)(IntercomStatus)) {
    mStatusChangeCallback = callback;
}

void IntercomDevice::onButtonStatusChange(void (*callback)(IntercomButton, IntercomButtonState)) {
    mButtonStateChangeCallback = callback;
}

void IntercomDevice::checkButtonStatusChange(int talk, int open) {
    if (!mReportButtonStatus) {
        return;
    }
    int buttonPinState[] = {talk, open};
    int buttonIndex = 0;
    std::map<IntercomButton, IntercomButtonState>::iterator it;
    for (it = mIntercomButtonStateMap.begin(); it != mIntercomButtonStateMap.end(); ++it) {
        int button = buttonPinState[buttonIndex];
        if (buttonIndex > 1) {
            return;
        }
        IntercomButtonState buttonState = (button == LOW) ? IntercomButtonState::On : IntercomButtonState::Off;
        if (it->second != buttonState) {
            it->second = buttonState;
            if (mButtonStateChangeCallback != NULL) {
                mButtonStateChangeCallback(it->first, it->second);
            }
        }
        buttonIndex++;
    }
}

void IntercomDevice::checkIntercomStatus() {
    IntercomStatus intercomStatus = lastIntercomStatus;

    int ring = digitalRead(mCheckRing);
    int open = digitalRead(mCheckOpen);
    int talk = digitalRead(mCheckTalk);

    if (lastIntercomStatus == IntercomStatus::Ready && ring == HIGH) {
        intercomStatus = IntercomStatus::Ring;
        mHandledByAutoOpen = false;

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

    if (
        lastIntercomStatus != IntercomStatus::Ready &&
        ring == LOW &&
        (open == LOW || talk == LOW)) {
        changeStatus(IntercomStatus::Ready);
    }

    if (intercomStatus != lastIntercomStatus) {
        lastIntercomStatus = intercomStatus;
        mStatusChangedTime = millis();

        if (mStatusChangeCallback != NULL) {
            mStatusChangeCallback(intercomStatus);
        }
    }

    checkButtonStatusChange(talk, open);
}

void IntercomDevice::handleAutoOpen() {
    unsigned long int time = millis();
    if (
        mAutoOpen &&
        lastIntercomStatus == IntercomStatus::Ring &&
        !mHandledByAutoOpen) {
        if (time < mStatusChangedTime) {
            mStatusChangedTime = time;
        }
        if (time - mStatusChangedTime > (mAutoOpenDelay * 1000)) {
            changeStatus(IntercomStatus::Open);
            mHandledByAutoOpen = true;
        }
    }
}

void IntercomDevice::loop() {
    checkIntercomStatus();
    handleAutoOpen();
}