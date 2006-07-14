/*
 *  Copyright (C) 2005 Christian Persch
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#ifndef EMBED_H
#define EMBED_H

#include <glib-object.h>
#include <gtkmozembed.h>

G_BEGIN_DECLS

#define TYPE_EMBED        (embed_get_type ())
#define EMBED(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_EMBED, Embed))
#define EMBED_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), TYPE_EMBED, EmbedClass))
#define IS_EMBED(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_EMBED))
#define IS_EMBED_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_EMBED))
#define EMBED_GET_CLASS(o)(G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_EMBED, EmbedClass))

typedef struct _Embed        Embed;
typedef struct _EmbedPrivate EmbedPrivate;
typedef struct _EmbedClass   EmbedClass;

struct _Embed
{
  GtkMozEmbed parent_instance;
  /*< private >*/
  EmbedPrivate *priv;
};

struct _EmbedClass
{
  GtkMozEmbedClass parent_class;
  void (* onload)     (Embed *embed);
  void (* ready)      (Embed *embed);
  void (* print_done) (Embed *embed,
                       gboolean success);
};

GType embed_get_type (void);

void  embed_load     (Embed *embed,
                      const char *uri);

void  embed_stop     (Embed *embed);

G_END_DECLS

#endif
