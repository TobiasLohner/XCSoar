/***********************************************************************
**
**   vla_support.cpp
**
**   This file is part of libkfrgcs.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht,
**                           Garrecht Ingenieurgesellschaft
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef VLA_SUPPORT
#define VLA_SUPPORT

#include "vlapityp.h"
#include "vlapierr.h"

#include <stddef.h>

class Port;
class OperationEnvironment;

/*
	This class contains target system independent base functions
	for communication with the VOLKSLOGGER.
	These functions are critical and normally don't need to be changed
	by the user. If you think that something has to be changed here,
	contact GARRECHT for comments on your intended changes.
*/
class VLA_XFR {
protected:
  Port &port;
  OperationEnvironment &env;

  int32 databaud; // Baudrate as integer (e.g. 115200)

public:
  VLA_XFR(Port &port, unsigned _databaud, OperationEnvironment &env);
  void set_databaud(int32 db);

	// establish connection with VL within specified time
  VLA_ERROR connect(int32 waittime);

  VLA_ERROR readinfo(void *buffer, int32 buffersize);

  // write binary DATABASE-block to VL
  VLA_ERROR dbbput(const void *dbbbuffer, int32 dbbsize);
  // read binary DATABASE from VL
  VLA_ERROR dbbget(void *dbbbuffer, int32 dbbsize);
  // read all binary flight logs from VL
  VLA_ERROR all_logsget(void *dbbbuffer, int32 dbbsize);

  /**
   * Read one binary flight log from VL.
   *
   * @return the number of bytes written to the buffer or 0 on error
   */
  size_t flightget(void *buffer, size_t buffersize,
                   unsigned flightnr, bool secmode);

  /**
   * read Flightlist(Directory) in binary format from VL to buffer,
   * return the length of the read data in bytes
   * or -1 if a problem was encountered and the data read failed.
   */
   int readdir(void *buffer, int32 buffersize);
};

#endif
