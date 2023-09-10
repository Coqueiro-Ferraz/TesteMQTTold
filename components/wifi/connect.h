#ifndef __CONNECT_H
    #define __CONNET_H
    #include "esp_err.h"

    void wifi_init(void);
    esp_err_t wifi_connect_sta(const char * ssid, const char * pwd, int timeout);
    void wifi_disconnect(void);

#endif