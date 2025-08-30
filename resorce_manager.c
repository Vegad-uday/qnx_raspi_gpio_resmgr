
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <devctl.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/mman.h>


#define BCM2711_GPIO_BASE                   0xfe200000
#define GPSET0    7
#define GPLEV0	(0x34/4)
#define GPCLR0    (0x28/4)
#define BLOCK_SIZE	(4*1024)
#define GPIO_SR		0x07
#define OP	1
#define IP	0


struct gpio_info{
	int pin;
	int mode;
	int state;
	int state_read;
}gpio;

static uint32_t volatile *gpio_base;

#define MSG_COMD __DIOT(_DCMD_MISC, 1, struct gpio_info)

static int set_gpio_input(int gpio_pin);
static int set_gpio_output(int gpio_pin);
static int set_gpio_state(int gpio_pin , int state);
static int gpio_read_func(int gpio_pin);

static int my_devctl_handler(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);

int main(int argc, char *argv[]) {
    resmgr_attr_t resmgr_attr;
    dispatch_t *dpp;
    dispatch_context_t *ctp;
    resmgr_connect_funcs_t connect_funcs;
    resmgr_io_funcs_t io_funcs;
    iofunc_attr_t io_attr;

    // Create dispatch
    if ((dpp = dispatch_create()) == NULL) {
        perror("dispatch_create");
        exit(EXIT_FAILURE);
    }

    // Initialize default attributes
    memset(&resmgr_attr, 0, sizeof(resmgr_attr));
    resmgr_attr.nparts_max = 1;
    resmgr_attr.msg_max_size = 2048;

    // Set up default I/O functions
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,_RESMGR_IO_NFUNCS, &io_funcs);

    io_funcs.devctl = my_devctl_handler;

    // Set up attributes
    iofunc_attr_init(&io_attr, S_IFCHR | 0666, NULL, NULL);

    // Attach resource manager to /dev/rpi_gpio
    if (resmgr_attach(dpp, &resmgr_attr, "/dev/rpi_gpio",_FTYPE_ANY, 0, &connect_funcs, &io_funcs, &io_attr) == -1) {
        printf("resmgr_attach failed\n");
        exit(EXIT_FAILURE);
    }
    //GPIO_BASE gets  physical address
    gpio_base = mmap_device_memory(NULL,BLOCK_SIZE,PROT_NOCACHE|PROT_READ|PROT_WRITE,0,BCM2711_GPIO_BASE);
    if (gpio_base == (uint32_t *) MAP_FAILED) {
    	printf("ERROR :GPIO Memory map failed\n");
    	return -1;
    }

    ctp = dispatch_context_alloc(dpp);

    while (1) {
        if ((ctp = dispatch_block(ctp)) == NULL) {
            perror("dispatch_block");
            exit(EXIT_FAILURE);
        }
        dispatch_handler(ctp);
    }

    return 0;
}

//function for set gpio as input output
static int set_gpio_input(int gpio_pin) {
    int reg = gpio_pin / 10;
    int sel = gpio_pin % 10;
    int val = *(gpio_base + reg );

    val &= ~(GPIO_SR << (3 * sel));
    val |= ( IP & GPIO_SR) << (3*sel);
    *(gpio_base + reg )=val;

    return 0;
}
//function for set gpio as  output
static int set_gpio_output(int gpio_pin) {

    int reg = gpio_pin / 10;
    int sel = gpio_pin % 10;
    int val = *(gpio_base + reg );

    val &= ~(GPIO_SR << (3 * sel));
    val |= ( OP & GPIO_SR) << (3*sel);
    *(gpio_base + reg )=val;

    return 0;
}

//function for set pin high and low
static int set_gpio_state(int gpio_pin , int state) {

    if (state != 0) {
    	*(gpio_base + GPSET0) = (uint32_t)(0x1 << gpio_pin);
    	printf("PIN %d SET OUTPUt HIGH\n",gpio_pin);
    }
    else {
    	*(gpio_base + GPCLR0) = (uint32_t)(0x1 << gpio_pin);
    	printf("PIN %d SET OUTPUT LOW\n",gpio_pin);
    }
    return 0;
}

//function for gpio read
static int gpio_read_func(int gpio_pin){
	int gpio_level =*(gpio_base + GPLEV0);
	gpio_level &= (1 << gpio_pin);
	if (gpio_level == 0) {
		printf("PIN %d is LOW\n",gpio_pin);
		return 0;
	}
	else {
		printf("PIN %d is HIGH\n",gpio_pin);
		return 0;
	}
}


//devctl handler
static int my_devctl_handler(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb) {

    switch (msg->i.dcmd) {
        case MSG_COMD:

            resmgr_msgread(ctp, &gpio, sizeof(struct gpio_info), sizeof(msg->i));

            if (gpio.mode == 0) {
            	set_gpio_input(gpio.pin);
            	if (gpio.state_read == 1) {
            		gpio_read_func(gpio.pin);
            	}
            	else if (gpio.state_read == 0) {
            		printf("PIN %d SET INPUT \n",gpio.pin);
            	} else {
            		printf("Invalid opration of GPIO\n");
            		return EIO;
            	}
            }
            else if (gpio.mode == 1 ) {
            	set_gpio_output(gpio.pin);
            	if (gpio.state == 0) {
            		set_gpio_state(gpio.pin, gpio.state);
            	}
            	else if (gpio.state == 1) {
            		set_gpio_state(gpio.pin, gpio.state);
            	}
            	else {
            		printf("Invalid opration of GPIO\n");
            		return EIO;
            	}
            }
            break;

        default:
        	printf("wrong Argumrnt\n");
            return EINVAL;
    }

    return 0;
}

