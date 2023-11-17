#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "mqtt_client.h"
#include <esp_log.h>
#include "driver/gpio.h"
#include <math.h>
#define TAG "[MQTT]"
#define min(a, b) (a>b?b:a)
static bool mqttOk = false;
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;

void event_handler(void* arg, esp_event_base_t event_base,
                    int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
        } 
        else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGD("Ip", IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                        &event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        &event_handler, NULL, NULL);
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                        pdFALSE, pdFALSE, portMAX_DELAY);
}
//configura el led, agrega mascara de bits y configura las resistencias pull up y down 

void hardware_init(void){
  gpio_config_t gpioLed =
  {
   .pin_bit_mask = 1ULL << GPIO_NUM_2,
   .mode = GPIO_MODE_DEF_OUTPUT,
   .pull_up_en = GPIO_PULLUP_DISABLE,
   .pull_down_en = GPIO_PULLDOWN_DISABLE,
   .intr_type = GPIO_INTR_DISABLE
  };   
  ESP_ERROR_CHECK(gpio_config(&gpioLed));
ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_2, 0));
}

static esp_mqtt_client_handle_t actClient; //puntero a estructura de cliente MQTT


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    char topic[64];
    char data[64];
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, " Conectado al servidor ");
        actClient = client;
        msg_id = esp_mqtt_client_subscribe(client, "/TP/cmd", 1);
        ESP_LOGI(TAG, " Subscripto exitosamente - id del mensaje: %d ", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/TP/5/cmd", 1);
        ESP_LOGI(TAG, " Subscripto exitosamente - id del mensaje: %d ", msg_id);

        mqttOk = true;
break;
    case MQTT_EVENT_DISCONNECTED: 
        ESP_LOGI(TAG, " Desconectado del servidor ");
        mqttOk = false;
        break;

    case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, " Subscripto a nuevo topico - id de mensaje: %d", event->msg_id);
            break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, " Se ha desubscripto - id del mensaje: %d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, " Datos publicados - id del mensaje: %d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, " Datos recibidos ");
        strncpy(topic, event->topic, min(63, event->topic_len));
        strncpy(data, event->data, min(63, event->data_len));
         ESP_LOGI(TAG, " Topico: %s\r",topic);
          ESP_LOGI(TAG, " Dato recibido: %s \r: ",data);
        //printf("TOPIC =%s\r\n", topic);
        //printf("DATA =%s\r\n", data);

        if (strcmp(topic, "/TP/cmd") == 0) {
            // Mensaje recibido en /ej02/cmd
            if (strcmp(data, "getId: ") == 0) {
                // Publicar el valor de id en /ej02/id
                //ESP_LOGI(TAG, "Received message: %s", data);
        msg_id = esp_mqtt_client_publish(client, "/TP/id", "5", 0, 1, 0);
        ESP_LOGI(TAG, "SE PUBLICO EL MENSAJE: %d ", msg_id);
            }

        } else if (strcmp(topic,"/TP/5/cmd") == 0) {
            // Mensaje recibido en /ej02/{id}/cmd
           
                // Mensaje dirigido a este dispositivo específico
                if (strcmp(data, "LedOn") == 0) {
                     ESP_LOGI(TAG, "LED ON ");
                     ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_2, 1));
                } else if (strcmp(data, "LedOff") == 0) {
                     // Código para apagar el LED
                     ESP_LOGI(TAG, "LED OFF ");  
                    ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_2, 0));
                    
                }
            
        }
        break;
   case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, " HUBO UN ERROR - MQTT EVENT ERROR ");
    ESP_LOGE(TAG, "Tipo de error: %d", event->error_handle->error_type);
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
        ESP_LOGE(TAG, "Ultima cadena de error: %s", strerror(event->error_handle->esp_transport_sock_errno));
    }
    break;
    default:
        ESP_LOGI(TAG, "Ha ocurrido otro evento fuera de los slots:%d ", event->event_id);
        break;
    }
}
void taskPublish(void *param){
    char valorsin[128];
    float cont = 1;
    
    while(1){
        vTaskDelay(2500 / portTICK_PERIOD_MS);
        if(mqttOk){
            float senoidal=sin(cont);
            sprintf(valorsin, "%f", senoidal);
            strcat(valorsin,"/5");
            esp_mqtt_client_publish(actClient, "/TP/data/sensor", valorsin, 0, 2, 0);
            cont=cont+(0.2);
        }
    }
}


void app_main(void)
{
   
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://192.168.0.53",
        .credentials.username = "TP2INFO2",
        .credentials.authentication.password = "lucas1234",
    };
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      nvs_flash_init();
    }
    hardware_init();
    wifi_init_sta();

    
    
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    xTaskCreate(taskPublish, "Publish Task", configMINIMAL_STACK_SIZE + 4096, NULL, 1, NULL);

}