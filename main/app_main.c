/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "driver/gpio.h"
#include "esp_adc_cal.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "connect.h"
#include "ioplaca.h"
#include "lcdvia595.h"

static const char *TAG = "MQTT_EXAMPLE";
char *Inform;
int temp_val = 0;
int ledstatus = 0;
int entradas = 0;
const char *strLED = "LED\":";
const char *string_temp = "{\"data\": {\"Temperatura\": ";
char mensa[40];
esp_mqtt_client_handle_t cliente; 


void qualificador(esp_mqtt_client_handle_t client)
{
    if(strstr(Inform, strLED))
    {
        
        if(strstr(Inform, "true"))
        {
            ledstatus = !ledstatus;
            if(ledstatus==1)
            {
                printf("LED ligado\n");
                esp_mqtt_client_publish(client, "wnology/64ece4be7b49f24719198597/state"/*"/SENAI_Coqueiro/qos1""/topic/qos1"*/, 
                "{\"data\": {\"est_esp\": \"https://files.wnology.io/64ece41c7188db1db16b6c74/imagens/esp%20led.png\" }}"/*"data_3"*/, 0, 0, 0);
            }
            else
            {
                printf("LED desligado\n");
                esp_mqtt_client_publish(client, "wnology/64ece4be7b49f24719198597/state"/*"/SENAI_Coqueiro/qos1""/topic/qos1"*/, 
                "{\"data\": {\"est_esp\": \"https://files.wnology.io/64ece41c7188db1db16b6c74/imagens/esp%20ligado.png\" }}"/*"data_3"*/, 0, 0, 0);
            }
        }
        else
        {
            //ledstatus = 0;
            temp_val = rand() % 100;
            sprintf(&mensa[0],"%s %d }}",string_temp,temp_val);
            ESP_LOGI(TAG, "%s", &mensa[0]);
           // printf("LED desligado\n");
           // esp_mqtt_client_publish(client, "wnology/64ece4be7b49f24719198597/state"/*"/SENAI_Coqueiro/qos1""/topic/qos1"*/, 
           // "{\"data\": {\"est_esp\": \"https://files.wnology.io/64ece41c7188db1db16b6c74/imagens/esp%20ligado.png\" }}"/*"data_3"*/, 0, 0, 0);
           esp_mqtt_client_publish(client, "wnology/64ece4be7b49f24719198597/state", &mensa[0], 0, 0, 0);
        }
        
       // vTaskDelay(1200 / portTICK_PERIOD_MS);
        gpio_set_level(TEC_SH_LD,ledstatus);
       // sprintf(string_temp, "{\"data\": {\"Temperatura\": %.d }}", (rand(X) % 3));
       // esp_mqtt_client_publish(client, "wnology/64ece4be7b49f24719198597/state"/*"/SENAI_Coqueiro/qos1""/topic/qos1"*/, 
       //     string_temp/*"data_3"*/, 0, 0, 0);
    }

}


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
       // sprintf(string_temp, "{\"data\": {\"Temperatura\": 35 }}"/*(rand() % 10)*/);
        
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "wnology/64ece4be7b49f24719198597/state"/*"/SENAI_Coqueiro/qos1""/topic/qos1"*/, 
            &mensa[0]/*"{\"data\": {\"Temperatura\": 35 }}"*//*"{\"data\": {\"Temperatura\": 65 }}""data_3"*/, 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        ESP_LOGI(TAG, "%s", &mensa[0]);
    //    msg_id = esp_mqtt_client_publish(client, "wnology/64ece4be7b49f24719198597/state"/*"/SENAI_Coqueiro/qos1""/topic/qos1"*/, 
    //        "{\"data\": {\"est_esp\": \"https://files.wnology.io/64ece41c7188db1db16b6c74/imagens/esp%20ligado.png\" }}"/*"data_3"*/, 0, 0, 0);
    //    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
       

       //https://files.wnology.io/64ece41c7188db1db16b6c74/imagens/esp%20ligado.png
        msg_id = esp_mqtt_client_subscribe(client, "wnology/64ece4be7b49f24719198597/command"/*"/SENAI_Coqueiro/qos0""/topic/qos0"*/, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

       // msg_id = esp_mqtt_client_subscribe(client, "/SENAI_Coqueiro/qos1"/*"/topic/qos1"*/, 1);
       // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

       // msg_id = esp_mqtt_client_unsubscribe(client, "/SENAI_Coqueiro/qos1"/*"/topic/qos1"*/);
       // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
      //  msg_id = esp_mqtt_client_publish(client, "/SENAI_Coqueiro/qos0"/*"/topic/qos0"*/, "data", 0, 0, 0);
      //  ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        Inform = event->data;
       // ledstatus = !ledstatus;
       // gpio_set_level(TEC_SH_LD,ledstatus);
        qualificador(client);


        /*        
        if(Inform[0] == 'A')
        {
            gpio_set_level(TEC_SH_LD,1);
            printf("%c \r\n",Inform[0]);
            entradas = io_le_escreve(1);
            sprintf(Inform,"Entradas: %x", entradas);
           // msg_id = esp_mqtt_client_publish(client, "/SENAI_Coqueiro/qos0", Inform, 0, 0, 0);
        }
        else if(Inform[0] == 'd')
        {
            gpio_set_level(TEC_SH_LD,0);
            printf("%c \r\n",Inform[0]);
            entradas = io_le_escreve(0);
            sprintf(Inform,"Entradas: %x", entradas);
           // msg_id = esp_mqtt_client_publish(client, "/SENAI_Coqueiro/qos0", Inform, 0, 0, 0);
        }*/
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
        .set_null_client_id = false,  //eu que acrescentei
        .client_id = "64ece4be7b49f24719198597",
        .username = "c588fd20-5be4-4172-86f8-a0ee67cd8406",
        .password = "fbdcde94ebafa343f4e0fc02911d2c098b8fe7b9f3aaf4f30c5cc78a906ef1f8",
        
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    cliente = client;
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
/*
static int count = 0;
static char line[128];
void initall()
{
    while (count < 128) {
        int c = fgetc(stdin);
        if (c == '\n') {
            line[count] = '\0';
            break;
        } else if (c > 0 && c < 127) {
            line[count] = c;
            ++count;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    ESP_LOGE("Init","Pressionado: %s",&line[0]);

}*/



void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    //initall();

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    //wifi_connect_sta("coqueiro", "amigos12", 4000);
    ioinit();
  
    temp_val = rand() % 10;
    sprintf(&mensa[0],"%s %d }}",string_temp,temp_val);
    mqtt_app_start();
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    while (1) 
    {
            temp_val = rand() % 100;
            sprintf(&mensa[0],"%s %d }}",string_temp,temp_val);
            ESP_LOGI(TAG, "%s", &mensa[0]);
           esp_mqtt_client_publish(cliente, "wnology/64ece4be7b49f24719198597/state", &mensa[0], 0, 0, 0);
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
