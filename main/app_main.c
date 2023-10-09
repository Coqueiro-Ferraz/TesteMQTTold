/* TESTEMQTT (em TCP) por HagaCeeF. Não me responsabilizo se não funcionar, não está completo,
    é apenas um código para fins didáticos, para ajudar os alunos a fazerem o TCC
    baseado no arquivo de exemplo do ESP-IDF para comunicar com o broker wegnology.
    Originalmente feito para versão 4 do IDF, tive muito trabalho para atualizar para a versão 5, podem haver
    problemas sobressalentes.
    Sugiro que você programe sua própria coexão com wifi pois esta é genérica e não tem tratamento de erros 

    Esse código foi desenvolvido no meu PC, por isso esta na pasta C:\coqueiro2\TesteMQTT, se você quiser que funcione
    no seu PC, faça o mesmo caminho ou faça a correção de pasta manualmente se não compilar.
*/
#include <inttypes.h> //<- não tem problema se ficar sublinhado de vermelho ou azul, se quiser resolver basta 
#include <stdio.h> //incluir o arquivo cpp_properties.json na pasta .vscode
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
#include "driver/ledc.h"
//#include "connect.h"
#include "ioplaca.h"
#include "lcdvia595.h"

/* 
<------------- Olhe a pasta Recursos/dashboard.png 
*/

#define W_DEVICE_ID "64ece4be7b49f24719198597" //Use o DeviceID no Wegnology  
#define W_ACCESS_KEY "c588fd20-5be4-4172-86f8-a0ee67cd8406" //use a chave de acesso e a senha
#define W_PASSWORD "fbdcde94ebafa343f4e0fc02911d2c098b8fe7b9f3aaf4f30c5cc78a906ef1f8"
#define W_TOPICO_PUBLICAR "wnology/64ece4be7b49f24719198597/state" //esse número no meio do tópico deve ser mudado pelo ID do seu device Wegnology
#define W_TOPICO_SUBSCREVER "wnology/64ece4be7b49f24719198597/command" // aqui também


static const char *TAG = "MQTT_EXAMPLE";
char *Inform;
int temp_val = 0;
int ledstatus = 0;
int entradas = 0;
const char *strLED = "LED\":"; //você poderá criar suas variáveis baseando-se nessas aqui
const char *string_temp = "{\"data\": {\"Temperatura\": ";
char mensa[40];
esp_mqtt_client_handle_t cliente; 

//leia com atenção!!!!!

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                _                    //
//                   _  .   _         _   _               _   ___   _         _   _    _               //
//              |   |_  |  |_|       |   | |  |\/|       |_|   |   |_  |\ |  |   |_|  | |              //    
//              |_  |_  |  | |       |_  |_|  |  |       | |   |   |_  | \|  |_  | |  |_|              //
//                                                                            /                        //
/////////////////////////////////////////////////////////////////////////////////////////////////////////


