/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Teunis van Beelen
*
* teuniz@gmail.com
*
***************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation version 2 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
***************************************************************************
*
* This version of GPL is at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*
***************************************************************************
*/

/* last revision: February 1, 2013 */

/* For more info and how to use this libray, visit: http://www.teuniz.net/RS-232/ */


#include "rs232.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>

RS232::RS232 (PCSTR dev, int baudrate)
{
	mPort = -1 ;
	Open (dev, baudrate) ;
}

RS232::RS232 ()
{
	mPort = -1 ;
}
RS232::~RS232 ()
{
	Close () ;
	mPort = -1 ;
}

bool RS232::Open(PCSTR dev, int baudrate)
{
  int baudr, status;
#define CASE_LOC(val) case val: baudr=B##val; break ;
  switch(baudrate) {
    CASE_LOC(50) ;
    CASE_LOC(75) ;
    CASE_LOC(110) ;
    CASE_LOC(134) ;
    CASE_LOC(150) ;
    CASE_LOC(200) ;
    CASE_LOC(300) ;
    CASE_LOC(600) ;
    CASE_LOC(1200) ;
    CASE_LOC(1800) ;
    CASE_LOC(2400) ;
    CASE_LOC(4800) ;
    CASE_LOC(9600) ;
    CASE_LOC(19200) ;
    CASE_LOC(38400) ;
    CASE_LOC(57600) ;
    CASE_LOC(115200) ;
    CASE_LOC(230400) ;
    CASE_LOC(460800) ;
    CASE_LOC(500000) ;
    CASE_LOC(576000) ;
    CASE_LOC(921600) ;
    CASE_LOC(1000000) ;
    default:
		printf("invalid baudrate\n");
        return false ;
		break;
  }

  mPort  = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
  if(mPort==-1) {
    perror("unable to open comport ");
    return false ;
  }

  int error = tcgetattr(mPort, &mOldPortSettings);
  if(error==-1) {
    close(mPort);
    perror("unable to read portsettings ");
    return false ;
  }
  memset(&mNewPortSettings, 0, sizeof(mNewPortSettings));  /* clear the new struct */

  mNewPortSettings.c_cflag = baudr | CS8 | CLOCAL | CREAD;
  mNewPortSettings.c_iflag = IGNPAR;
  mNewPortSettings.c_oflag = 0;
  mNewPortSettings.c_lflag = 0;
  mNewPortSettings.c_cc[VMIN] = 0;      /* block untill n bytes are received */
  mNewPortSettings.c_cc[VTIME] = 0;     /* block untill a timer expires (n * 100 mSec.) */
  error = tcsetattr(mPort, TCSANOW, &mNewPortSettings);
  if(error==-1) {
    close(mPort);
    perror("unable to adjust portsettings ");
    return false ;
  }

  if(ioctl(mPort, TIOCMGET, &status) == -1) {
    perror("unable to get portstatus");
    return false ;
  }

  status |= TIOCM_DTR;    /* turn on DTR */
  status |= TIOCM_RTS;    /* turn on RTS */

  if(ioctl(mPort, TIOCMSET, &status) == -1) {
    perror("unable to set portstatus");
    return false ;
  }

  printf ("mPort=%d   baudr:%d  baudrate;%d\n", mPort, baudr, baudrate) ;
  return true ;
}

int RS232::ReadBufMax(puint8_t buf, int size)
{
  int n;

  if(size > 4096)  size = 4096;

  n = read(mPort, buf, size);

  return(n);
}

bool RS232::ReadBuf(puint8_t buf, int size, int *addrRemain)
{
	puint8_t ptr = buf ;
	do {
		int ret = ReadBufMax (ptr, size) ;
		if (ret > 0) {
			size -= ret ;
			ptr += ret ;
		}
		else {
//FETS				printf ("RS232::ReadBuf FAILED : ret=%d\n", ret) ;
			break ;
		}
	} while (size != 0) ;
	if (addrRemain) *addrRemain = size ;
//FETS		if (size != 0) {
//FETS			printf ("RS232::ReadBuf FAILED : sizeRemaining=%d\n", size) ;
//FETS		}
	return (size == 0) ;
}

bool RS232::SendByte (uint8_t byte)
{
  int n;

  n = write (mPort, &byte, 1);
  if(n<0)  return false;

  return true ;
}

