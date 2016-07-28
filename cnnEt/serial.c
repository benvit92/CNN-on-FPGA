#include <stdio.h>
#include <termios.h> //header contains the definitions used by the terminal I/O interfaces 
#include <unistd.h> //read() write() close()
#include <fcntl.h>
#include <string.h>

//Serial port defines and variables:
#define BAUDRATE B115200
#define SERIAL_PATH "/dev/ttyS0"//"/dev/ttyUSB0"
int serial_fd;
int serial_read_ret, serial_write_ret;
struct termios serial_settings;
char serial_buffer_send[1024] = "Yo Windows: ";
char serial_buffer_recv[1024] = {0};

int main() {
 printf("Program to write a string to the serial port and read a string from it.\n");
 printf("Make sure to run this program with elevated privileges.\n\n");
 printf("Opening %s in Read/Write mode at 115200 8-N-1...",SERIAL_PATH);
 
 fflush(stdout);
 //Try opening serial port
 serial_fd = open(SERIAL_PATH,O_RDWR|O_NOCTTY);
 if(serial_fd == -1) { //Checks the availability of the Serial Port
  printf("Failed.\n");
  fflush(stdout);
  return 0;
 } else {
  printf("Success.\n");
  fflush(stdout);

  //Get serial port settings
  tcgetattr(serial_fd, &serial_settings); //Get Current Settings of the Port
  cfsetispeed(&serial_settings,BAUDRATE); //Set Input Baudrate
  cfsetospeed(&serial_settings,BAUDRATE); //Set Output Baudrate
  serial_settings.c_cflag &= ~PARENB; //Mask Parity Bit as No Parity
  serial_settings.c_cflag &= ~CSTOPB; //Set Stop Bits as 1 or else it will be 2
  serial_settings.c_cflag &= ~CSIZE; //Clear the current no. of data bit setting
  serial_settings.c_cflag |= CS8; //Set no. of data bits as 8 Bits
 }

 serial_write_ret = write(serial_fd,serial_buffer_send,strlen(serial_buffer_send));
 printf("Sent to serial port: %s\n",serial_buffer_send);
	//printf("Result is %d\n",serial_write_ret);

	//usleep ((7 + 25) * 100000); 
	//printf("Woken Up\n");

 serial_read_ret = read(serial_fd,serial_buffer_recv,sizeof(serial_buffer_recv));
 //serial_read_ret = read(serial_fd,serial_buffer_recv,strlen(serial_buffer_send));
 //printf("Result is %d\n",serial_read_ret);
 printf("Read from serial port: %s\n",serial_buffer_recv);

 serial_read_ret = close(serial_fd); //Close the serial port

 printf("Serial port closed.\n\n");
 return 0;
}