/*  A função a seguir usa o dado recebido, publicado pelo dashboard (variavel Inform)
    e compara com a string strLED (que no meu exemplo assume o valor "LED\":")
    isso significa que, se o que foi publicado no dashboard tem a sequencia de caracteres "LED":"
    vai executar o próximo bloco de código, onde a variavel Inform agora será comparada ao valor "true"
    pois no lado do dashboard eu poderia ter enviado "LED":"true" ou "LED":"false"
    quando o valor é true o led embarcado do ESP (pino 2) muda de estado, se estava apagado, acende, e vice-versa.

    Como o código que fiz é para mostrar a imagem do led no dashboard aceso, tenho que publicar no tópico state,
    por isso uso a macro W_TOPICO_PUBLICAR, a variavel que usei no meu dashboard se chama est_esp, e como a publicação 
    tem que ser formatada como json tem que ter essas chaves {}, a TAG data tem que ser usada para passar o valor
    e não testei se funcionaria sem ela. O link passado é o de onde armazenei a imagem dentro do Wegnology, 
    só estou dizendo para o dashboard carregá-la.

*/
void qualificador(esp_mqtt_client_handle_t client) //se for mexer aqui, mexa apenas no nome da função qualificador, usei esse nome pq foi o que eu pensei na hora 
{
    // você pode colocar seu código aqui, isso não vai afetar o funcionamento do programa, lembre-se de configurar o broker e o wifi corretamente
    // para configurar o wifi, use o menuconfig e vá até exampleconnection
    // o broker deverá ser configurado na função mqtt_app_start abaixo
    if(strstr(Inform, strLED)) // aqui estou perguntando se o que foi publicado no tópico command está relacionado a minha TAG LED
    {//caso afirmativo
        
        if(strstr(Inform, "true")) // aqui pergunto se a TAG LED recebeu o valor "true"
        {//se "LED":"true"
            ledstatus = !ledstatus;//inverte LED embarcado
            if(ledstatus==1)//se o LED embarcado está ligado
            {
                printf("LED ligado\n");
                // publico no tópico state {"data":{"est_esp":"link da imagem"}}
                esp_mqtt_client_publish(client, W_TOPICO_PUBLICAR, 
                "{\"data\": {\"est_esp\": \"https://files.wnology.io/64ece41c7188db1db16b6c74/imagens/esp%20led.png\" }}", 0, 0, 0);
            }
            else
            {
                printf("LED desligado\n");
                esp_mqtt_client_publish(client, W_TOPICO_PUBLICAR, 
                "{\"data\": {\"est_esp\": \"https://files.wnology.io/64ece41c7188db1db16b6c74/imagens/esp%20ligado.png\" }}", 0, 0, 0);
            }
        }
        else
        {//se "LED":"false"
            temp_val = rand() % 100;  // valor aleatório entre 0 e 100 para simular a temperatura
            sprintf(&mensa[0],"%s %d }}",string_temp,temp_val);//isso tudo aqui é só pra mostrar no console o que estou enviando pro broker
            ESP_LOGI(TAG, "%s", &mensa[0]);//vai aparecer de verde no console: MQTT_EXAMPLE: {"data": {Temperatura": 25 }}
            esp_mqtt_client_publish(client, W_TOPICO_PUBLICAR, &mensa[0], 0, 0, 0); // Aqui é onde está fazendo a publicação no broker efetivamente
        }
        
        gpio_set_level(TEC_SH_LD,ledstatus);//aqui é onde realmente acende ou apaga o LED embarcado
    
    }

}

//-------------------------------------------------------------------------------------------------------------------------
//não mexa! começando aqui
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:        
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, W_TOPICO_PUBLICAR, 
            &mensa[0], 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        ESP_LOGI(TAG, "%s", &mensa[0]);
       
        msg_id = esp_mqtt_client_subscribe(client, W_TOPICO_SUBSCREVER, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
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
        qualificador(client); //aqui está a mágica
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
        .broker.address.uri = "mqtt://broker.app.wnology.io",// se tu tiver outro broker pode colocar aqui, mas pro wnology é esse mesmo
        .credentials.set_null_client_id = false,  //coloquei e nem sei se precisa
        .credentials.client_id = W_DEVICE_ID,
        .credentials.username = W_ACCESS_KEY,
        .credentials.authentication.password = W_PASSWORD,
        
    };
    //isso aqui a baixo não vai precisar, só é usado se vc quiser passar o endereço do broker manualmente via teclado
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
        //ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    cliente = client;
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

//-----------------------------------------------------------------------------------------------------------------------
//finalizando aqui


void app_main(void)
{
    // a seguir, apenas informações de console, aquelas notas verdes no início da execução
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %lu bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init()); //se vc quiser saber prar que serve, basta copiar e colar no chatGPT ou semelhante e pedir que explique
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    //wifi_connect_sta("coqueiro", "amigos12", 4000); minha biblioteca wifi ainda não está pronta
    ioinit(); //inicializa nossa placa de desenvolvimento do SENAI
  
    temp_val = rand() % 100; //isso é só formatação para a minha aplicação, você não vai precisar disso
    sprintf(&mensa[0],"%s %d }}",string_temp,temp_val);//mas pode se basear nisso para fazer o seu
    mqtt_app_start();
    vTaskDelay(10000 / portTICK_PERIOD_MS);//espera a conexão e depois fica mandando o dado de temperatura periodicamente
    while (1) 
    {
        temp_val = rand() % 100;
        sprintf(&mensa[0],"%s %d }}",string_temp,temp_val);
        ESP_LOGI(TAG, "%s", &mensa[0]);
        esp_mqtt_client_publish(cliente, W_TOPICO_PUBLICAR, &mensa[0], 0, 0, 0);
        vTaskDelay(10000 / portTICK_PERIOD_MS); //eu estou enviando a temperatura a cada 10s
        //lembrando que o wegnology só aceita no máximo 2 payloads por segundo
    }
}
