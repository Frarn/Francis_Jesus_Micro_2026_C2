#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "mqtt_client.h"


/* =====================================================
   PINES
   ===================================================== */

#define LED_GPIO        GPIO_NUM_4
#define BUTTON_START    GPIO_NUM_0
#define BUTTON_REACT    GPIO_NUM_5


/* =====================================================
   WIFI
   ===================================================== */

#define WIFI_SSID       "GGgobierno"
#define WIFI_PASSWORD   "12345678"


/* =====================================================
   MQTT
   ===================================================== */

#define MQTT_BROKER     "mqtt://test.mosquitto.org"

#define TOPIC_ESTADO    "tarea7/estado"
#define TOPIC_RESULTADO "tarea7/resultado"


/* =====================================================
   VARIABLES
   ===================================================== */

static const char *TAG = "TAREA7";

static esp_mqtt_client_handle_t mqtt_client = NULL;


/* =====================================================
   MÁQUINA DE ESTADOS
   ===================================================== */

typedef enum
{
    ESTADO_ESPERA = 0,
    ESTADO_ARMADO,
    ESTADO_ESPERANDO,
    ESTADO_REACCION,
    ESTADO_RESULTADO

} estado_t;


static estado_t estado_actual = ESTADO_ESPERA;


/* =====================================================
   TIEMPO
   ===================================================== */

static int64_t tiempo_inicio = 0;
static int64_t tiempo_final = 0;

static int64_t tiempo_reaccion_ms = 0;


/* =====================================================
   PROTOTIPOS
   ===================================================== */

static void cambiar_estado(estado_t nuevo_estado);

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


/* =====================================================
   PUBLICAR MQTT
   ===================================================== */

static void mqtt_publicar(
    const char *topic,
    const char *mensaje
)
{
    if (mqtt_client != NULL)
    {
        esp_mqtt_client_publish(
            mqtt_client,
            topic,
            mensaje,
            0,
            1,
            0
        );
    }
}


/* =====================================================
   CAMBIAR ESTADO
   ===================================================== */

static void cambiar_estado(estado_t nuevo_estado)
{
    estado_actual = nuevo_estado;

    switch (estado_actual)
    {
        case ESTADO_ESPERA:

            gpio_set_level(
                LED_GPIO,
                0
            );

            ESP_LOGI(
                TAG,
                "ESTADO: ESPERA"
            );

            mqtt_publicar(
                TOPIC_ESTADO,
                "ESPERA"
            );

            break;


        case ESTADO_ARMADO:

            gpio_set_level(
                LED_GPIO,
                0
            );

            ESP_LOGI(
                TAG,
                "ESTADO: ARMADO"
            );

            mqtt_publicar(
                TOPIC_ESTADO,
                "ARMADO"
            );

            break;


        case ESTADO_ESPERANDO:

            gpio_set_level(
                LED_GPIO,
                0
            );

            ESP_LOGI(
                TAG,
                "ESTADO: ESPERANDO"
            );

            mqtt_publicar(
                TOPIC_ESTADO,
                "ESPERANDO"
            );

            break;


        case ESTADO_REACCION:

            gpio_set_level(
                LED_GPIO,
                1
            );

            /*
             * COMIENZA LA MEDICIÓN
             */

            tiempo_inicio =
                esp_timer_get_time();

            ESP_LOGI(
                TAG,
                "SEÑAL!!! TIEMPO INICIADO"
            );

            mqtt_publicar(
                TOPIC_ESTADO,
                "GO"
            );

            break;


        case ESTADO_RESULTADO:

            gpio_set_level(
                LED_GPIO,
                0
            );

            ESP_LOGI(
                TAG,
                "RESULTADO: %lld ms",
                tiempo_reaccion_ms
            );

            break;
    }
}


/* =====================================================
   CONFIGURAR LED
   ===================================================== */

static void configurar_led(void)
{
    gpio_config_t config =
    {
        .pin_bit_mask =
            (1ULL << LED_GPIO),

        .mode =
            GPIO_MODE_OUTPUT,

        .pull_up_en =
            GPIO_PULLUP_DISABLE,

        .pull_down_en =
            GPIO_PULLDOWN_DISABLE,

        .intr_type =
            GPIO_INTR_DISABLE
    };


    ESP_ERROR_CHECK(
        gpio_config(&config)
    );


    gpio_set_level(
        LED_GPIO,
        0
    );
}


/* =====================================================
   CONFIGURAR BOTONES
   ===================================================== */

