/*
 *  Copyright (C) 2005 Christian Persch
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is GNOME Web Photo code.
 *
 * The Initial Developer of the Original Code is
 * Christian Persch <chpe@gnome.org>.
 * Portions created by the Initial Developer are Copyright (C) 2004, 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK *****
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
