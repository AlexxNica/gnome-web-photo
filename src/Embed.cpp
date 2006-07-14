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

#include "mozilla-config.h"
#include "config.h"

#include "Embed.h"
#include "Listener.h"

typedef enum
{
  STATE_ONLOAD   = 1 << 0,
  STATE_NETSTOP  = 1 << 1,
  STATE_READY    = 1 << 2,
} EmbedState;

#define EMBED_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_EMBED, EmbedPrivate))

struct _EmbedPrivate
{
  Listener *listener;
  guint state : 16;
  guint cleaning : 1;
};

enum
{
  LOAD,
  READY,
  PRINT_DONE,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

G_DEFINE_TYPE (Embed, embed, GTK_TYPE_MOZ_EMBED);

static void
check_state (Embed *embed)
{
  EmbedPrivate *priv = embed->priv;

  if (priv->state == (STATE_ONLOAD | STATE_NETSTOP)) {
    priv->state = STATE_READY;

    g_signal_emit (embed, signals[READY], 0);
  }
}

static void
embed_net_stop (GtkMozEmbed *mozembed)
{
  Embed *embed = EMBED (mozembed);
  EmbedPrivate *priv = embed->priv;

  priv->state |= STATE_NETSTOP;
  check_state (embed);
}

static void
embed_onload (Embed *embed)
{
  EmbedPrivate *priv = embed->priv;

  priv->state |= STATE_ONLOAD;
  check_state (embed);
}

static void
embed_realize (GtkWidget *widget)
{
  GtkMozEmbed *mozembed = GTK_MOZ_EMBED (widget);
  Embed *embed = EMBED (widget);
  EmbedPrivate *priv = embed->priv;

  GTK_WIDGET_CLASS (embed_parent_class)->realize (widget);

  priv->listener = new Listener (mozembed);
  if (NS_FAILED (priv->listener->Attach ())) {
    g_warning ("Couldn't attach the listener!\n");
  }
}

static void
embed_init (Embed *embed)
{
  embed->priv = EMBED_GET_PRIVATE (embed);
}

static void
embed_class_init (EmbedClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkMozEmbedClass *moz_embed_class = GTK_MOZ_EMBED_CLASS (klass);

  widget_class->realize = embed_realize;

  moz_embed_class->net_stop = embed_net_stop;

  klass->onload = embed_onload;

  signals[LOAD] =
    g_signal_new ("onload",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (EmbedClass, onload),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  signals[READY] =
    g_signal_new ("ready",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (EmbedClass, ready),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  signals[PRINT_DONE] =
    g_signal_new ("print_done",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (EmbedClass, ready),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

  g_type_class_add_private (klass, sizeof (EmbedPrivate));
}

/* Public API */

void
embed_load (Embed *embed,
            const char *uri)
{
  EmbedPrivate *priv = embed->priv;

  priv->state = 0;
  gtk_moz_embed_load_url (GTK_MOZ_EMBED (embed), uri);
}

void
embed_stop (Embed *embed)
{
  gtk_moz_embed_stop_load (GTK_MOZ_EMBED (embed));
}
