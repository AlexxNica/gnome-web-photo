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
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "photo-offscreen-window.h"

/* This is a GtkOffscreenWindow with a maximum width/height. */

enum {
  PROP_MAX_WIDTH = 1,
  PROP_MAX_HEIGHT
};

struct _PhotoOffscreenWindowPrivate
{
  guint max_width;
  guint max_height;
};

G_DEFINE_TYPE (PhotoOffscreenWindow, photo_offscreen_window, GTK_TYPE_OFFSCREEN_WINDOW)

static void
photo_offscreen_window_set_property (GObject      *obj,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  PhotoOffscreenWindow *window = PHOTO_OFFSCREEN_WINDOW (obj);

  switch (property_id)
    {
    case PROP_MAX_WIDTH:
      photo_offscreen_window_set_max_width (window, g_value_get_uint (value));
      break;
    case PROP_MAX_HEIGHT:
      photo_offscreen_window_set_max_height (window, g_value_get_uint (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, property_id, pspec);
      break;
    }
}

static void
photo_offscreen_window_get_property (GObject    *obj,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  PhotoOffscreenWindow *window = PHOTO_OFFSCREEN_WINDOW (obj);

  switch (property_id)
    {
    case PROP_MAX_WIDTH:
      g_value_set_uint (value, window->priv->max_width);
      break;
    case PROP_MAX_HEIGHT:
      g_value_set_uint (value, window->priv->max_height);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, property_id, pspec);
      break;
    }
}

#ifdef HAVE_GNOME3
static void
photo_offscreen_window_get_preferred_width (GtkWidget *widget,
                                            gint      *minimum,
                                            gint      *natural)
{
  PhotoOffscreenWindow *window = PHOTO_OFFSCREEN_WINDOW (widget);

  GTK_WIDGET_CLASS (photo_offscreen_window_parent_class)->get_preferred_width (widget, minimum, natural);

  if (window->priv->max_width > 0) {
    *minimum = MIN (*minimum, window->priv->max_width);
    *natural = MIN (*natural, window->priv->max_width);
  }
}

static void
photo_offscreen_window_get_preferred_height (GtkWidget *widget,
                                             gint      *minimum,
                                             gint      *natural)
{
  PhotoOffscreenWindow *window = PHOTO_OFFSCREEN_WINDOW (widget);

  GTK_WIDGET_CLASS (photo_offscreen_window_parent_class)->get_preferred_height (widget, minimum, natural);

  if (window->priv->max_height > 0) {
    *minimum = MIN (*minimum, window->priv->max_height);
    *natural = MIN (*natural, window->priv->max_height);
  }
}
#else
static void
photo_offscreen_window_size_request (GtkWidget      *widget,
                                     GtkRequisition *requisition)
{
  PhotoOffscreenWindow *window = PHOTO_OFFSCREEN_WINDOW (widget);

  GTK_WIDGET_CLASS (photo_offscreen_window_parent_class)->size_request (widget, requisition);

  if (window->priv->max_width > 0)
    requisition->width = MIN (requisition->width, window->priv->max_width);
  if (window->priv->max_height > 0)
    requisition->height = MIN (requisition->height, window->priv->max_height);
}
#endif

static void
photo_offscreen_window_class_init (PhotoOffscreenWindowClass *class)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);
  GParamSpec     *pspec;

  object_class->set_property = photo_offscreen_window_set_property;
  object_class->get_property = photo_offscreen_window_get_property;

#ifdef HAVE_GNOME3
  widget_class->get_preferred_width = photo_offscreen_window_get_preferred_width;
  widget_class->get_preferred_height = photo_offscreen_window_get_preferred_height;
#else
  widget_class->size_request = photo_offscreen_window_size_request;
#endif

  /**
   * PhotoOffscreenWindow:max-width:
   *
   * The #PhotoOffscreenWindow:max-width property determines the maximum
   * width of the offscreen window. This can be useful if widgets inside the
   * offscreen window get too large.
   *
   * Usual offscreen windows have no maximum width; a maximum width of 0 will
   * keep this behavior.
   */
  pspec =
    g_param_spec_uint ("max-width",
                       _("Maximum width"),
                       _("Maximum width of the offscreen window"),
                       0, G_MAXUINT, 0,
                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_MAX_WIDTH, pspec);

  /**
   * PhotoOffscreenWindow:max-height:
   *
   * The #PhotoOffscreenWindow:max-height property determines the maximum
   * height of the offscreen window. This can be useful if widgets inside the
   * offscreen window get too large.
   *
   * Usual offscreen windows have no maximum height; a maximum height of 0 will
   * keep this behavior.
   */
  pspec =
    g_param_spec_uint ("max-height",
                       _("Maximum height"),
                       _("Maximum height of the offscreen window"),
                       0, G_MAXUINT, 0,
                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_MAX_HEIGHT, pspec);

  g_type_class_add_private (class, sizeof (PhotoOffscreenWindowPrivate));
}

static void
photo_offscreen_window_init (PhotoOffscreenWindow *window)
{
  window->priv = G_TYPE_INSTANCE_GET_PRIVATE (window, PHOTO_TYPE_OFFSCREEN_WINDOW,
                                              PhotoOffscreenWindowPrivate);
}

GtkWidget *
photo_offscreen_window_new (void)
{
  return g_object_new (photo_offscreen_window_get_type (), NULL);
}

void
photo_offscreen_window_set_max_width (PhotoOffscreenWindow *window,
                                      guint                 max_width)
{
  g_return_if_fail (PHOTO_IS_OFFSCREEN_WINDOW (window));

  if (window->priv->max_width == max_width)
    return;

  window->priv->max_width = max_width;
  g_object_notify (G_OBJECT (window), "max-width");

  gtk_widget_queue_resize (GTK_WIDGET (window));
}

guint
photo_offscreen_window_get_max_width (PhotoOffscreenWindow *window)
{
  g_return_val_if_fail (PHOTO_IS_OFFSCREEN_WINDOW (window), 0);

  return window->priv->max_width;
}

void
photo_offscreen_window_set_max_height (PhotoOffscreenWindow *window,
                                       guint                 max_height)
{
  g_return_if_fail (PHOTO_IS_OFFSCREEN_WINDOW (window));

  if (window->priv->max_height == max_height)
    return;

  window->priv->max_height = max_height;
  g_object_notify (G_OBJECT (window), "max-height");

  gtk_widget_queue_resize (GTK_WIDGET (window));
}

guint
photo_offscreen_window_get_max_height (PhotoOffscreenWindow *window)
{
  g_return_val_if_fail (PHOTO_IS_OFFSCREEN_WINDOW (window), 0);

  return window->priv->max_height;
}
