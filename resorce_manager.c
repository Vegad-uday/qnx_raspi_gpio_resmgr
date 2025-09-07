#include "gpio_function.h"

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
        printf("dispatch_create");
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

    ctp = dispatch_context_alloc(dpp);

    while (1) {
        if ((ctp = dispatch_block(ctp)) == NULL) {
            printf("dispatch_block");
            exit(EXIT_FAILURE);
        }
        dispatch_handler(ctp);
    }

    return 0;
}


//devctl handler
static int my_devctl_handler(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb) {

    switch (msg->i.dcmd) {
        case MSG_COMD:

            resmgr_msgread(ctp, &gpio, sizeof(struct gpio_info), sizeof(msg->i));
    printf("This is line %d of the program.\n", __LINE__);
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
    printf("This is line %d of the program.\n", __LINE__);
            	set_gpio_output(gpio.pin);
            	if (gpio.state == 0) {
            		set_gpio_state(gpio.pin, gpio.state);
    printf("This is line %d of the program.\n", __LINE__);
            	}
            	else if (gpio.state == 1) {
    printf("This is line %d of the program.\n", __LINE__);
            		set_gpio_state(gpio.pin, gpio.state);
            	}
            	else {
    printf("This is line %d of the program.\n", __LINE__);
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

