#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "mqtt_client.h"


/* =====================================================
   PINES
   ===================================================== */

#define LED_GPIO        GPIO_NUM_4
#define BUTTON_1       GPIO_NUM_0
#define BUTTON_2       GPIO_NUM_5


/* =====================================================
   WIFI
   ===================================================== */

#define WIFI_SSID       "GGgobierno"
#define WIFI_PASSWORD   "12345678"


/* =====================================================
   MQTT
   ===================================================== */

#define MQTT_BROKER     "mqtt://test.mosquitto.org"

#define TOPIC_CONTROL   "tarea7/control"
#define TOPIC_ESTADO    "tarea7/estado"
#define TOPIC_RESULTADO "tarea7/resultado"


/* =====================================================
   CONFIGURACIÓN
   ===================================================== */

#define TOTAL_PULSACIONES 10


static const char *TAG = "TAREA7";

static esp_mqtt_client_handle_t mqtt_client = NULL;


/* =====================================================
   MODOS
   ===================================================== */

typedef enum
{
    MODO_1 = 1,
    MODO_2 = 2

} modo_t;


static volatile modo_t modo_actual = MODO_1;


/* =====================================================
   ESTADOS
   ===================================================== */

typedef enum
{
    ESTADO_ESPERA = 0,
    ESTADO_MIDIENDO

} estado_t;


static volatile estado_t estado_actual =
    ESTADO_ESPERA;


/* =====================================================
   VARIABLES DE TIEMPO
   ===================================================== */

static int64_t tiempo_inicio = 0;

static int64_t tiempo_anterior = 0;

static int64_t tiempo_actual = 0;

static int64_t tiempo_ms = 0;


/* =====================================================
   VARIABLES MODO 2
   ===================================================== */

static int numero_pulsacion = 0;


/* =====================================================
   PROTOTIPOS
   ===================================================== */

static void cambiar_modo(modo_t nuevo_modo);

static void reset_prueba(void);

static void mqtt_publicar(
    const char *topic,
    const char *mensaje
);

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
   MQTT PUBLICAR
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
   CAMBIAR MODO
   ===================================================== */

static void cambiar_modo(modo_t nuevo_modo)
{
    modo_actual = nuevo_modo;

    reset_prueba();


    if (modo_actual == MODO_1)
    {
        ESP_LOGI(
            TAG,
            "================================"
        );

        ESP_LOGI(
            TAG,
            "MODO 1 ACTIVADO"
        );

        ESP_LOGI(
            TAG,
            "BOTON 1 -> BOTON 2"
        );

        ESP_LOGI(
            TAG,
            "================================"
        );


        mqtt_publicar(
            TOPIC_ESTADO,
            "MODO1"
        );
    }


    else if (modo_actual == MODO_2)
    {
        ESP_LOGI(
            TAG,
            "================================"
        );

        ESP_LOGI(
            TAG,
            "MODO 2 ACTIVADO"
        );

        ESP_LOGI(
            TAG,
            "10 PULSACIONES"
        );

        ESP_LOGI(
            TAG,
            "================================"
        );


        mqtt_publicar(
            TOPIC_ESTADO,
            "MODO2"
        );
    }
}


/* =====================================================
   RESET
   ===================================================== */

