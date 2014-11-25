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


#ifndef rs232_INCLUDED
#define rs232_INCLUDED

#include <termios.h>
#include "ISerial.h"

typedef const char * PCSTR ;

class RS232 : public ISerial 
{
public:
	RS232 (PCSTR dev, int baud) ;
	RS232 () ;
	~RS232 () ;
	bool	Open (PCSTR dev, int baud);
	int		ReadBufMax (puint8_t, int);
	virtual bool	ReadBuf (puint8_t, int, int *addrRemain=NULL);
	virtual bool	SendBuf(puint8_t, int);
	bool	SendByte(uint8_t);
	void	Close();
	void	SendString (const char *);
	bool	IsCTSEnabled();
	bool	IsDSREnabled();
	void	EnableDTR();
	void	DisableDTR();
	void	EnableRTS();
	void	DisableRTS();

protected:
	int		mPort ;
	struct termios mNewPortSettings;
	struct termios mOldPortSettings;

} ;

#endif