#if 0
int RS232::SendBuf(uint8_t *buf, int size)
{
  return write(mPort, buf, size);
}
#endif
bool RS232::SendBuf(uint8_t *buf, int size)
{
#ifdef DBG_SS232
	printf ("SendBuf:") ;
	for (int i = 0; i < size; i++) {
		printf ("%c ", char(buf[i])) ;
	}
	printf ("\n") ;
#endif // DBG_SS232

  return (write(mPort, buf, size) == size) ;
}

void RS232::Close ()
{
	if (mPort == -1) return ;

  int status;

  if(ioctl(mPort, TIOCMGET, &status) == -1) {
    perror("unable to get portstatus");
  }

  status &= ~TIOCM_DTR;    /* turn off DTR */
  status &= ~TIOCM_RTS;    /* turn off RTS */

  if(ioctl(mPort, TIOCMSET, &status) == -1) {
    perror("unable to set portstatus");
  }

  close(mPort);
  tcsetattr(mPort, TCSANOW, &mOldPortSettings);
}

/*
Constant  Description
TIOCM_LE  DSR (data set ready/line enable)
TIOCM_DTR DTR (data terminal ready)
TIOCM_RTS RTS (request to send)
TIOCM_ST  Secondary TXD (transmit)
TIOCM_SR  Secondary RXD (receive)
TIOCM_CTS CTS (clear to send)
TIOCM_CAR DCD (data carrier detect)
TIOCM_CD  Synonym for TIOCM_CAR
TIOCM_RNG RNG (ring)
TIOCM_RI  Synonym for TIOCM_RNG
TIOCM_DSR DSR (data set ready)
*/

bool RS232::IsCTSEnabled ()
{
  int status;

  ioctl(mPort, TIOCMGET, &status);

  if(status&TIOCM_CTS) return false ;
  else return true ;
}



//FETS	char comports[30][16]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2","/dev/ttyS3","/dev/ttyS4","/dev/ttyS5",
//FETS	                       "/dev/ttyS6","/dev/ttyS7","/dev/ttyS8","/dev/ttyS9","/dev/ttyS10","/dev/ttyS11",
//FETS	                       "/dev/ttyS12","/dev/ttyS13","/dev/ttyS14","/dev/ttyS15","/dev/ttyUSB0",
//FETS	                       "/dev/ttyUSB1","/dev/ttyUSB2","/dev/ttyUSB3","/dev/ttyUSB4","/dev/ttyUSB5",
//FETS	                       "/dev/ttyAMA0","/dev/ttyAMA1","/dev/ttyACM0","/dev/ttyACM1",
//FETS	                       "/dev/rfcomm0","/dev/rfcomm1","/dev/ircomm0","/dev/ircomm1"};

bool RS232::IsDSREnabled ()
{
  int status;

  ioctl(mPort, TIOCMGET, &status);

  if(status&TIOCM_DSR) return(1);
  
  return(0);
}

void RS232::EnableDTR ()
{
  int status;

  if(ioctl(mPort, TIOCMGET, &status) == -1) {
    perror("unable to get portstatus");
  }

  status |= TIOCM_DTR;    /* turn on DTR */

  if(ioctl(mPort, TIOCMSET, &status) == -1) {
    perror("unable to set portstatus");
  }
}

void RS232::DisableDTR ()
{
  int status;

  if(ioctl(mPort, TIOCMGET, &status) == -1) {
    perror("unable to get portstatus");
  }

  status &= ~TIOCM_DTR;    /* turn off DTR */

  if(ioctl(mPort, TIOCMSET, &status) == -1) {
    perror("unable to set portstatus");
  }
}

void RS232::EnableRTS ()
{
  int status;

  if(ioctl(mPort, TIOCMGET, &status) == -1) {
    perror("unable to get portstatus");
  }

  status |= TIOCM_RTS;    /* turn on RTS */

  if(ioctl(mPort, TIOCMSET, &status) == -1) {
    perror("unable to set portstatus");
  }
}

void RS232::DisableRTS ()
{
  int status;

  if(ioctl(mPort, TIOCMGET, &status) == -1) {
    perror("unable to get portstatus");
  }

  status &= ~TIOCM_RTS;    /* turn off RTS */

  if(ioctl(mPort, TIOCMSET, &status) == -1) {
    perror("unable to set portstatus");
  }
}

void RS232::SendString  (PCSTR text)  /* sends a string to serial port */
{
  while(*text != 0)   RS232::SendByte(*(text++));
}

