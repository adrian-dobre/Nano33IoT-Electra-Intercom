#include <Adafruit_SleepyDog.h>

class DeviceMonitor {
   private:
    int watchdogMaxPeriod = 16000;
    int maxConnectionPeriod = 150000;
    unsigned long int mLastServerPong = 0;
    unsigned long int mLastNetworkConnection = 0;
    void checkNetworkConnectionStatus();
    void checkServerConnectionStatus();

   public:
    DeviceMonitor();
    void startMonitoring();
    void stopMonitoring();
    void resetDevice();
    void onNetworkCheck(bool isConnected);
    void onServerPong();
    void loop();
};

DeviceMonitor::DeviceMonitor() {
}

void DeviceMonitor::startMonitoring() {
    Watchdog.enable(watchdogMaxPeriod);
}

void DeviceMonitor::stopMonitoring() {
    Watchdog.disable();
}

void DeviceMonitor::onNetworkCheck(bool isConnected) {
    if (isConnected) {
        mLastNetworkConnection = millis();
    }
}

void DeviceMonitor::onServerPong() {
    mLastServerPong = millis();
}

void DeviceMonitor::checkNetworkConnectionStatus() {
    unsigned long int time = millis();
    if (time < mLastNetworkConnection) {
        mLastNetworkConnection = time;
    }
    if (time - mLastNetworkConnection > maxConnectionPeriod) {
        resetDevice();
    }
}

void DeviceMonitor::checkServerConnectionStatus() {
    unsigned long int time = millis();
    if (time < mLastServerPong) {
        mLastServerPong = time;
    }
    if (time - mLastServerPong > maxConnectionPeriod) {
        resetDevice();
    }
}

void DeviceMonitor::resetDevice() {
    Watchdog.disable();
    int time = Watchdog.enable(1);
    delay(time + 1000);
}

void DeviceMonitor::loop() {
    checkServerConnectionStatus();
    checkNetworkConnectionStatus();
    Watchdog.reset();
}
