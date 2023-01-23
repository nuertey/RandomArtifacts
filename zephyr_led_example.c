https://devzone.nordicsemi.com/f/nordic-q-a/87784/adding-to-the-device-tree-with-overlays---zephyr

========================================================================

GPIO Configuration Issue Zephyr RTOS+PlatformIO+STM32
Asked 5 months ago
Modified 5 months ago
Viewed 243 times
0

This is probably a dumb question but I need some help : I'm trying to simply blink an LED on my STM32L073 board. The LED is connected to the PB6 pin here's my code :

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

void main(void)
{
    const struct device *dev;
    dev = device_get_binding("GPIOB");
    gpio_pin_configure(dev,6,GPIO_OUTPUT);
    while(1){
        gpio_pin_set(dev,6,1);
        k_msleep(1000);
        gpio_pin_set(dev,6,0);
        k_msleep(1000);
    }
} 

I'm using platformio with Zephyr RTOS framework In prj.conf I do have : CONFIG_GPIO=y In zephyr.dts, for gpiob the label is well GPIOB so I don't understand what is wrong here, so if anyone can help me it would be great :)


========================================================================
https://github.com/zephyrproject-rtos/zephyr/discussions/35932


gabopushups
on Aug 16, 2021

This is some snippet from my dts, under the root node:

/ {
    model = "96Boards Gumstix AeroCore 2";
    compatible = "gumstix,aerocore2";

    chosen {
        zephyr,console = &usart1;
        zephyr,shell-uart = &usart1;
        zephyr,sram = &sram0;
        zephyr,flash = &flash0;
        zephyr,ccm = &ccm0;
    };
        mypins {
        compatible = "gpio-keys";
        dirA: button_A{
            gpios = <&gpioa 8 GPIO_ACTIVE_HIGH>;
            label = "PinDir 0";
        };
        dirB: button_B{
            gpios = <&gpioa 9 GPIO_ACTIVE_HIGH>;
            label = "PinDir 1";
    };
        aliases {
        ina = &dirA;
        inb = &dirB;
        };
}

As you can see, I created a node called "mypins", under which I used the binding "gpio-keys". In your dts. you'll need to change gpioa 8 for gpiob 8 and it should work for you, and use whatever aliases and names better suit your needs.

Then in the app code I call the alias ina (for input A) under the name MPOS_NODE (for motor positive node, but as I said, use whatever name you need)

#define MPOS_NODE   DT_ALIAS(inb) // 
#if DT_NODE_HAS_STATUS(MPOS_NODE, okay)
#define MPOS_GPIO_LABEL DT_GPIO_LABEL(MPOS_NODE, gpios)
#define MPOS_GPIO_PIN   DT_GPIO_PIN(MPOS_NODE, gpios)
#define MPOS_GPIO_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(MPOS_NODE, gpios))
#else
#error "Unsupported board: INA devicetree alias is not defined"
#define MPOS_GPIO_LABEL ""
#define MPOS_GPIO_PIN   0
#define MPOS_GPIO_FLAGS 0
#endif

And finally

      const struct device *motPos;
       motPos = device_get_binding(MPOS_GPIO_LABEL);
    if (motPos == NULL) {
        printk("Error: didn't find %s device\n", MPOS_GPIO_LABEL);
        return;
    }
        int ret;
    ret = gpio_pin_configure(motPos, MPOS_GPIO_PIN, GPIO_OUTPUT_ACTIVE | MPOS_GPIO_FLAGS);
    if (ret != 0) {
        printk("Error %d: failed to configure %s pin %d\n",
               ret, MPOS_GPIO_LABEL, MPOS_GPIO_PIN);
        return;
    }
