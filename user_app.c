#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <devctl.h>
#include <errno.h>

#define GPIO_MIN 2
#define GPIO_MAX  27

struct gpio_info{
	int pin;
	int mode;
	int state;
	int state_read;
}gpio;

#define MSG_COMD __DIOT(_DCMD_MISC, 1, struct gpio_info)


int main(int argc, char *argv[]) {

	gpio.mode = 2;
	gpio.state = 2;
	int check_arg_p;
	int check_arg_o;
	int check_arg_i;
	int check_arg_s;

    int fd = open("/dev/rpi_gpio", O_RDWR);
    if (fd == -1) {
        printf("ERROR FILE OPENING\n");
        return ENOENT;
    }
    for (int i=1; i<argc; i++) {
    	if (argv[i][0] == '-') {
    		switch (argv[i][1]) {
    			case 'p':
    					check_arg_p =1;
    					gpio.pin = atoi(&argv[i][2]);
    					if (gpio.pin < GPIO_MIN || gpio.pin > GPIO_MAX ) {
    					    printf("Argument -p is Invalid\n");
    					    return EINVAL;
    					}
    					break;
    			case 'i':
    					gpio.mode = 0;
    					check_arg_i =1;
    					break;
    			case 'o':
    					gpio.mode = 1;
    					check_arg_o =1;
    					break;
    			case 's':
    					gpio.state = atoi(&argv[i][2]);
    					if (!(gpio.state == 0 || gpio.state == 1) ) {
    						printf("Argument %s is Invalid\n",argv[i]);
    						return EINVAL;
    					}
    					check_arg_s =1;
    					break;
    			case 'r':
    					gpio.state_read =1;
    					if (!( gpio.state_read == 1 )) {
    						printf("Argument %s is Invalid\n",argv[i]);
    						return EINVAL;
    					}
    					break;
    			default :
    					printf("Arguments %d is Invalid\n",i);
    					return EINVAL;
    		}
    	}
    }
    if (!(check_arg_p == 1) ){
    	printf("Argument -p Missing\n");
    	return EINVAL;
    }
    if (check_arg_p == 1 && !(gpio.mode == 0 || gpio.mode ==1 )) {
    	printf("Pin mode not set \n");
    	return EINVAL;
    }
    if (check_arg_s == 1 && !(check_arg_o  == 1)) {
    	printf("Argument -s required -o\n");
    	return EINVAL;
    }
    if (!(check_arg_s == 1) && check_arg_o ==1) {
    	printf("Argument -o required -s\n");
    	return EINVAL;
    }
//    if (!(gpio.mode == 0)) {
//    	if (!(gpio.mode == 1 && (gpio.state == 0 || gpio.state ==1 ))) {
//    		printf("Argument -o and -s not  \n");
//        	return EIO;
//     	}
//     }
    if (gpio.mode == 1 && gpio.state_read == 1) {
    	printf("Argument -o not use with -r\n");
    	return EPERM;
    }
    if (gpio.mode == 0 && (gpio.state == 0 || gpio.state == 1)) {
    	printf("Argument -i not use with -s\n");
    	return EPERM;//not opration permission
    }
    if (check_arg_i == 1 && check_arg_o ==1){
    	printf("input and output both mode not set at time\n");
    	return EIO;
    }

    int ret = devctl(fd, MSG_COMD, &gpio, sizeof(struct gpio_info), NULL);

    if (ret != 0) {
        printf("devctl failed\n");
        return ENOTTY;
    }

    close(fd);
    return 0;
}
