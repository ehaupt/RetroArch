/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2015 - Daniel De Matteis
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#include <boolean.h>

#include "../configuration.h"

bool menu_navigation_ctl(enum menu_navigation_ctl_state state, void *data)
{
   const menu_ctx_driver_t *driver = menu_ctx_driver_get_ptr();
   settings_t          *settings   = config_get_ptr();
   menu_navigation_t        *nav   = menu_navigation_get_ptr();
   menu_list_t          *menu_list = menu_list_get_ptr();

   (void)menu_list;
   (void)settings;

   switch (state)
   {
      case MENU_NAVIGATION_CTL_CLEAR:
         {
            size_t idx         = 0;
            bool scroll        = true;
            bool *pending_push = (bool*)data;

            if (!pending_push)
               return false;

            menu_navigation_ctl(MENU_NAVIGATION_CTL_SET_SELECTION, &idx);
            menu_navigation_ctl(MENU_NAVIGATION_CTL_SET, &scroll);
            if (driver->navigation_clear)
               driver->navigation_clear(*pending_push);
         }
         return true;
      case MENU_NAVIGATION_CTL_INCREMENT:
         {
            size_t selection       = nav->selection_ptr;
            unsigned *scroll_speed = (unsigned*)data;

            if (!scroll_speed)
               return false;

            if ((selection + (*scroll_speed)) < (menu_list_get_size(menu_list)))
            {
               size_t idx  = selection + (*scroll_speed);
               bool scroll = true;
               menu_navigation_ctl(MENU_NAVIGATION_CTL_SET_SELECTION, &idx);
               menu_navigation_ctl(MENU_NAVIGATION_CTL_SET, &scroll);
               menu_navigation_ctl(MENU_NAVIGATION_CTL_INCREMENT, NULL);
            }
            else
            {
               if (settings->menu.navigation.wraparound.vertical_enable)
               {
                  bool pending_push = false;
                  menu_navigation_ctl(MENU_NAVIGATION_CTL_CLEAR, &pending_push);
               }
               else
               {
                  if ((menu_list_get_size(menu_list) > 0))
                  {
                     menu_navigation_ctl(MENU_NAVIGATION_CTL_SET_LAST,  NULL);
                     menu_navigation_ctl(MENU_NAVIGATION_CTL_INCREMENT, NULL);
                  }
               }
            }
            if (driver->navigation_increment)
               driver->navigation_increment();
         }
         return true;
      case MENU_NAVIGATION_CTL_DECREMENT:
         {
            size_t idx             = 0;
            bool scroll            = true;
            size_t selection       = nav->selection_ptr;
            unsigned *scroll_speed = (unsigned*)data;

            if (!scroll_speed)
               return false;

            if (selection >= *scroll_speed)
               idx = selection - *scroll_speed;
            else
            {
               idx  = menu_list_get_size(menu_list) - 1;
               if (!settings->menu.navigation.wraparound.vertical_enable)
                  idx = 0;
            }
            menu_navigation_ctl(MENU_NAVIGATION_CTL_SET_SELECTION, &idx);
            menu_navigation_ctl(MENU_NAVIGATION_CTL_SET, &scroll);
            menu_navigation_ctl(MENU_NAVIGATION_CTL_DECREMENT, NULL);

            if (driver->navigation_decrement)
               driver->navigation_decrement();
         }
         return true;
      case MENU_NAVIGATION_CTL_SET:
         {
            bool *scroll = (bool*)data;

            if (!scroll)
               return false;

            if (driver->navigation_set)
               driver->navigation_set(*scroll);
         }
         return true;
      case MENU_NAVIGATION_CTL_SET_LAST:
         nav->selection_ptr = menu_list_get_size(menu_list) - 1;
         if (driver->navigation_set_last)
            driver->navigation_set_last();
         return true;
      case MENU_NAVIGATION_CTL_ASCEND_ALPHABET:
         {
            size_t *ptr_out = (size_t*)data;
            size_t i = 0, ptr = *ptr_out;
            if (!nav || !nav->scroll.indices.size || !ptr_out)
               return false;
            if (ptr == nav->scroll.indices.list[nav->scroll.indices.size - 1])
               return false;

            while (i < nav->scroll.indices.size - 1
                  && nav->scroll.indices.list[i + 1] <= ptr)
               i++;
            *ptr_out = nav->scroll.indices.list[i + 1];

            if (driver->navigation_ascend_alphabet)
               driver->navigation_ascend_alphabet(ptr_out);
         }
         return true;
      case MENU_NAVIGATION_CTL_DESCEND_ALPHABET:
         {
            size_t *ptr_out = (size_t*)data;
            size_t i, ptr = *ptr_out;

            if (!nav || !nav->scroll.indices.size || ptr == 0 || !ptr_out)
               return false;

            i = nav->scroll.indices.size - 1;

            while (i && nav->scroll.indices.list[i - 1] >= ptr)
               i--;
            *ptr_out = nav->scroll.indices.list[i - 1];

            if (driver->navigation_descend_alphabet)
               driver->navigation_descend_alphabet(ptr_out);
         }
         return true;
      case MENU_NAVIGATION_CTL_GET_SELECTION:
         {
            size_t *sel = (size_t*)data;
            if (!nav || !sel)
               return false;
            *sel = nav->selection_ptr;
         }
         return true;
      case MENU_NAVIGATION_CTL_SET_SELECTION:
         {
            size_t *sel = (size_t*)data;
            if (!nav || !sel)
               return false;
            nav->selection_ptr = *sel;
         }
         return true;
      case MENU_NAVIGATION_CTL_CLEAR_SCROLL_INDICES:
         {
            if (!nav)
               return false;
            nav->scroll.indices.size = 0;
         }
         return true;
      case MENU_NAVIGATION_CTL_ADD_SCROLL_INDEX:
         {
            size_t *sel = (size_t*)data;
            if (!nav || !sel)
               return false;
            nav->scroll.indices.list[nav->scroll.indices.size++] = *sel;
         }
         return true;
   }

   return false;
}
