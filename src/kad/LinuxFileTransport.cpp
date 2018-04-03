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

#include <string.h>
#include <stdio.h>
#include <thread>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/limits.h>
#include <atomic>
#include "Config.h"
#include "PlatformUtils.h"
#include "LinuxFileTransport.h"

namespace kad
{
  const std::string LinuxFileTransport::transportRoot = "transport";


  LinuxFileTransport::LinuxFileTransport(int reliability)
    : ITransport()
    , reliability(reliability)
  {
    std::srand(std::time(nullptr));
  }


  void LinuxFileTransport::Send(ContactPtr target, const void * data, size_t size)
  {
    static std::atomic<uint32_t> packageId{0};

    uint8_t * buffer = new uint8_t[size];
    memcpy(buffer, data, size);

    bool lost = false;

    if (reliability < 100)
    {
      lost = std::rand() % 100 < reliability;
#ifdef DEBUG
      if (lost)
      {
        printf("Oops, package lost\n");
      }
#endif
    }

    std::thread thread = std::thread(
      [target, buffer, size, lost](uint32_t packageId)
      {
        Mkdir(*target);

        bool complete = lost;

        while (!complete)
        {
          const Contact & self = Config::ContactInfo();

          char filename[PATH_MAX];
          sprintf(filename,
            "%s/%08X-%02X/%08X-%02X-%04X",
            LinuxFileTransport::transportRoot.c_str(),
            (unsigned)target->addr, (unsigned)target->port,
            (unsigned)self.addr, (unsigned)self.port,
            packageId
          );

          int file = open(filename, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, 0755);
          if (file != -1)
          {
            if (flock(file, LOCK_EX | LOCK_NB) == 0)
            {
              UNUSED_RESULT(write(file, &size, sizeof(size)));
              UNUSED_RESULT(write(file, buffer, size));

              complete = true;
            }

            close(file);
          }

          if (!complete)
          {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
          }
        }
      },
      packageId++
    );

    thread.detach();
  }


  ContactPtr LinuxFileTransport::Receive(uint8_t ** buffer, size_t * len)
  {
    ContactPtr result = nullptr;

    while (!result)
    {
      const Contact & self = Config::ContactInfo();

      Mkdir(self);

      char dirname[PATH_MAX];

      sprintf(dirname, "%s/%08X-%02X", LinuxFileTransport::transportRoot.c_str(), (unsigned)self.addr, (unsigned)self.port);

      DIR * dp = opendir(dirname);

      if (dp)
      {
        struct dirent * entry = nullptr;

        while ((entry = readdir(dp)) != nullptr && !result)
        {
          if (strcmp(entry->d_name, ".")  != 0 && strcmp(entry->d_name, "..") != 0)
          {
            unsigned int addr = 0;
            unsigned int port = 0;
            uint32_t packageId = 0;

            if (sscanf(entry->d_name, "%08X-%02X-%04X", &addr, &port, &packageId) != 3)
            {
              continue;
            }

            char filename[PATH_MAX];

            sprintf(filename, "%s/%s", dirname, entry->d_name);

            int fd = open(filename, O_RDONLY | O_CLOEXEC);

            if (fd != -1)
            {
              if (flock(fd, LOCK_EX | LOCK_NB) == 0)
              {
                if (read(fd, len, sizeof(size_t)) == sizeof(size_t))
                {
                  *buffer = new uint8_t[*len];
                  if (read(fd, *buffer, *len) == static_cast<ssize_t>(*len))
                  {
                    result = std::make_shared<Contact>();
                    result->addr = static_cast<long>(addr);
                    result->port = static_cast<short>(port);
                  }
                  else
                  {
                    delete[] (*buffer);
                    *buffer = nullptr;
                    *len = 0;
                  }
                }
              }

              close(fd);

              if (result)
              {
                unlink(filename);
              }
            }
          }
        } 

        closedir(dp);
      }

      if (!result)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
    }

    return result;
  }


  void LinuxFileTransport::Mkdir(const Contact & contact)
  {
    mkdir(LinuxFileTransport::transportRoot.c_str(), 0775);

    char name[PATH_MAX];

    sprintf(name, "%s/%08X-%02X", LinuxFileTransport::transportRoot.c_str(), (unsigned)contact.addr, (unsigned)contact.port);

    mkdir(name, 0775);
  }
}