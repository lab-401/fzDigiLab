/*
    Pilote SK6805 FlipperZero
    Copyright (C) 2022-2023 Victor Nikitchuk (https://github.com/quen0n)

    Ce programme est un logiciel libre : vous pouvez le redistribuer et/ou le
   modifier selon les termes de la Licence Publique Générale GNU telle que
   publiée par la Free Software Foundation, soit la version 3 de la licence,
   soit (à votre choix) toute version ultérieure.

    Ce programme est distribué dans l'espoir qu'il sera utile,
    mais SANS AUCUNE GARANTIE ; sans même la garantie implicite de
    QUALITÉ MARCHANDE ou D'ADÉQUATION À UN USAGE PARTICULIER. Voir le
    Licence Publique Générale GNU pour plus de détails.

    Vous devriez avoir reçu une copie de la Licence Publique Générale GNU
    avec ce programme. Si ce n'est pas le cas, consultez
   <https://www.gnu.org/licenses/>.
*/

#include "sk6805.h"
#include <furi_hal.h>

#ifndef SK6805_LED_COUNT
#define SK6805_LED_COUNT 3 // Nombre de LEDs sur la carte de rétroéclairage
#endif

#ifndef SK6805_LED_PIN
#define SK6805_LED_PIN &led_pin // Port de connexion des LEDs
#endif
// #ifdef FURI_DEBUG
/*#define DEBUG_PIN &gpio_ext_pa7
#define //DEBUG_INIT() \
    furi_hal_gpio_init(DEBUG_PIN, GpioModeOutputPushPull, GpioPullNo,
GpioSpeedVeryHigh) #define //DEBUG_SET_HIGH() furi_hal_gpio_write(DEBUG_PIN,
true) #define //DEBUG_SET_LOW() furi_hal_gpio_write(DEBUG_PIN, false) #else
#define //DEBUG_INIT()
#define //DEBUG_SET_HIGH()
#define //DEBUG_SET_LOW()
#endif
*/

static const GpioPin led_pin = {.port = GPIOA, .pin = LL_GPIO_PIN_13};
static uint8_t led_buffer[SK6805_LED_COUNT][3];

void SK6805_init(void) {
    // DEBUG_INIT();
    furi_hal_gpio_write(SK6805_LED_PIN, false);
    furi_hal_gpio_init(SK6805_LED_PIN, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
}

void SK6805_off(void) {
    // furi_kernel_lock();
    FURI_CRITICAL_ENTER();
    uint32_t end;
    for(uint16_t lednumber = 0; lednumber < SK6805_LED_COUNT * 24; lednumber++) {
        furi_hal_gpio_write(SK6805_LED_PIN, true);
        end = DWT->CYCCNT + 11;
        while(DWT->CYCCNT < end) {
        }
        furi_hal_gpio_write(SK6805_LED_PIN, false);
        end = DWT->CYCCNT + 43;
        while(DWT->CYCCNT < end) {
        }
    }
    FURI_CRITICAL_EXIT();
    // furi_kernel_unlock();
}

uint8_t SK6805_get_led_count(void) {
    return (const uint8_t)SK6805_LED_COUNT;
}

void SK6805_set_led_color(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b) {
    furi_check(led_index < SK6805_LED_COUNT);

    led_buffer[led_index][0] = g;
    led_buffer[led_index][1] = r;
    led_buffer[led_index][2] = b;
}

void SK6805_update(void) {
    //  SK6805_init();
    FURI_CRITICAL_ENTER();
    uint32_t end;
    /* Envoi séquentiel des couleurs des LEDs */
    for(uint8_t lednumber = 0; lednumber < SK6805_LED_COUNT; lednumber++) {
        // Envoi séquentiel des couleurs de la LED
        for(uint8_t color = 0; color < 3; color++) {
            // Envoi séquentiel des bits de couleur
            uint8_t i = 0b10000000;
            while(i != 0) {
                if(led_buffer[lednumber][color] & (i)) {
                    furi_hal_gpio_write(SK6805_LED_PIN, true);
                    // DEBUG_SET_HIGH();
                    end = DWT->CYCCNT + 30;
                    // T1H 600 us (615 us)
                    while(DWT->CYCCNT < end) {
                    }
                    furi_hal_gpio_write(SK6805_LED_PIN, false);
                    // DEBUG_SET_LOW();
                    end = DWT->CYCCNT + 26;
                    // T1L  600 us (587 us)
                    while(DWT->CYCCNT < end) {
                    }
                } else {
                    furi_hal_gpio_write(SK6805_LED_PIN, true);
                    // DEBUG_SET_HIGH();
                    end = DWT->CYCCNT + 11;
                    // T0H 300 ns (312 ns)
                    while(DWT->CYCCNT < end) {
                    }
                    furi_hal_gpio_write(SK6805_LED_PIN, false);
                    // DEBUG_SET_LOW();
                    end = DWT->CYCCNT + 43;
                    // T0L 900 ns (890 ns)
                    while(DWT->CYCCNT < end) {
                    }
                }
                i >>= 1;
            }
        }
    }
    FURI_CRITICAL_EXIT();
}
