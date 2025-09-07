#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <devctl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define GPIO_MIN 2
#define GPIO_MAX 27

struct gpio_info {
    int pin;
    int mode;
    int state;
    int state_read;
} gpio;

#define MSG_COMD __DIOT(_DCMD_MISC, 1, struct gpio_info)

int main(int argc, char *argv[]) {

    gpio.mode = -1;
    gpio.state = -1;
    gpio.state_read = 0;
    

    uint16_t pin_flag = 0;
    uint16_t output_flag = 0;
    uint16_t input_flag = 0;
    uint16_t state_flag = 0;

    int fd = open("/dev/rpi_gpio", O_RDWR);
    if (fd == -1) {
        printf("ERROR FILE OPENING\n");
        return ENOENT;
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'p':
		    if (argv[i][4] == 0){
                        pin_flag = 1;
                        gpio.pin = atoi(&argv[i][2]);
		    }
                    if (gpio.pin < GPIO_MIN || gpio.pin > GPIO_MAX) {
                        printf("Argument %s is Invalid\n",argv[i]);
                        return EINVAL;
                    }
                    break;
                case 'i':
		    if ( argv[i][2] == 0 ) {
		        gpio.mode = 0;
		        input_flag = 1;
		    }
		    else { 
			printf("Argument %s is Invalid\n",argv[i]);
			return EINVAL;
		    }
                    break;
                case 'o':
		    if (argv[i][2] == 0 ){
                        gpio.mode = 1;
                        output_flag = 1;
		    }
		    else {
			printf("Argument %s is Invalid\n",argv[i]);
			return EINVAL;
		    }
                    break;
                case 's':
		    if (argv[i][3]  == 0 ){
		        if (argv[i][2] == 48 || argv[i][2] == 49) {
                            gpio.state = atoi(&argv[i][2]);
                            state_flag = 1;
		        }
		    }
		    else {
			    printf("Argument %s is Invalid\n",argv[i]);
			    return EINVAL;
		    }
                    break;
                case 'r':
		    if (argv[i][2] == 0 ){
                        gpio.state_read = 1;
		    }else {
			printf("Argument %s is Invalid\n",argv[i]);
			return EINVAL;
		    }
                    break;
                default:
                    printf("Arguments %d is Invalid\n", i);
                    return EINVAL;
            }
        }
    }

    if (!(pin_flag == 1)) {
        printf("Argument -p Missing\n");
        return EINVAL;
    }

    if (pin_flag == 1 && !(gpio.mode == 0 || gpio.mode == 1)) {
        printf("Pin Mode Not Set \n");
        return EINVAL;
    }

    if (input_flag == 1 && output_flag == 1) {
        printf("Input and Output Mode Not Use in single Command\n");
        return EIO;
    }

    if (state_flag == 1 && !(output_flag == 1)) {
        printf("State High or Low  Need Output Argument\n");
        return EINVAL;
    }

    if (output_flag == 1 && state_flag == 0) {
        printf("Output Mode Need Argument State High or Low\n");
	return EINVAL;
    }

    if (gpio.mode == 1 && gpio.state_read == 1) {
        printf("Output Mode  Not Use with State Read(-r)\n");
        return EPERM;
    }

    if (gpio.mode == 0 && (gpio.state == 0 || gpio.state == 1)) {
        printf("Input Mode Not Use with State High (-s1) or Low (-s0)\n");
        return EPERM; 
    }

    int ret = devctl(fd, MSG_COMD, &gpio, sizeof(struct gpio_info), NULL);

    if (ret != 0) {
        printf("devctl failed \n");
        return ENOTTY;
    }

    close(fd);
    return 0;
}

