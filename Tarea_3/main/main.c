#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "mqtt_client.h"


/* =====================================================
   CONFIGURACIÓN DE PINES
   ===================================================== */

#define LED_GPIO        GPIO_NUM_4
#define BUTTON_GPIO     GPIO_NUM_0


/* =====================================================
   CONFIGURACIÓN WIFI
   ===================================================== */

#define WIFI_SSID       "GGgobierno"
#define WIFI_PASSWORD   "12345678"


/* =====================================================
   CONFIGURACIÓN MQTT
   ===================================================== */

#define MQTT_BROKER_URI     "mqtt://test.mosquitto.org"

#define MQTT_TOPIC_CONTROL  "tarea3/esp32/control"
#define MQTT_TOPIC_STATE    "tarea3/esp32/state"


static const char *TAG = "TAREA3";

static esp_mqtt_client_handle_t mqtt_client = NULL;


/* =====================================================
   MÁQUINA DE ESTADOS
   ===================================================== */

typedef enum
{
    ESTADO_OFF = 0,
    ESTADO_ON = 1

} estado_t;


static estado_t estado_actual = ESTADO_OFF;


/* =====================================================
   PROTOTIPOS
   ===================================================== */

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
);


static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
);


static void cambiar_estado(
    estado_t nuevo_estado
);


/* =====================================================
   CAMBIAR ESTADO
   ===================================================== */

static void cambiar_estado(estado_t nuevo_estado)
{
    estado_actual = nuevo_estado;


    if (estado_actual == ESTADO_ON)
    {
        gpio_set_level(LED_GPIO, 1);

        ESP_LOGI(TAG, "ESTADO: ON");
    }
    else
    {
        gpio_set_level(LED_GPIO, 0);

        ESP_LOGI(TAG, "ESTADO: OFF");
    }


    /* Publicar estado mediante MQTT */

    if (mqtt_client != NULL)
    {
        const char *mensaje;

        if (estado_actual == ESTADO_ON)
        {
            mensaje = "ON";
        }
        else
        {
            mensaje = "OFF";
        }


        esp_mqtt_client_publish(
            mqtt_client,
            MQTT_TOPIC_STATE,
            mensaje,
            0,
            1,
            0
        );
    }
}


/* =====================================================
   CONFIGURACIÓN DEL LED
   ===================================================== */

static void configurar_led(void)
{
    gpio_config_t led_config = {

        .pin_bit_mask = (1ULL << LED_GPIO),

        .mode = GPIO_MODE_OUTPUT,

        .pull_up_en = GPIO_PULLUP_DISABLE,

        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE
    };


    ESP_ERROR_CHECK(
        gpio_config(&led_config)
    );


    ESP_ERROR_CHECK(
        gpio_set_level(LED_GPIO, 0)
    );
}


/* =====================================================
   CONFIGURACIÓN DEL BOTÓN
   ===================================================== */

static void configurar_boton(void)
{
    gpio_config_t button_config = {

        .pin_bit_mask = (1ULL << BUTTON_GPIO),

        .mode = GPIO_MODE_INPUT,

        .pull_up_en = GPIO_PULLUP_ENABLE,

        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE
    };


    ESP_ERROR_CHECK(
        gpio_config(&button_config)
    );
}


/* =====================================================
   TAREA DEL BOTÓN
   ===================================================== */

static void button_task(void *pvParameters)
{
    int estado_anterior = 1;


    while (1)
    {
        int estado_boton =
            gpio_get_level(BUTTON_GPIO);


        /*
         * Con pull-up:
         *
         * Botón sin presionar = 1
         * Botón presionado    = 0
         */


        if (
            estado_boton == 0 &&
            estado_anterior == 1
        )
        {
            /*
             * Anti-rebote
             */

            vTaskDelay(
                pdMS_TO_TICKS(50)
            );


            if (
                gpio_get_level(BUTTON_GPIO) == 0
            )
            {
                /*
                 * Cambiar estado
                 */

                if (
                    estado_actual == ESTADO_OFF
                )
                {
                    cambiar_estado(
                        ESTADO_ON
                    );
                }
                else
                {
                    cambiar_estado(
                        ESTADO_OFF
                    );
                }


                /*
                 * Esperar a que se suelte
                 * el botón
                 */

                while (
                    gpio_get_level(BUTTON_GPIO) == 0
                )
                {
                    vTaskDelay(
                        pdMS_TO_TICKS(10)
                    );
                }
            }
        }


        estado_anterior =
            gpio_get_level(BUTTON_GPIO);


        vTaskDelay(
            pdMS_TO_TICKS(20)
        );
    }
}


