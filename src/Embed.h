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
