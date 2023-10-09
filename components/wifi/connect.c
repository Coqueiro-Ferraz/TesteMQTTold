#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"

#define TAG "WIFI"

esp_netif_t *wifi_netif;

static EventGroupHandle_t wifi_events;
static const int CONNECTED_GOT_IP = BIT0;
static const int DISCONNECTED = BIT1;

char *get_wifi_err(uint8_t errcode)
{
    switch (errcode)
    {
    case WIFI_REASON_UNSPECIFIED:
        return "WIFI_REASON_UNSPECIFIED";
    case WIFI_REASON_AUTH_EXPIRE:
        return "WIFI_REASON_AUTH_EXPIRE";
    case WIFI_REASON_AUTH_LEAVE:
        return "WIFI_REASON_AUTH_LEAVE";
    case WIFI_REASON_ASSOC_EXPIRE:
        return "WIFI_REASON_ASSOC_EXPIRE";
    case WIFI_REASON_ASSOC_TOOMANY:
        return "WIFI_REASON_ASSOC_TOOMANY";
    case WIFI_REASON_NOT_AUTHED:
        return "WIFI_REASON_NOT_AUTHED";
    case WIFI_REASON_NOT_ASSOCED:
        return "WIFI_REASON_NOT_ASSOCED";
    case WIFI_REASON_ASSOC_LEAVE:
        return "WIFI_REASON_ASSOC_LEAVE";
    case WIFI_REASON_ASSOC_NOT_AUTHED:
        return "WIFI_REASON_ASSOC_NOT_AUTHED";
    case WIFI_REASON_DISASSOC_PWRCAP_BAD:
        return "WIFI_REASON_DISASSOC_PWRCAP_BAD";
    case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
        return "WIFI_REASON_DISASSOC_SUPCHAN_BAD";
    case WIFI_REASON_BSS_TRANSITION_DISASSOC:
        return "WIFI_REASON_BSS_TRANSITION_DISASSOC";
    case WIFI_REASON_IE_INVALID:
        return "WIFI_REASON_IE_INVALID";
    case WIFI_REASON_MIC_FAILURE:
        return "WIFI_REASON_MIC_FAILURE";
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        return "WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT";
    case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
        return "WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT";
    case WIFI_REASON_IE_IN_4WAY_DIFFERS:
        return "WIFI_REASON_IE_IN_4WAY_DIFFERS";
    case WIFI_REASON_GROUP_CIPHER_INVALID:
        return "WIFI_REASON_GROUP_CIPHER_INVALID";
    case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
        return "WIFI_REASON_PAIRWISE_CIPHER_INVALID";
    case WIFI_REASON_AKMP_INVALID:
        return "WIFI_REASON_AKMP_INVALID";
    case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
        return "WIFI_REASON_UNSUPP_RSN_IE_VERSION";
    case WIFI_REASON_INVALID_RSN_IE_CAP:
        return "WIFI_REASON_INVALID_RSN_IE_CAP";
    case WIFI_REASON_802_1X_AUTH_FAILED:
        return "WIFI_REASON_802_1X_AUTH_FAILED";
    case WIFI_REASON_CIPHER_SUITE_REJECTED:
        return "WIFI_REASON_CIPHER_SUITE_REJECTED";
    case WIFI_REASON_INVALID_PMKID:
        return "WIFI_REASON_INVALID_PMKID";
    case WIFI_REASON_BEACON_TIMEOUT:
        return "WIFI_REASON_BEACON_TIMEOUT";
    case WIFI_REASON_NO_AP_FOUND:
        return "WIFI_REASON_NO_AP_FOUND";
    case WIFI_REASON_AUTH_FAIL:
        return "WIFI_REASON_AUTH_FAIL";
    case WIFI_REASON_ASSOC_FAIL:
        return "WIFI_REASON_ASSOC_FAIL";
    case WIFI_REASON_HANDSHAKE_TIMEOUT:
        return "WIFI_REASON_HANDSHAKE_TIMEOUT";
    case WIFI_REASON_CONNECTION_FAIL:
        return "WIFI_REASON_CONNECTION_FAIL";
    case WIFI_REASON_AP_TSF_RESET:
        return "WIFI_REASON_AP_TSF_RESET";
    case WIFI_REASON_ROAMING:
        return "WIFI_REASON_ROAMING";
    }
    return "WIFI_REASON_UNSPECIFIED";
}

void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "Conectando...");
        esp_wifi_connect();
        break;
    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "Conectado com sucesso");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
    {
        wifi_event_sta_disconnected_t *wifi_event_sta_disconnected = event_data;
        // wifi_err_reason_t
        char *err = get_wifi_err(wifi_event_sta_disconnected->reason);
        if (wifi_event_sta_disconnected->reason != WIFI_REASON_ASSOC_LEAVE)
        {
            ESP_LOGE(TAG, "Desconectado %s", err);
        }
        else
        {
            ESP_LOGI(TAG, "Desconectado");
        }

        xEventGroupSetBits(wifi_events, DISCONNECTED);
        break;
    }
    case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "IP Obitido");
        xEventGroupSetBits(wifi_events, CONNECTED_GOT_IP);
        break;
    default:
        break;
    }
}

void wifi_init(void)
{
    wifi_init_config_t wifiCfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_wifi_init(&wifiCfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_events = xEventGroupCreate();
}

esp_err_t wifi_connect_sta(const char *ssid, const char *pwd, int timeout)
{
    wifi_netif = esp_netif_create_default_wifi_sta();

    wifi_config_t wifiCfg;
    memset(&wifiCfg, 0, sizeof(wifi_config_t));
    strncpy((char *)wifiCfg.sta.ssid, ssid, sizeof(wifiCfg.sta.ssid) - 1);
    strncpy((char *)wifiCfg.sta.password, pwd, sizeof(wifiCfg.sta.password) - 1);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiCfg);
    esp_wifi_start();

    EventBits_t event_result = xEventGroupWaitBits(wifi_events, CONNECTED_GOT_IP || DISCONNECTED, pdTRUE, pdFALSE, pdMS_TO_TICKS(timeout));
    if (event_result)
    {
        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
}
void wifi_disconnect(void)
{
    esp_wifi_disconnect();
    esp_wifi_stop();
}