/* =====================================================
   CALLBACK DE MQTT
   ===================================================== */

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
)
{
    esp_mqtt_event_handle_t event =
        (esp_mqtt_event_handle_t)event_data;


    switch (
        (esp_mqtt_event_id_t)event_id
    )
    {

        /* ---------------------------------------------
           MQTT CONECTADO
           --------------------------------------------- */

        case MQTT_EVENT_CONNECTED:

            ESP_LOGI(
                TAG,
                "MQTT conectado"
            );


            /*
             * Suscribirse al topic de control
             */

            esp_mqtt_client_subscribe(
                event->client,
                MQTT_TOPIC_CONTROL,
                1
            );


            /*
             * Publicar estado actual
             */

            if (
                estado_actual == ESTADO_ON
            )
            {
                esp_mqtt_client_publish(
                    event->client,
                    MQTT_TOPIC_STATE,
                    "ON",
                    0,
                    1,
                    0
                );
            }
            else
            {
                esp_mqtt_client_publish(
                    event->client,
                    MQTT_TOPIC_STATE,
                    "OFF",
                    0,
                    1,
                    0
                );
            }

            break;


        /* ---------------------------------------------
           MENSAJE MQTT RECIBIDO
           --------------------------------------------- */

        case MQTT_EVENT_DATA:

            ESP_LOGI(
                TAG,
                "Mensaje MQTT recibido"
            );


            /*
             * Mostrar topic recibido
             */

            printf(
                "TOPIC: %.*s\n",
                event->topic_len,
                event->topic
            );


            /*
             * Mostrar mensaje recibido
             */

            printf(
                "MENSAJE: %.*s\n",
                event->data_len,
                event->data
            );


            /*
             * Si recibe ON
             */

            if (
                event->data_len == 2 &&
                memcmp(
                    event->data,
                    "ON",
                    2
                ) == 0
            )
            {
                cambiar_estado(
                    ESTADO_ON
                );
            }


            /*
             * Si recibe OFF
             */

            else if (
                event->data_len == 3 &&
                memcmp(
                    event->data,
                    "OFF",
                    3
                ) == 0
            )
            {
                cambiar_estado(
                    ESTADO_OFF
                );
            }


            /*
             * Otro mensaje
             */

            else
            {
                ESP_LOGW(
                    TAG,
                    "Mensaje no reconocido"
                );
            }

            break;


        /* ---------------------------------------------
           MQTT DESCONECTADO
           --------------------------------------------- */

        case MQTT_EVENT_DISCONNECTED:

            ESP_LOGI(
                TAG,
                "MQTT desconectado"
            );

            break;


        /* ---------------------------------------------
           ERROR MQTT
           --------------------------------------------- */

        case MQTT_EVENT_ERROR:

            ESP_LOGE(
                TAG,
                "Error de MQTT"
            );

            break;


        default:

            break;
    }
}


