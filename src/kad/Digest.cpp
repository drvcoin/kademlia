/**
 *
 * MIT License
 *
 * Copyright (c) 2018 drvcoin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * =============================================================================
 */

#include <drive/kad/Digest.h>

#include <string.h>
#include <stdio.h>

namespace kad
{
  Digest::Digest()
  {
    SHA1_Init(&sha);
  }

  bool Digest::Update(const void * buffer, size_t size)
  {
    SHA1_Update(&sha, buffer, size);

    return true;
  }

  bool Digest::Finish(sha1_t digest)
  {
    SHA1_Final(digest, &sha);

    return true;
  }

  bool Digest::Compute(const void * buffer, size_t size, sha1_t digest)
  {
    SHA1((unsigned char *)buffer, size, digest);

    return true;
  }

  bool Digest::Compare(const void * buffer, size_t size, sha1_t digest)
  {
    sha1_t computed;

    SHA1((unsigned char *)buffer, size, computed);

    return memcmp(computed, digest, sizeof(computed)) == 0;
  }

  std::string Digest::ToString(sha1_t digest)
  {
    const int n = sizeof(sha1_t);
    char buffer[2*n+1];
    for (size_t i = 0; i < n; i++)
    {
      sprintf(&buffer[i*2], "%02X", digest[i]);
    }
    buffer[2*n] = '\0';
    return std::string(buffer);
  }

  void Digest::Print(sha1_t digest)
  {
    for (size_t i = 0; i < sizeof(sha1_t); i++)
    {
      printf("%02X", digest[i]);
    }
    printf("\n");
  }
}