static void reset_prueba(void)
{
    estado_actual =
        ESTADO_ESPERA;


    numero_pulsacion = 0;


    tiempo_inicio = 0;

    tiempo_anterior = 0;

    tiempo_actual = 0;

    tiempo_ms = 0;


    gpio_set_level(
        LED_GPIO,
        0
    );


    ESP_LOGI(
        TAG,
        "Prueba reiniciada"
    );
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
            (1ULL << BUTTON_1) |
            (1ULL << BUTTON_2),

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
   ESPERAR SOLTAR BOTÓN
   ===================================================== */

static void esperar_soltar(
    gpio_num_t boton
)
{
    while (
        gpio_get_level(boton) == 0
    )
    {
        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}


/* =====================================================
   MODO 1
   ===================================================== */

static void ejecutar_modo_1(void)
{
    /*
     * Esperar botón 1
     */

    if (
        gpio_get_level(BUTTON_1) == 0
    )
    {
        /*
         * Anti-rebote
         */

        vTaskDelay(
            pdMS_TO_TICKS(50)
        );


        if (
            gpio_get_level(BUTTON_1) != 0
        )
        {
            return;
        }


        /*
         * Botón 1 presionado
         */

        tiempo_inicio =
            esp_timer_get_time();


        estado_actual =
            ESTADO_MIDIENDO;


        gpio_set_level(
            LED_GPIO,
            1
        );


        ESP_LOGI(
            TAG,
            "BOTON 1 PRESIONADO"
        );


        ESP_LOGI(
            TAG,
            "CRONOMETRO INICIADO"
        );


        mqtt_publicar(
            TOPIC_ESTADO,
            "MIDIENDO"
        );


        /*
         * Esperar que se suelte
         */

        esperar_soltar(
            BUTTON_1
        );
    }


    /*
     * Si ya comenzó la medición,
     * esperar botón 2.
     */

    if (
        estado_actual ==
        ESTADO_MIDIENDO
    )
    {
        if (
            gpio_get_level(BUTTON_2) == 0
        )
        {
            vTaskDelay(
                pdMS_TO_TICKS(50)
            );


            if (
                gpio_get_level(BUTTON_2) != 0
            )
            {
                return;
            }


            /*
             * Tiempo final
             */

            tiempo_actual =
                esp_timer_get_time();


            /*
             * Diferencia en microsegundos
             */

            tiempo_ms =
                (
                    tiempo_actual -
                    tiempo_inicio
                ) / 1000;


            gpio_set_level(
                LED_GPIO,
                0
            );


            ESP_LOGI(
                TAG,
                "BOTON 2 PRESIONADO"
            );


            ESP_LOGI(
                TAG,
                "TIEMPO: %lld ms",
                tiempo_ms
            );


            /*
             * Crear mensaje
             */

            char mensaje[64];


            snprintf(
                mensaje,
                sizeof(mensaje),
                "%lld",
                tiempo_ms
            );


            /*
             * Publicar resultado
             */

            mqtt_publicar(
                TOPIC_RESULTADO,
                mensaje
            );


            mqtt_publicar(
                TOPIC_ESTADO,
                "TERMINADO"
            );


            estado_actual =
                ESTADO_ESPERA;


            esperar_soltar(
                BUTTON_2
            );
        }
    }
}


/* =====================================================
   MODO 2
   ===================================================== */

static void ejecutar_modo_2(void)
{
    /*
     * Esperar pulsaciones del botón 2.
     */

    if (
        gpio_get_level(BUTTON_2) == 0
    )
    {
        /*
         * Anti-rebote
         */

        vTaskDelay(
            pdMS_TO_TICKS(50)
        );


        if (
            gpio_get_level(BUTTON_2) != 0
        )
        {
            return;
        }


        /*
         * Primera pulsación
         */

        if (
            numero_pulsacion == 0
        )
        {
            tiempo_anterior =
                esp_timer_get_time();


            numero_pulsacion = 1;


            ESP_LOGI(
                TAG,
                "PULSACION 1"
            );


            ESP_LOGI(
                TAG,
                "Cronometro iniciado"
            );


            mqtt_publicar(
                TOPIC_ESTADO,
                "PULSACION_1"
            );
        }


        /*
         * Pulsaciones 2 a 10
         */

        else
        {
            tiempo_actual =
                esp_timer_get_time();


            /*
             * Tiempo entre pulsaciones
             */

            tiempo_ms =
                (
                    tiempo_actual -
                    tiempo_anterior
                ) / 1000;


            numero_pulsacion++;


            ESP_LOGI(
                TAG,
                "PULSACION %d",
                numero_pulsacion
            );


            ESP_LOGI(
                TAG,
                "TIEMPO ENTRE PULSACIONES: %lld ms",
                tiempo_ms
            );


            /*
             * Mensaje MQTT
             */

            char mensaje[64];


            snprintf(
                mensaje,
                sizeof(mensaje),
                "Pulsacion %d: %lld ms",
                numero_pulsacion,
                tiempo_ms
            );


            mqtt_publicar(
                TOPIC_RESULTADO,
                mensaje
            );


            /*
             * La pulsación actual pasa
             * a ser el punto de referencia
             * para la siguiente.
             */

            tiempo_anterior =
                tiempo_actual;
        }


        /*
         * LED indica pulsación
         */

        gpio_set_level(
            LED_GPIO,
            1
        );


        vTaskDelay(
            pdMS_TO_TICKS(100)
        );


        gpio_set_level(
            LED_GPIO,
            0
        );


        /*
         * Esperar que se suelte
         */

        esperar_soltar(
            BUTTON_2
        );


        /*
         * Si llegó a 10 pulsaciones
         */

        if (
            numero_pulsacion >=
            TOTAL_PULSACIONES
        )
        {
            ESP_LOGI(
                TAG,
                "================================"
            );


            ESP_LOGI(
                TAG,
                "10 PULSACIONES COMPLETADAS"
            );


            ESP_LOGI(
                TAG,
                "================================"
            );


            mqtt_publicar(
                TOPIC_ESTADO,
                "10_PULSACIONES_COMPLETADAS"
            );


            estado_actual =
                ESTADO_ESPERA;


            numero_pulsacion = 0;
        }
    }
}


/* =====================================================
   TAREA PRINCIPAL
   ===================================================== */

static void reaction_task(
    void *pvParameters
)
{
    while (1)
    {
        if (
            modo_actual == MODO_1
        )
        {
            ejecutar_modo_1();
        }


        else if (
            modo_actual == MODO_2
        )
        {
            ejecutar_modo_2();
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

        /* ---------------------------------------------
           MQTT CONECTADO
           --------------------------------------------- */

        case MQTT_EVENT_CONNECTED:

            ESP_LOGI(
                TAG,
                "MQTT CONECTADO"
            );


            /*
             * Suscribirse al control
             */

            esp_mqtt_client_subscribe(
                event->client,
                TOPIC_CONTROL,
                1
            );


            mqtt_publicar(
                TOPIC_ESTADO,
                "CONECTADO"
            );

            break;


        /* ---------------------------------------------
           MQTT RECIBIDO
           --------------------------------------------- */

        case MQTT_EVENT_DATA:

            ESP_LOGI(
                TAG,
                "MENSAJE MQTT RECIBIDO"
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
             * MODO 1
             */

            if (
                event->data_len == 5 &&
                memcmp(
                    event->data,
                    "MODO1",
                    5
                ) == 0
            )
            {
                cambiar_modo(
                    MODO_1
                );
            }


            /*
             * MODO 2
             */

            else if (
                event->data_len == 5 &&
                memcmp(
                    event->data,
                    "MODO2",
                    5
                ) == 0
            )
            {
                cambiar_modo(
                    MODO_2
                );
            }


            /*
             * RESET
             */

            else if (
                event->data_len == 5 &&
                memcmp(
                    event->data,
                    "RESET",
                    5
                ) == 0
            )
            {
                reset_prueba();


                mqtt_publicar(
                    TOPIC_ESTADO,
                    "RESET"
                );


                ESP_LOGI(
                    TAG,
                    "RESET POR MQTT"
                );
            }


            else
            {
                ESP_LOGW(
                    TAG,
                    "Comando MQTT desconocido"
                );
            }

            break;


        /* ---------------------------------------------
           MQTT DESCONECTADO
           --------------------------------------------- */

        case MQTT_EVENT_DISCONNECTED:

            ESP_LOGW(
                TAG,
                "MQTT DESCONECTADO"
            );

            break;


        /* ---------------------------------------------
           ERROR
           --------------------------------------------- */

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
            "Conectando al WiFi..."
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
         * Crear MQTT
         */

        if (
            mqtt_client == NULL
        )
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


            if (
                mqtt_client == NULL
            )
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
   WIFI INIT
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
     * Callback WiFi
     */

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            wifi_event_handler,
            NULL
        )
    );


    /*
     * Callback IP
     */

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            wifi_event_handler,
            NULL
        )
    );


    /*
     * Configurar WiFi
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
     * Hardware
     */

    configurar_led();

    configurar_botones();


    /*
     * Estado inicial
     */

    reset_prueba();


    /*
     * WiFi
     */

    wifi_init();


    /*
     * Tarea de reacción
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
        "MODO ACTUAL: MODO 1"
    );


    ESP_LOGI(
        TAG,
        "================================"
    );
}