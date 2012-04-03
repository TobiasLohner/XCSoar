/* Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
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

#include "Logger/MD5.hpp"
#include "Util/Macros.hpp"
#include "OS/ByteOrder.hpp"
#include "Compiler.h"

#include <algorithm>
#include <stdio.h>

static gcc_constexpr_data uint32_t k[64] = {
  // k[i] := floor(abs(sin(i)) * (2 pow 32))
  // RLD should be sin(i + 1) but want compatibility
  3614090360UL, // k=0
  3905402710UL, // k=1
  606105819UL, // k=2
  3250441966UL, // k=3
  4118548399UL, // k=4
  1200080426UL, // k=5
  2821735955UL, // k=6
  4249261313UL, // k=7
  1770035416UL, // k=8
  2336552879UL, // k=9
  4294925233UL, // k=10
  2304563134UL, // k=11
  1804603682UL, // k=12
  4254626195UL, // k=13
  2792965006UL, // k=14
  1236535329UL, // k=15
  4129170786UL, // k=16
  3225465664UL, // k=17
  643717713UL, // k=18
  3921069994UL, // k=19
  3593408605UL, // k=20
  38016083UL, // k=21
  3634488961UL, // k=22
  3889429448UL, // k=23
  568446438UL, // k=24
  3275163606UL, // k=25
  4107603335UL, // k=26
  1163531501UL, // k=27
  2850285829UL, // k=28
  4243563512UL, // k=29
  1735328473UL, // k=30
  2368359562UL, // k=31
  4294588738UL, // k=32
  2272392833UL, // k=33
  1839030562UL, // k=34
  4259657740UL, // k=35
  2763975236UL, // k=36
  1272893353UL, // k=37
  4139469664UL, // k=38
  3200236656UL, // k=39
  681279174UL, // k=40
  3936430074UL, // k=41
  3572445317UL, // k=42
  76029189UL, // k=43
  3654602809UL, // k=44
  3873151461UL, // k=45
  530742520UL, // k=46
  3299628645UL, // k=47
  4096336452UL, // k=48
  1126891415UL, // k=49
  2878612391UL, // k=50
  4237533241UL, // k=51
  1700485571UL, // k=52
  2399980690UL, // k=53
  4293915773UL, // k=54
  2240044497UL, // k=55
  1873313359UL, // k=56
  4264355552UL, // k=57
  2734768916UL, // k=58
  1309151649UL, // k=59
  4149444226UL, // k=60
  3174756917UL, // k=61
  718787259UL, // k=62
  3951481745UL,  // k=63
};

static gcc_constexpr_data uint32_t r[64] = {
  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,
};

static inline uint32_t
leftrotate(uint32_t x, uint32_t c)
{
    return (x << c) | (x >> (32 - c));
}

void
MD5::InitKey(uint32_t h0in, uint32_t h1in, uint32_t h2in, uint32_t h3in)
{
  h0 = h0in;
  h1 = h1in;
  h2 = h2in;
  h3 = h3in;
  message_length = 0;
}

bool
MD5::IsValidIGCChar(char c)
{
  /*
   * Version 1.0.0 first posted to OLC 8/23/2008 supressed 0x0D only, and used Key#1
   * Version 1.0.2 which uses the correct suppression filter from IGC spec and uses key #2
   * This is the version we want to use, but we're reverting to 1.0.0 until OLC can upgrade to 1.0.3
   * Validation program 1.0.3 is backwards compatible and reads either 1.0.0 or 1.0.2
   */

  // 1.0.2 filtering (and use key #2, 3 or 4 b/c key #1 used by 1.0.0 has a dupe in it

  return c >= 0x20 && c <= 0x7E && c != 0x24 &&
         c != 0x2A && c != 0x2C && c != 0x21 && c != 0x5C && c != 0x5E &&
         c != 0x7E;
}

void
MD5::Append(uint8_t ch)
{
  unsigned position = unsigned(message_length++) % ARRAY_SIZE(buff512bits);
  buff512bits[position++] = ch;
  if (position == ARRAY_SIZE(buff512bits))
    Process512(buff512bits);
}

void
MD5::AppendString(const unsigned char *in, bool skip_invalid_igc_chars)
{
  for (; *in != 0; ++in) {
    if (skip_invalid_igc_chars && !IsValidIGCChar(*in))
      continue;

    Append(*in);
  }
}

void
MD5::Finalize()
{
  // append "0" bits until message length in bits ? 448 (mod 512)
  int buffer_left_over = message_length % 64;
  // need at least 64 bits (8 bytes) for length bits at end

  if (buffer_left_over < (64 - 8)) {
    // append "1" bit to end of buffer
    buff512bits[buffer_left_over] = 0x80;

    // pad with 56 - len to get exactly
    std::fill(buff512bits + buffer_left_over + 1,
              buff512bits + ARRAY_SIZE(buff512bits), 0);

    // exactly 64 bits left for message size bits

    // ready to append message length
  } else {
    // >= 56 bits already in buffer

    // append "1" bit to end of buffer
    buff512bits[buffer_left_over] = 0x80;

    // fill buffer w/ 0's and process
    std::fill(buff512bits + buffer_left_over + 1,
              buff512bits + ARRAY_SIZE(buff512bits), 0);

    Process512(buff512bits);

    // now  load 1st 56 bytes of buffer w/ all 0's,
    std::fill(buff512bits, buff512bits + ARRAY_SIZE(buff512bits), 0);

    // ready to append message length
  }

  //append bit length (bit, not byte) of unpadded message as 64-bit little-endian integer to message
  // store 8 bytes of length into last 8 bytes of buffer (little endian least sig bytes first
  *(uint64_t *)(void *)(buff512bits + 56) = ToLE64(message_length * 8);

  Process512(buff512bits);
}

void
MD5::Process512(const uint8_t *s512in)
{
  // assume exactly 512 bytes

  // copy the 64 chars into the 16 uint32_ts
  uint32_t w[16];
  const uint32_t *s512_32 = (const uint32_t *)(const void *)s512in;
  for (int j = 0; j < 16; j++)
    w[j] = ToLE32(s512_32[j]);

  // Initialize hash value for this chunk:
  uint32_t a = h0, b = h1, c = h2, d = h3;

  // Main loop:
  for (int i = 0; i < 64; i++) {
    uint32_t f, g;
    if (i <= 15) {
      f = (b & c) | ((~b) & d);
      g = i;
    } else if (i <= 31) {
      f = (d & b) | ((~d) & c);
      g = (5 * i + 1) % 16;
    } else if (i <= 47) {
      f = b ^ c ^ d;
      g = (3 * i + 5) % 16;
    } else {
      f = c ^ (b | (~d));
      g = (7 * i) % 16;
    }

    uint32_t temp = d;
    d = c;
    c = b;
    b += leftrotate((a + f + k[i] + w[g]), r[i]);
    a = temp;
  }

  // Add this chunk's hash to result so far:
  h0 += a;
  h1 += b;
  h2 += c;
  h3 += d;
}

void
MD5::GetDigest(char *buffer)
{
  sprintf(buffer, "%08x%08x%08x%08x",
          ByteSwap32(h0), ByteSwap32(h1), ByteSwap32(h2), ByteSwap32(h3));
}
