#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "bcm2711.h"

#define GPFSEL1   1
#define GPSET0    7
#define GPCLR0    (0x28/4)
#define GPIO_MIN	2
#define GPIO_MAX	27
#define BLOCK_SIZE	(4*1024)
#define GPIO_SR		0x07

static uint32_t volatile *gpio_base;

//function for set gpio as input output
static int set_gpio_func(int gpio_pin , int func) {

    int reg = gpio_pin / 10;
    int sel = gpio_pin % 10;
    int val = *(gpio_base + reg );

    val &= ~(GPIO_SR << (3 * sel));
    val |= (func & GPIO_SR) << (3*sel);
    *(gpio_base + reg )=val;

    return 0;
}

//function for set pin high and low
static int set_gpio_state(int gpio_pin , int state) {

    if (state != 0) {
    	*(gpio_base + GPSET0) = (uint32_t)(0x1 << gpio_pin);
    }
    else {
    	*(gpio_base + GPCLR0) = (uint32_t)(0x1 << gpio_pin);
    }
    return 0;
}

int main(int argc, char *argv[]) {

    int gpio_pin;
    int set;
    int op =1;
    int ip =0;

    if (argc < 2 ) {
        printf("ERROR : Argument is not given - try Again\n");
        return EINVAL;
    }

    set = strcmp(argv[1],"set");
    if (set) {
    	printf("ERROR : Argument 1 %s is Invalid\n",argv[1]);
    	return EINVAL;
    }


    //BCM2711_GPIO_BASE this register assign to physical address that return array of 32 bit integer(4 byte)
    gpio_base = mmap_device_memory(NULL,BLOCK_SIZE,PROT_NOCACHE|PROT_READ|PROT_WRITE,0,BCM2711_GPIO_BASE);
    if (gpio_base == (uint32_t *) MAP_FAILED) {
        printf("ERROR :GPIO Memory map failed\n");
        return -1;
    }

    int pin = atoi(argv[2]);
    int count =0;
    for (int i= GPIO_MIN; i <= GPIO_MAX; i++ ) {

    	if (pin == i ) {
    		gpio_pin =i;
    		count ++;
    	}
    }
    if (count == 0) {
    	printf("Please Enter GPIO between 2 to 27 because this pins are only Function input output\n");
    	printf("ERROR : Argument 2 %s is Invalid -Try Agin \n",argv[2]);
    	return EINVAL;
    }

    int check = strcmp(argv[3] , "op");
    int check3 = strcmp(argv[3] , "ip");

    if (check == 0) {
    	set_gpio_func(gpio_pin , op);
    	check = strcmp(argv[4] ,"o1");
    	int check2 = strcmp(argv[4] ,"o0");
    	if (check == 0) {
    		set_gpio_state(gpio_pin ,1);
    	}
    	else if (check2 == 0) {
    		set_gpio_state(gpio_pin ,0);
    	}
    	else {
    		printf("ERROR : Argument 4 is Invalid - Try Again\n");
    		return EINVAL;
    	}
    }
    else if (check3 == 0) {
    	set_gpio_func(gpio_pin , ip);
    	check =(strcmp(argv[4],"o1") || strcmp(argv[4] ,"o0"));
    	if (check == 0) {
    		printf("ERROR : YOU can't set GPIO as HIGH OR LOW \n");
    		return EIO;
    	}
    }
    else {
    	printf("ERROR : Argument 3 is  Invalid Try Again\n");
    	return EINVAL;
    }

   return 0;
}

