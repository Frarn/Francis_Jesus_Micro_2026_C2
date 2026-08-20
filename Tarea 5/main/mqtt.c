#include <string.h>

#include "mqtt_client.h"
#include "esp_log.h"

static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t client = NULL;

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data)
{

    esp_mqtt_event_handle_t event = event_data;

    switch(event->event_id)
    {

        case MQTT_EVENT_CONNECTED:

            ESP_LOGI(TAG,"MQTT Connected");

            break;

        case MQTT_EVENT_DISCONNECTED:

            ESP_LOGI(TAG,"MQTT Disconnected");

            break;

        default:

            break;

    }

}

void mqtt_app_start(void)
{

    esp_mqtt_client_config_t mqtt_cfg = {

        .broker.address.uri = "mqtt://broker.hivemq.com"

    };

    client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(

        client,

        ESP_EVENT_ANY_ID,

        mqtt_event_handler,

        NULL

    );

    esp_mqtt_client_start(client);

}

void mqtt_publish(char *topic,char *msg)
{

    esp_mqtt_client_publish(

        client,

        topic,

        msg,

        0,

        1,

        0

    );

}