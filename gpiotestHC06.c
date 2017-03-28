//https://github.com/chiragnagpal/beaglebone_mmap
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include "beaglebone_gpio.h"
#include "../Beaglebone/beaglebone_gpio.h"
#include <errno.h>   /* Error number definitions */
// #include <termios.h> /* POSIX terminal control definitions */

#define DEVICE "/dev/ttyO4"
#define BAUDRATE B115200

int main(int argc, char *argv[]) {
	volatile void *gpio_addr = NULL;
	volatile unsigned int *gpio_oe_addr = NULL;
	volatile unsigned int *gpio_setdataout_addr = NULL;
	volatile unsigned int *gpio_dataout_addr = NULL;
	volatile unsigned int *gpio_cleardataout_addr = NULL;
	unsigned int reg;
	int fd = open("/dev/mem", O_RDWR);

	printf("Mapping %X - %X (size: %X)\n", GPIO_START_ADDR, GPIO_END_ADDR, GPIO_SIZE);

	gpio_addr = mmap(0, GPIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_START_ADDR);

	gpio_oe_addr = gpio_addr + GPIO_OE;
	gpio_setdataout_addr = gpio_addr + GPIO_SETDATAOUT;
	gpio_cleardataout_addr = gpio_addr + GPIO_CLEARDATAOUT;
        gpio_dataout_addr = gpio_addr + GPIO_DATAOUT;

	if(gpio_addr == MAP_FAILED) {
		printf("Unable to map GPIO\n");
		exit(1);
	}
	printf("GPIO mapped to %p\n", gpio_addr);
	printf("GPIO OE mapped to %p\n", gpio_oe_addr);
	printf("GPIO SETDATAOUTADDR mapped to %p\n", gpio_setdataout_addr);
	printf("GPIO CLEARDATAOUT mapped to %p\n", gpio_cleardataout_addr);

	reg = *gpio_oe_addr;
	// reg = reg & (0xFFFFFFFF - LED12 - LED15  /*- USR1_LED*/); //USR_LED pin 28 GPIO board 1, pin 12 of header 9
	reg = reg & (0xFFFFFFFF ^ LED12 ^ LED15); //0x01 ^ 0xFF = 0xFE one's complement ~ 2 LED OFF
	*gpio_oe_addr = reg;
	printf("GPIO1 configuration: %X\n", reg);           //GPIO1_28
        //stty -F /dev/ttyO2 9600 raw -echo (setup termninal for /dev/ttyO4)
        int fd1 = open(DEVICE, O_RDWR | O_NOCTTY);  /* open for read/write */

    if(fd1 == -1)
    {
        printf("file %s either doesnot exit, or locked by another process\n", DEVICE);
        exit(-1);
    }
    int act, numBytes ;
    char ch, write_buf[10], read_buf[10];
    printf("Start blinking from Bluetooth H06\n");
	while(1) {
	     printf("In loop wait for data from ttyO4 to read...\n");
	    numBytes = read(fd1, read_buf, sizeof(read_buf));
	    if (numBytes < 0)
	    {
	        printf("Failed to read %d\n", numBytes);
	        //break;
	    }
        printf("Read_buff  numBytes = %d data = %s\n", numBytes, read_buf);

	    if (*read_buf == '1')
	    {
	        *gpio_dataout_addr = LED15 ^ LED12;
	        printf("LED ON\n");	
		numBytes = write(fd1, "LED is ON", 9);
	    }

        else
        {
            *gpio_dataout_addr = (LED15 ^ LED12 ^ 0xFFFFFFFF);
            printf("LED OFF\n");
	    numBytes = write(fd1, "LED is OFF", 10);	
        }

		sleep(1);
	}

	close(fd);
	return 0;
}
/*
http://unix.stackexchange.com/questions/167948/how-does-mmaping-dev-mem-work-despite-being-from-unprivileged-mode
ubuntu@ubuntu-armhf:~$ sudo cat /proc/9370/maps
00008000-00009000 r-xp 00000000 b3:02 124989     /home/ubuntu/Ctest/Beaglebone/gpiotest ???
00010000-00011000 r--p 00000000 b3:02 124989     /home/ubuntu/Ctest/Beaglebone/gpiotest
00011000-00012000 rw-p 00001000 b3:02 124989     /home/ubuntu/Ctest/Beaglebone/gpiotest
b6fd4000-b6fd6000 rw-s 4804c000 00:05 2292       /dev/mem                      (here is physical to virtual memory) ***io is file /dev/mem
*/
