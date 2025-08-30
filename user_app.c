#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <devctl.h>
#include <string.h>
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

    int fd = open("/dev/rpi_gpio", O_RDWR);
    if (fd == -1) {
        printf("ERROR IN FILE OPEN\n");
        return 1;
    }
    for (int i=1; i<argc; i++) {

    	switch (argv[i][0]){
    		case 'p':
    			gpio.pin = atoi(&argv[i][1]);
    			if (!(gpio.pin > 1 || gpio.pin > 28) ) {
    			    printf("Argument %s Invalid\n",argv[i]);
    			    return 1;
    			}
    			break;
    		case 'm':
    			printf("PRINT MODE\n");
    			gpio.mode = atoi(&argv[i][1]);
    			printf("mode %d",gpio.mode);
    			if (!(gpio.mode == 0 || gpio.mode ==1) ) {
    			    printf("Argument %s Invalid\n",argv[i]);
    			    return 1;
    			}
    			break;
    		case 's':
    			printf("PRINT STATE\n");
    			gpio.state = atoi(&argv[i][1]);
    			if (!(gpio.state == 0 || gpio.state == 1) ) {
    			    printf("Argument %s Invalid\n",argv[i]);
    			    return 1;
    			}
			break;
    		case 'r':
    			printf("PRINT READ\n");
    			gpio.state_read = atoi(&argv[i][1]);
    			if (!(gpio.state_read == 0 || gpio.state_read == 1)) {
    			    printf("Argument %s Invalid\n",argv[i]);
    			    return 1;
    			}
    			break;
    		default :
    			printf("Arguments %d are Invalid\n",i);
    			return 2;
    	}
    }

    int ret = devctl(fd, MSG_COMD, &gpio, sizeof(struct gpio_info), NULL);

    printf("Send vleue\n");
    if (ret != 0) {
        printf("devctl failed\n");
        return 3;
    }

    close(fd);
    return 0;
}