static void configurar_botones(void)
{
    gpio_config_t config =
    {
        .pin_bit_mask =
            (1ULL << BUTTON_START) |
            (1ULL << BUTTON_REACT),

        .mode =
            GPIO_MODE_INPUT,

        .pull_up_en =
            GPIO_PULLUP_ENABLE,

        .pull_down_en =
            GPIO_PULLDOWN_DISABLE,

        .intr_type =
            GPIO_INTR_DISABLE
    };


    ESP_ERROR_CHECK(
        gpio_config(&config)
    );
}


/* =====================================================
   GENERAR TIEMPO ALEATORIO
   ===================================================== */

static int obtener_tiempo_aleatorio(void)
{
    /*
     * Tiempo entre 2 y 5 segundos
     */

    uint32_t numero =
        esp_random();


    int tiempo =
        2000 + (numero % 3001);


    return tiempo;
}


/* =====================================================
   TAREA PRINCIPAL DEL JUEGO
   ===================================================== */

static void reaction_task(void *pvParameters)
{
    while (1)
    {

        /* =============================================
           ESPERAR BOTÓN 1
           ============================================= */

        if (
            estado_actual == ESTADO_ESPERA
        )
        {
            if (
                gpio_get_level(
                    BUTTON_START
                ) == 0
            )
            {
                /*
                 * Esperar que suelte
                 */

                while (
                    gpio_get_level(
                        BUTTON_START
                    ) == 0
                )
                {
                    vTaskDelay(
                        pdMS_TO_TICKS(10)
                    );
                }


                cambiar_estado(
                    ESTADO_ARMADO
                );


                vTaskDelay(
                    pdMS_TO_TICKS(500)
                );


                /*
                 * Pasar a esperar
                 */

                cambiar_estado(
                    ESTADO_ESPERANDO
                );


                /*
                 * Tiempo aleatorio
                 */

                int tiempo_espera =
                    obtener_tiempo_aleatorio();


                ESP_LOGI(
                    TAG,
                    "Esperando %d ms",
                    tiempo_espera
                );


                /*
                 * Esperar antes de dar
                 * la señal
                 */

                vTaskDelay(
                    pdMS_TO_TICKS(
                        tiempo_espera
                    )
                );


                /*
                 * Dar señal
                 */

                cambiar_estado(
                    ESTADO_REACCION
                );
            }
        }


        /* =============================================
           MEDIR REACCIÓN
           ============================================= */

        else if (
            estado_actual ==
            ESTADO_REACCION
        )
        {
            /*
             * Esperar segundo botón
             */

            if (
                gpio_get_level(
                    BUTTON_REACT
                ) == 0
            )
            {

                /*
                 * Tomar tiempo final
                 */

                tiempo_final =
                    esp_timer_get_time();


                /*
                 * Calcular diferencia
                 *
                 * esp_timer_get_time()
                 * devuelve microsegundos.
                 *
                 * /1000 = milisegundos.
                 */

                tiempo_reaccion_ms =
                    (
                        tiempo_final -
                        tiempo_inicio
                    ) / 1000;


                /*
                 * Mostrar resultado
                 */

                ESP_LOGI(
                    TAG,
                    "================================"
                );

                ESP_LOGI(
                    TAG,
                    "TIEMPO DE REACCION: %lld ms",
                    tiempo_reaccion_ms
                );

                ESP_LOGI(
                    TAG,
                    "================================"
                );


                /*
                 * Publicar resultado
                 */

                char mensaje[64];


                snprintf(
                    mensaje,
                    sizeof(mensaje),
                    "%lld ms",
                    tiempo_reaccion_ms
                );


                mqtt_publicar(
                    TOPIC_RESULTADO,
                    mensaje
                );


                /*
                 * Cambiar estado
                 */

                cambiar_estado(
                    ESTADO_RESULTADO
                );


                /*
                 * Esperar que suelte
                 * el botón
                 */

                while (
                    gpio_get_level(
                        BUTTON_REACT
                    ) == 0
                )
                {
                    vTaskDelay(
                        pdMS_TO_TICKS(10)
                    );
                }


                /*
                 * Esperar antes de
                 * comenzar otra prueba
                 */

                vTaskDelay(
                    pdMS_TO_TICKS(2000)
                );


                cambiar_estado(
                    ESTADO_ESPERA
                );
            }
        }


        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}


/* =====================================================
   CALLBACK MQTT
   ===================================================== */

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
)
{
    esp_mqtt_event_handle_t event =
        (esp_mqtt_event_handle_t)
        event_data;


    switch (
        (esp_mqtt_event_id_t)
        event_id
    )
    {

        case MQTT_EVENT_CONNECTED:

            ESP_LOGI(
                TAG,
                "MQTT CONECTADO"
            );


            /*
             * Suscribirse a control
             */

            esp_mqtt_client_subscribe(
                event->client,
                "tarea7/control",
                1
            );


            mqtt_publicar(
                TOPIC_ESTADO,
                "CONECTADO"
            );

            break;


        case MQTT_EVENT_DATA:

            ESP_LOGI(
                TAG,
                "Mensaje MQTT recibido"
            );


            printf(
                "TOPIC: %.*s\n",
                event->topic_len,
                event->topic
            );


            printf(
                "MENSAJE: %.*s\n",
                event->data_len,
                event->data
            );


            /*
             * RESET desde MQTT
             */

            if (
                event->data_len == 5 &&
                memcmp(
                    event->data,
                    "RESET",
                    5
                ) == 0
            )
            {
                cambiar_estado(
                    ESTADO_ESPERA
                );

                ESP_LOGI(
                    TAG,
                    "Juego reiniciado"
                );
            }

            break;


        case MQTT_EVENT_DISCONNECTED:

            ESP_LOGW(
                TAG,
                "MQTT DESCONECTADO"
            );

            break;


        case MQTT_EVENT_ERROR:

            ESP_LOGE(
                TAG,
                "ERROR MQTT"
            );

            break;


        default:

            break;
    }
}


