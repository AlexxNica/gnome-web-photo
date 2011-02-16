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

#ifndef _PHOTO_OFFSCREEN_WINDOW_H_
#define _PHOTO_OFFSCREEN_WINDOW_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PHOTO_TYPE_OFFSCREEN_WINDOW             (photo_offscreen_window_get_type ())
#define PHOTO_OFFSCREEN_WINDOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), PHOTO_TYPE_OFFSCREEN_WINDOW, PhotoOffscreenWindow))
#define PHOTO_OFFSCREEN_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), PHOTO_TYPE_OFFSCREEN_WINDOW, PhotoOffscreenWindowClass))
#define PHOTO_IS_OFFSCREEN_WINDOW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PHOTO_TYPE_OFFSCREEN_WINDOW))
#define PHOTO_IS_OFFSCREEN_WINDOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), PHOTO_TYPE_OFFSCREEN_WINDOW))
#define PHOTO_OFFSCREEN_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), PHOTO_TYPE_OFFSCREEN_WINDOW, PhotoOffscreenWindowClass))

typedef struct _PhotoOffscreenWindow        PhotoOffscreenWindow;
typedef struct _PhotoOffscreenWindowClass   PhotoOffscreenWindowClass;
typedef struct _PhotoOffscreenWindowPrivate PhotoOffscreenWindowPrivate;

struct _PhotoOffscreenWindow
{
  GtkOffscreenWindow parent_object;

  /*< private >*/
  PhotoOffscreenWindowPrivate *priv;
};

struct _PhotoOffscreenWindowClass
{
  GtkOffscreenWindowClass parent_class;
};

GType photo_offscreen_window_get_type (void) G_GNUC_CONST;

GtkWidget *photo_offscreen_window_new (void);

void       photo_offscreen_window_set_max_height (PhotoOffscreenWindow *window,
                                                  guint                 max_height);
guint      photo_offscreen_window_get_max_height (PhotoOffscreenWindow *window);

G_END_DECLS

#endif /* _PHOTO_OFFSCREEN_WINDOW_H_ */
