    int result = -1;
    
    const struct device *const green_led = DEVICE_DT_GET(DT_NODELABEL(green_led));
    const struct device *const blue_led = DEVICE_DT_GET(DT_NODELABEL(blue_led));
    const struct device *const red_led = DEVICE_DT_GET(DT_NODELABEL(red_led));
    
    if (!device_is_ready(green_led))
    {
        printk("Device %s not ready!\n", green_led->name);
        printf("Exiting application...\n");
        return;
    }
    
    if (!device_is_ready(blue_led))
    {
        printk("Device %s not ready!\n", blue_led->name);
        printf("Exiting application...\n");
        return;
    }
    
    if (!device_is_ready(red_led))
    {
        printk("Device %s not ready!\n", red_led->name);
        printf("Exiting application...\n");
        return;
    }

    // Return values
    //     0 – If successful.
    //     -EIO – I/O error when accessing an external GPIO chip.
    //     -EWOULDBLOCK – if operation would block.    
    result = gpio_pin_set_dt(green_led, 1);
    
    // SUCCESS_OR_EXIT(result)
    if (result != 0) 
    {
        printk("LED %s write failed!\n", green_led->name);
        printf("Exiting application...\n");
        return;
    }   

    // Usually just write without checking for streamlined program flow:
    gpio_pin_set_dt(green_led, 1);
