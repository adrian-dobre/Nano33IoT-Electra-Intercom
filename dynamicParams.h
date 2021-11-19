#ifndef dynamicParams_h
#define dynamicParams_h

#include "defines.h"

// USE_DYNAMIC_PARAMETERS defined in defined.h

/////////////// Start dynamic Credentials ///////////////

//Defined in <WiFiManager_NINA_Lite_SAMD.h>
/**************************************
  #define MAX_ID_LEN                5
  #define MAX_DISPLAY_NAME_LEN      16

  typedef struct
  {
  char id             [MAX_ID_LEN + 1];
  char displayName    [MAX_DISPLAY_NAME_LEN + 1];
  char *pdata;
  uint8_t maxlen;
  } MenuItem;
**************************************/

#if USE_DYNAMIC_PARAMETERS

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

#else

MenuItem myMenuItems[] = {};

uint16_t NUM_MENU_ITEMS = 0;

#endif  //USE_DYNAMIC_PARAMETERS

#endif  //dynamicParams_h