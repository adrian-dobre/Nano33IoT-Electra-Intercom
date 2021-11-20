#define DEVICE_ID "2d37d801-591e-477c-9daa-9686c317bffc"

// WiFi Board Config
/* Comment this out to disable prints and save space */
// #define DEBUG_WIFI_WEBSERVER_PORT Serial
// #define WIFININA_DEBUG_OUTPUT Serial

#define _WIFININA_LOGLEVEL_ 4
#define DRD_GENERIC_DEBUG true
#define WIFININA_USE_SAMD true
#define WIFI_USE_SAMD true
#define BOARD_TYPE "SAMD NANO_33_IOT"

// Start location in EEPROM to store config data. Default 0
// Config data Size currently is 128 bytes)
#define EEPROM_START 0
#define EEPROM_SIZE (2 * 1024)

#define USING_CUSTOMS_STYLE false
#define USING_CUSTOMS_HEAD_ELEMENT false
#define USING_CORS_FEATURE false

#define USE_WIFI_NINA true
#define SHIELD_TYPE "WiFiNINA using WiFiNINA_Generic Library"
#include "WiFiNINA_Pinout_Generic.h"

// Permit running CONFIG_TIMEOUT_RETRYTIMES_BEFORE_RESET times before reset hardware
// to permit user another chance to config. Only if Config Data is valid.
// If Config Data is invalid, this has no effect as Config Portal will persist
#define RESET_IF_CONFIG_TIMEOUT true
// Permitted range of user-defined RETRY_TIMES_RECONNECT_WIFI between 2-5 times
#define RETRY_TIMES_RECONNECT_WIFI 5
// Permitted range of user-defined CONFIG_TIMEOUT_RETRYTIMES_BEFORE_RESET between 2-100
#define CONFIG_TIMEOUT_RETRYTIMES_BEFORE_RESET 5
// Config Timeout 30s (default 60s). Applicable only if Config Data is Valid
#define CONFIG_TIMEOUT 30000L
// Permit input only one set of WiFi SSID/PWD. The other can be "NULL or "blank"
// Default is false (if not defined) => must input 2 sets of SSID/PWD
#define REQUIRE_ONE_SET_SSID_PW true
#define USE_DYNAMIC_PARAMETERS true

#define SCAN_WIFI_NETWORKS true
// To be able to manually input SSID, not from a scanned SSID lists
#define MANUAL_SSID_INPUT_ALLOWED true
// From 2-15
#define MAX_SSID_IN_LIST 8
#include <WiFiManager_NINA_Lite_SAMD.h>
#define HOST_NAME "Intercom"

#define TO_LOAD_DEFAULT_CONFIG_DATA false
bool LOAD_DEFAULT_CONFIG_DATA = false;
WiFiNINA_Configuration defaultConfig;


// Dynamic Params
#define MAX_WEB_SOCKET_SERVER_HOST_LEN 34
char Web_Socket_Server_Host[MAX_WEB_SOCKET_SERVER_HOST_LEN + 1] = "echo.websocket.org";

#define MAX_WEB_SOCKET_SERVER_PORT_LEN 6
char Web_Socket_Server_Port[MAX_WEB_SOCKET_SERVER_PORT_LEN + 1] = "80";

#define MAX_WEB_SOCKET_SERVER_USE_SSL_LEN 6
char Web_Socket_Server_Use_SSL[MAX_WEB_SOCKET_SERVER_USE_SSL_LEN + 1] = "false";

#define MAX_USERNAME_LEN 34
char Web_Socket_User[MAX_USERNAME_LEN + 1] = "user@example.com";

#define MAX_PASSWORD_LEN 34
char Web_Socket_Password[MAX_PASSWORD_LEN + 1] = "example-password";

MenuItem myMenuItems[] =
    {
        {"wsh", "Server Host", Web_Socket_Server_Host, MAX_WEB_SOCKET_SERVER_HOST_LEN},
        {"wsp", "Server Port", Web_Socket_Server_Port, MAX_WEB_SOCKET_SERVER_PORT_LEN},
        {"wsssl", "Server Use SSL", Web_Socket_Server_Use_SSL, MAX_WEB_SOCKET_SERVER_USE_SSL_LEN},
        {"wsusr", "Username", Web_Socket_User, MAX_USERNAME_LEN},
        {"wspas", "Password", Web_Socket_Password, MAX_PASSWORD_LEN},
};

uint16_t NUM_MENU_ITEMS = sizeof(myMenuItems) / sizeof(MenuItem);  //MenuItemSize;