/*
 * Copyright (C) 2011 Novell, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authors:
 *   Vincent Untz <vuntz@gnome.org>
 */

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "photo-offscreen-window.h"

/* Value to keep in sync with the one from cairo:
 * MAX_IMAGE_SIZE in cairo-image-surface.c */
#define MAX_SIZE        32767

/* This is a GtkOffscreenWindow with a maximum height.
 * See comment above gtk_widget_set_size_request() call to understand why we
 * need this. */

G_DEFINE_TYPE (PhotoOffscreenWindow, photo_offscreen_window, GTK_TYPE_OFFSCREEN_WINDOW)

#ifdef HAVE_GNOME3
static void
photo_offscreen_window_get_preferred_height (GtkWidget *widget,
                                             gint      *minimum,
                                             gint      *natural)
{
  GTK_WIDGET_CLASS (photo_offscreen_window_parent_class)->get_preferred_height (widget, minimum, natural);

  *minimum = MIN (*minimum, MAX_SIZE);
  *natural = MIN (*natural, MAX_SIZE);
}
#else
static void
photo_offscreen_window_size_request (GtkWidget      *widget,
                                     GtkRequisition *requisition)
{
  GTK_WIDGET_CLASS (photo_offscreen_window_parent_class)->size_request (widget, requisition);

  requisition->height = MIN (requisition->height, MAX_SIZE);
}
#endif

static void
photo_offscreen_window_class_init (PhotoOffscreenWindowClass *class)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

#ifdef HAVE_GNOME3
  widget_class->get_preferred_height = photo_offscreen_window_get_preferred_height;
#else
  widget_class->size_request = photo_offscreen_window_size_request;
#endif
}

static void
photo_offscreen_window_init (PhotoOffscreenWindow *window)
{
}

GtkWidget *
photo_offscreen_window_new (void)
{
  return g_object_new (photo_offscreen_window_get_type (), NULL);
}