/* =====================================================
   CALLBACK DE WIFI
   ===================================================== */

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
)
{

    /* ---------------------------------------------
       WIFI INICIADO
       --------------------------------------------- */

    if (
        event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START
    )
    {
        ESP_LOGI(
            TAG,
            "Conectando al WiFi..."
        );


        ESP_ERROR_CHECK(
            esp_wifi_connect()
        );
    }


    /* ---------------------------------------------
       WIFI DESCONECTADO
       --------------------------------------------- */

    else if (
        event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_DISCONNECTED
    )
    {
        ESP_LOGW(
            TAG,
            "WiFi desconectado. Reintentando..."
        );


        ESP_ERROR_CHECK(
            esp_wifi_connect()
        );
    }


    /* ---------------------------------------------
       WIFI CONECTADO / IP OBTENIDA
       --------------------------------------------- */

    else if (
        event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP
    )
    {
        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;


        ESP_LOGI(
            TAG,
            "================================="
        );


        ESP_LOGI(
            TAG,
            "WIFI CONECTADO"
        );


        ESP_LOGI(
            TAG,
            "IP: " IPSTR,
            IP2STR(
                &event->ip_info.ip
            )
        );


        ESP_LOGI(
            TAG,
            "================================="
        );


        /*
         * Iniciar MQTT solamente una vez
         */

        if (mqtt_client == NULL)
        {

            esp_mqtt_client_config_t mqtt_config =
            {
                .broker.address.uri =
                    MQTT_BROKER_URI
            };


            /*
             * Crear cliente MQTT
             */

            mqtt_client =
                esp_mqtt_client_init(
                    &mqtt_config
                );


            if (mqtt_client == NULL)
            {
                ESP_LOGE(
                    TAG,
                    "No se pudo crear MQTT"
                );

                return;
            }


            /*
             * Registrar CALLBACK MQTT
             */

            ESP_ERROR_CHECK(
                esp_mqtt_client_register_event(
                    mqtt_client,
                    ESP_EVENT_ANY_ID,
                    mqtt_event_handler,
                    NULL
                )
            );


            /*
             * Iniciar MQTT
             */

            ESP_ERROR_CHECK(
                esp_mqtt_client_start(
                    mqtt_client
                )
            );
        }
    }
}


/* =====================================================
   CONFIGURAR WIFI
   ===================================================== */

static void wifi_init(void)
{

    /*
     * Inicializar TCP/IP
     */

    ESP_ERROR_CHECK(
        esp_netif_init()
    );


    /*
     * Crear event loop
     */

    ESP_ERROR_CHECK(
        esp_event_loop_create_default()
    );


    /*
     * Crear interfaz WiFi
     */

    esp_netif_create_default_wifi_sta();


    /*
     * Configuración por defecto
     */

    wifi_init_config_t cfg =
        WIFI_INIT_CONFIG_DEFAULT();


    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg)
    );


    /*
     * Registrar CALLBACK de WiFi
     */

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL
        )
    );


    /*
     * Registrar evento de IP
     */

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL
        )
    );


    /*
     * Configuración de la red
     */

    wifi_config_t wifi_config =
    {
        .sta =
        {
            .ssid = WIFI_SSID,

            .password = WIFI_PASSWORD,

            .threshold.authmode =
                WIFI_AUTH_WPA2_PSK
        }
    };


    /*
     * Modo estación
     */

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(
            WIFI_MODE_STA
        )
    );


    /*
     * Aplicar configuración
     */

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config
        )
    );


    /*
     * Iniciar WiFi
     */

    ESP_ERROR_CHECK(
        esp_wifi_start()
    );


    ESP_LOGI(
        TAG,
        "WiFi iniciado"
    );


    ESP_LOGI(
        TAG,
        "SSID: %s",
        WIFI_SSID
    );
}


/* =====================================================
   PROGRAMA PRINCIPAL
   ===================================================== */

void app_main(void)
{

    /*
     * Inicializar NVS
     */

    esp_err_t ret =
        nvs_flash_init();


    if (
        ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND
    )
    {
        ESP_ERROR_CHECK(
            nvs_flash_erase()
        );


        ESP_ERROR_CHECK(
            nvs_flash_init()
        );
    }
    else
    {
        ESP_ERROR_CHECK(ret);
    }


    /*
     * Configurar LED
     */

    configurar_led();


    /*
     * Configurar botón
     */

    configurar_boton();


    /*
     * Estado inicial = OFF
     */

    cambiar_estado(
        ESTADO_OFF
    );


    /*
     * Iniciar WiFi
     */

    wifi_init();


    /*
     * Crear tarea del botón
     */

    xTaskCreate(
        button_task,
        "button_task",
        2048,
        NULL,
        5,
        NULL
    );


    ESP_LOGI(
        TAG,
        "================================="
    );


    ESP_LOGI(
        TAG,
        "SISTEMA INICIADO"
    );


    ESP_LOGI(
        TAG,
        "Estado inicial: OFF"
    );


    ESP_LOGI(
        TAG,
        "================================="
    );
}