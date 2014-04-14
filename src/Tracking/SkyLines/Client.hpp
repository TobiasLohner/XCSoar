/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2014 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_TRACKING_SKYLINES_CLIENT_HPP
#define XCSOAR_TRACKING_SKYLINES_CLIENT_HPP

#include "Handler.hpp"
#include "Protocol.hpp"
#include "OS/SocketAddress.hpp"
#include "OS/SocketDescriptor.hpp"
#include "IO/Async/FileEventHandler.hpp"
#include "Util/OverwritingRingBuffer.hpp"

#include <stdint.h>

struct NMEAInfo;
class IOThread;
class SocketAddress;

namespace SkyLinesTracking {
  struct TrafficResponsePacket;
  struct UserNameResponsePacket;

  class Client
#ifdef HAVE_SKYLINES_TRACKING_HANDLER
    : private FileEventHandler
#endif
  {
#ifdef HAVE_SKYLINES_TRACKING_HANDLER
    IOThread *io_thread;
    Handler *handler;
#endif

    // The tracking key
    uint64_t key;

    // Flag if we stored a previous packet already
    bool last_packet_valid;

    SocketAddress address;
    SocketDescriptor socket;

    static constexpr unsigned MAX_DELTA_FIXES = 40;

    /**
     * List of delta fixes. Maximum is 40 to keep the total UDP packet size
     * at roundabout 1000 bytes.
     */
    OverwritingRingBuffer<DeltaFix, MAX_DELTA_FIXES> delta_fixes;

    // The last packet sent, store it to calculate the next delta packet
    FixPacket last_packet;

    /**
     * Time in milliseconds of the last fix that was received and
     * ack'ed by SkyLines
     */
    unsigned last_fix_received;

  public:
    Client()
      :
#ifdef HAVE_SKYLINES_TRACKING_HANDLER
      io_thread(nullptr), handler(nullptr),
#endif
      key(0), last_packet_valid(false) {}
    ~Client() { Close(); }

    constexpr
    static unsigned GetDefaultPort() {
      return 5597;
    }

#ifdef HAVE_SKYLINES_TRACKING_HANDLER
    void SetIOThread(IOThread *io_thread);
    void SetHandler(Handler *handler);
#endif

    bool IsDefined() const {
      return socket.IsDefined();
    }

    void SetKey(uint64_t _key) {
      key = _key;
    }

    bool Open(const SocketAddress &_address);
    void Close();

    bool SendFix(const NMEAInfo &basic);
    bool SendPing(uint16_t id);
    bool SendTrafficRequest(bool followees, bool club);
    bool SendUserNameRequest(uint32_t user_id);

    void StoreDeltaPacket(FixPacket &packet);

#ifdef HAVE_SKYLINES_TRACKING_HANDLER
  private:
    void OnTrafficReceived(const TrafficResponsePacket &packet, size_t length);
    void OnUserNameReceived(const UserNameResponsePacket &packet,
                            size_t length);
    void OnDatagramReceived(void *data, size_t length);

    /* virtual methods from FileEventHandler */
    virtual bool OnFileEvent(int fd, unsigned mask) override;
#endif
  };
}

#endif
