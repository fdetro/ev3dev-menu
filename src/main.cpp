/*
 * ev3dev configuration and application launcher GUI
 *
 * Copyright (c) 2014 - Franz Detro
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <ev3dev.h>
#include <u8g.h>

#include <iostream>
#include <fstream>
#include <thread>
#include <cstring>

#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#define KEY_RELEASE 0
#define KEY_PRESS   1
#define KEY_REPEAT  2

using namespace std;

#define ROW(x) (10+(x+1)*fh)
#define COL(x) ((x)*fw)

u8g_uint_t w, h, fw, fh, fd;
u8g_t lcd;

void draw()
{
  u8g_BeginDraw(&lcd);
  u8g_SetColorIndex(&lcd, 1);

  u8g_DrawFrame(&lcd, 0, 0, w, h);

  u8g_DrawStr(&lcd, COL(0), ROW(3), "Voltage:");
  u8g_DrawStr(&lcd, COL(0), ROW(4), "Current:");

  ifstream is("/etc/hostname");
  if (is.is_open())
  {
    string hostname;
    getline(is, hostname);
    is.close();
    u8g_DrawStr(&lcd, COL(0), ROW(0), hostname.c_str());
  }

  char buf[256];

  ifaddrs *if_iter = nullptr;
  if (getifaddrs(&if_iter) == 0)
  {
    ifaddrs *i = if_iter;
    while (i)
    {
      if (i->ifa_addr->sa_family == AF_INET)
      {
        sockaddr_in *a = reinterpret_cast<sockaddr_in*>(i->ifa_addr);
        inet_ntop(AF_INET, &(a->sin_addr), buf, INET_ADDRSTRLEN);        
        if ( strcmp(i->ifa_name, "usb0") == 0)
        {
          u8g_DrawStr(&lcd, COL(1), ROW(1), buf);
        }
        else if (strcmp(i->ifa_name, "wlan0") == 0)
        {
          u8g_DrawStr(&lcd, COL(1), ROW(2), buf);
        }
      }
      i = i->ifa_next;
    }
    freeifaddrs(if_iter);
  }

  sprintf(buf, "%4.1f V", ev3dev::battery::voltage());
  u8g_DrawStr(&lcd, COL(9), ROW(3), buf);
  sprintf(buf, "%4.0f mA", ev3dev::battery::current());
  u8g_DrawStr(&lcd, COL(9), ROW(4), buf);

  u8g_EndDraw(&lcd);    
}

int main()
{
  u8g_Init(&lcd, &u8g_dev_linux_fb);
  
  w = u8g_GetWidth (&lcd);
  h = u8g_GetHeight(&lcd);

  u8g_SetFont(&lcd, u8g_font_10x20);
  
  fw = u8g_GetFontBBXWidth (&lcd);
  fh = u8g_GetFontBBXHeight(&lcd);
  fd = u8g_GetFontDescent  (&lcd);

  bool terminate = false;
  
  thread t([&] () {
    int fd = open("/dev/input/by-path/platform-gpio-keys.0-event", O_RDONLY);
    if (fd  < 0)
    {
      cout << "Couldn't open platform-gpio-keys device!" << endl;
      return;
    }

    input_event ev;
    while (true)
    {
      size_t rb = read(fd, &ev, sizeof(ev));

      if (rb < sizeof(input_event))
        continue;

      if ((ev.type == EV_KEY) /*&& (ev.value == KEY_PRESS)*/)
      {
        terminate = true;
        return;
      }
    }
  });
  t.detach();
  
  int i = 0;
  while (!terminate)
  {
    if (i%10==0)
    {
      draw();
    }
    this_thread::sleep_for(chrono::milliseconds(100));
    ++i;
  }
  
  u8g_Stop(&lcd);

  return 0;
}