/* =====================================================
   CALLBACK WIFI
   ===================================================== */

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
)
{

    /*
     * WiFi iniciado
     */

    if (
        event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START
    )
    {
        ESP_LOGI(
            TAG,
            "Conectando a WiFi..."
        );


        esp_wifi_connect();
    }


    /*
     * WiFi desconectado
     */

    else if (
        event_base == WIFI_EVENT &&
        event_id ==
            WIFI_EVENT_STA_DISCONNECTED
    )
    {
        ESP_LOGW(
            TAG,
            "WiFi desconectado"
        );


        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );


        esp_wifi_connect();
    }


    /*
     * IP obtenida
     */

    else if (
        event_base == IP_EVENT &&
        event_id ==
            IP_EVENT_STA_GOT_IP
    )
    {

        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)
            event_data;


        ESP_LOGI(
            TAG,
            "================================"
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
            "================================"
        );


        /*
         * Iniciar MQTT
         */

        if (mqtt_client == NULL)
        {

            esp_mqtt_client_config_t config =
            {
                .broker.address.uri =
                    MQTT_BROKER
            };


            mqtt_client =
                esp_mqtt_client_init(
                    &config
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
             * Registrar callback MQTT
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
   CONFIGURACIÓN WIFI
   ===================================================== */

static void wifi_init(void)
{

    ESP_ERROR_CHECK(
        esp_netif_init()
    );


    ESP_ERROR_CHECK(
        esp_event_loop_create_default()
    );


    esp_netif_create_default_wifi_sta();


    wifi_init_config_t cfg =
        WIFI_INIT_CONFIG_DEFAULT();


    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg)
    );


    /*
     * Registrar callback WiFi
     */

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            wifi_event_handler,
            NULL
        )
    );


    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            wifi_event_handler,
            NULL
        )
    );


    /*
     * Configuración de WiFi
     */

    wifi_config_t wifi_config =
    {
        .sta =
        {
            .ssid =
                WIFI_SSID,

            .password =
                WIFI_PASSWORD,

            .threshold.authmode =
                WIFI_AUTH_WPA2_PSK
        }
    };


    ESP_ERROR_CHECK(
        esp_wifi_set_mode(
            WIFI_MODE_STA
        )
    );


    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config
        )
    );


    ESP_ERROR_CHECK(
        esp_wifi_start()
    );


    ESP_LOGI(
        TAG,
        "WiFi iniciado"
    );
}


/* =====================================================
   APP MAIN
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
     * Configurar hardware
     */

    configurar_led();

    configurar_botones();


    /*
     * Estado inicial
     */

    cambiar_estado(
        ESTADO_ESPERA
    );


    /*
     * WiFi
     */

    wifi_init();


    /*
     * Crear tarea del juego
     */

    xTaskCreate(
        reaction_task,
        "reaction_task",
        4096,
        NULL,
        5,
        NULL
    );


    ESP_LOGI(
        TAG,
        "================================"
    );

    ESP_LOGI(
        TAG,
        "TAREA 7 INICIADA"
    );

    ESP_LOGI(
        TAG,
        "BOTON 1: GPIO %d",
        BUTTON_START
    );

    ESP_LOGI(
        TAG,
        "BOTON 2: GPIO %d",
        BUTTON_REACT
    );

    ESP_LOGI(
        TAG,
        "LED: GPIO %d",
        LED_GPIO
    );

    ESP_LOGI(
        TAG,
        "================================"
    );
}