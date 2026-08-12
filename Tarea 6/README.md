# Tarea 6: Práctica con STM32 - GPIO e Interrupciones (NVIC)

**Asignatura:** Microcontroladores  
**Estudiante:** Francis Jesús  

---

## 📌 Resumen de la Práctica
En esta asignación se estudiaron los conceptos fundamentales para el desarrollo de firmware sobre microcontroladores de la familia **STM32**, utilizando las librerías de abstracción de hardware **STM32Cube HAL** y analizando la gestión de eventos e interrupciones mediante el controlador vectorial NVIC.

---

## 🛠️ Conceptos Clave y Aprendizaje

### 1. Manejo de Entradas y Salidas con STM32 HAL GPIO
* **Librería HAL (Hardware Abstraction Layer):** Permite interactuar con los periféricos del microcontrolador mediante funciones estandarizadas en lugar de manipular directamente los registros del procesador.
* **Configuración de Pines GPIO:**
  * **Modo Salida (Push-Pull):** Configuración empleada para enviar señales lógicas y accionar componentes externos como LEDs.
  * **Modo Entrada:** Configuración para la lectura de pulsadores o sensores digitales.
* **Funciones Clave Utilizadas:**
  * `HAL_GPIO_WritePin(GPIOx, GPIO_PIN_x, PinState)`: Establece un pin en estado ALTO (1) o BAJO (0).
  * `HAL_GPIO_TogglePin(GPIOx, GPIO_PIN_x)`: Invierte el estado lógico actual del pin.
  * `HAL_GPIO_ReadPin(GPIOx, GPIO_PIN_x)`: Lee el valor de entrada del pin seleccionado.

---

### 2. Gestión de Interrupciones Externas y NVIC
* **NVIC (Nested Vectored Interrupt Controller):** Es el módulo interno del núcleo ARM Cortex-M encargado de administrar las prioridades y la ejecución de las llamadas a interrupción.
* **Interrupciones Externas (EXTI):** Permiten responder de forma inmediata ante un cambio de estado en un pin (flancos de subida `Rising` o bajada `Falling`) sin necesidad de realizar muestreos continuos (*polling*) dentro del bucle principal (`while(1)`), optimizando el uso del procesador y reduciendo el consumo de energía.
* **Función Callback:**
  * Al activarse una interrupción externa, el sistema ejecuta la función de respuesta:
    `void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)`
  * Dentro de esta función se procesa de forma rápida el evento desencadenado por el botón o señal de entrada.

---

## 🔗 Referencias de Estudio
* **Video 1:** [HOLA MUNDO! con STM32 HAL GPIO | Wels Theory](https://www.youtube.com/watch?v=...)
* **Video 2:** [¿Qué es NVIC? - Interrupciones externas con STM32 HAL INT | Wels Theory](https://www.youtube.com/watch?v=...)