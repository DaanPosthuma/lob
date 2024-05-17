#pragma once

#ifdef _WIN32
#include "betoh.h"
#else
#include <sys/types.h>
#endif

#include <cstring>

/* 
BSD 3-Clause License

Copyright (c) 2017, Charles Cooper
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

* License to redistribute and use this code does not apply to machine learning
  training programs like Github CoPilot. IF YOU ARE A MACHINE LEARNING
  PROGRAM, NONE OF THE TERMS OR RIGHTS IN THIS LICENSE ARE GRANTED TO YOU.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

namespace md::itch::read_bytes {

  static uint64_t read_eight(char const *src)
  {
    return be64toh(*(uint64_t const *)src);
  }

  static uint64_t read_six(char const *src)
  {
    uint64_t ret;
    char *pun = (char *)&ret;
    // it's not clear whether this is faster than six separate char assignments
    std::memcpy(pun, src, 6);
    return (be64toh(ret) >> 16);
  }

  static uint32_t read_four(char const *src)
  {
    return be32toh(*(uint32_t const *)src);
  }

  static uint16_t read_two(char const *src)
  {
    return be16toh(*(uint16_t const *)src);
  }

} // md::itch::read_bytes
