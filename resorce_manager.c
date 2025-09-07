#include "gpio_function.h"

static int my_devctl_handler(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);

int main(int argc, char *argv[]) {
    resmgr_attr_t resmgr_attr;
    dispatch_t *dpp;
    dispatch_context_t *ctp;
    resmgr_connect_funcs_t connect_funcs;
    resmgr_io_funcs_t io_funcs;
    iofunc_attr_t io_attr;

    if ((dpp = dispatch_create()) == NULL) {
        printf("dispatch_create failed\n");
        exit(EXIT_FAILURE);
    }

    memset(&resmgr_attr, 0, sizeof(resmgr_attr));
    resmgr_attr.nparts_max = 1;
    resmgr_attr.msg_max_size = 2048;

    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
                     _RESMGR_IO_NFUNCS, &io_funcs);

    io_funcs.devctl = my_devctl_handler;

    iofunc_attr_init(&io_attr, S_IFCHR | 0666, NULL, NULL);

    if (resmgr_attach(dpp, &resmgr_attr, "/dev/rpi_gpio",
                      _FTYPE_ANY, 0, &connect_funcs, &io_funcs, &io_attr) == -1) {
        printf("resmgr_attach failed\n");
        exit(EXIT_FAILURE);
    }

    ctp = dispatch_context_alloc(dpp);

    while (1) {
        if ((ctp = dispatch_block(ctp)) == NULL) {
            printf("dispatch_block failed\n");
            exit(EXIT_FAILURE);
        }
        dispatch_handler(ctp);
    }

    return 0;
}

static int my_devctl_handler(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb) {
    struct gpio_info gpio;   // Make local to avoid conflicts

    switch (msg->i.dcmd) {
        case MSG_COMD:
            // Read GPIO info from client
            if (resmgr_msgread(ctp, &gpio, sizeof(gpio), 0) != sizeof(gpio)) {
                return EIO;
            }

            if (gpio.mode == 0) { // Input
                set_gpio_input(gpio.pin);
                if (gpio.state_read == 1) {
                    int val = gpio_read_func(gpio.pin);
                    // Return value to client
                    resmgr_msgwrite(ctp, &val, sizeof(val), sizeof(msg->o));
                } else if (gpio.state_read == 0) {
                    printf("PIN %d SET INPUT\n", gpio.pin);
                } else {
                    printf("Invalid operation of GPIO\n");
                    return EIO;
                }
            } else if (gpio.mode == 1) { // Output
                set_gpio_output(gpio.pin);
                if (gpio.state == 0 || gpio.state == 1) {
                    set_gpio_state(gpio.pin, gpio.state);
                } else {
                    printf("Invalid operation of GPIO\n");
                    return EIO;
                }
            } else {
                printf("Invalid GPIO mode\n");
                return EIO;
            }
            break;

        default:
            printf("Wrong argument\n");
            return EINVAL;
    }

    return 0;
}

