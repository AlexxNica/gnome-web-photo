/*
 *  Copyright (C) 2005 Christian Persch
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#include "mozilla-config.h"
#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtkmozembed.h>
#include <nsCOMPtr.h>
#include <nsIPrefService.h>
#include <nsIServiceManager.h>
#include <nsILocalFile.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "Listener.h"
#include "Writer.h"

#define MIN_WIDTH	64
#define DEFAULT_SIZE	1024
#define THUMBNAILERSIZE 1024
#define MAX_WIDTH	2048
#define THUMB_SIZE_MAX	256
#define HEIGHT		64
#define TIMEOUT		60 * 1000 /* 60 seconds */

#ifdef GNOME_ENABLE_DEBUG
#define LOG g_print
#else
#define LOG //
#endif

/* --- */

enum
{
  PNG
#ifdef ENABLE_JPEG
  , JPEG
#endif
};

static char **arguments = NULL;
static int width = -1;
static int size = -1;
static gboolean thumbnail = FALSE;
static char *filename;
static int retval = 1;
static char *type_string = NULL;
static int type = PNG;
static int quality = 100;
static guint timeout;

static GOptionEntry entries [] =
{
#ifdef THUMBNAILER
  { "size", 's', 0, G_OPTION_ARG_INT, &size, N_("The thumbnail size"), "S" },
#else
  { "width", 'w', 0, G_OPTION_ARG_INT, &width, N_("The desired width of the image"), "W" },
  { "thumbnail", 't', 0, G_OPTION_ARG_NONE, &thumbnail, N_("If given, writes a thumbnail instead of a full image"), NULL },
#endif
#if defined(ENABLE_JPEG) && !defined(THUMBNAILER)
  { "quality", 'q', 0, G_OPTION_ARG_INT, &quality, N_("The desired quality of the JPEG image (1 .. 100)"), "Q" },
  { "type", '\0', 0, G_OPTION_ARG_STRING, &type_string, N_("Which image type to write (PNG, JPEG)"), "T" },
#endif
  { G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME_ARRAY, &arguments, "", NULL },
  { NULL }
};

/* --- */

#define TYPE_EMBED (embed_get_type ())
#define EMBED(o)   (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_EMBED, Embed))

GType embed_get_type (void);

typedef struct _Embed      Embed;
typedef struct _EmbedClass EmbedClass;

struct _Embed
{
  GtkMozEmbed parent_instance;
  Listener *listener;
  guint state;
};

struct _EmbedClass
{
  GtkMozEmbedClass parent_class;
  void (* onload) (Embed *embed);
};

enum
{
  LOAD,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };
static GObjectClass *parent_class = NULL;

static gboolean
embed_take_picture (Embed *embed)
{
  GtkMozEmbed *mozembed = GTK_MOZ_EMBED (embed);

  /* don't need this anymore */
  g_source_remove (timeout);

  Writer *writer = NULL;
  if (thumbnail) {
    writer = new ThumbnailWriter (mozembed, filename, size);
  } else if (type == PNG) {
    writer = new PNGWriter (mozembed, filename);
  } else if (type == JPEG) {
    writer = new JPEGWriter (mozembed, filename, quality);
  }
  g_return_val_if_fail (writer, FALSE);

  retval = writer->Write() != PR_TRUE;
  delete writer;

  g_idle_add ((GSourceFunc) gtk_widget_destroy,
	      gtk_widget_get_toplevel (GTK_WIDGET (embed)));

  /* don't run again */
  return FALSE;
}

static void
check_state (Embed *embed)
{
  if (embed->state == 3)
    {
      embed->state = 4;
      g_idle_add_full (G_PRIORITY_LOW, (GSourceFunc) embed_take_picture, embed, NULL);
    }
}

static void
embed_net_stop (GtkMozEmbed *mozembed)
{
  Embed *embed = EMBED (mozembed);

  embed->state |= 1;
  check_state (embed);
}

static void
embed_onload (Embed *embed)
{
  embed->state |= 2;
  check_state (embed);
}

static void
embed_realize (GtkWidget *widget)
{
  GtkMozEmbed *mozembed = GTK_MOZ_EMBED (widget);
  Embed *embed = EMBED (widget);

  GTK_WIDGET_CLASS (parent_class)->realize (widget);

  embed->listener = new Listener(mozembed);
  if (NS_FAILED (embed->listener->Attach ()))
    {
      g_warning ("Couldn't attach the listener!\n");
    }
}

static void
embed_init (Embed *embed)
{
}

static void
embed_class_init (EmbedClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkMozEmbedClass *moz_embed_class = GTK_MOZ_EMBED_CLASS (klass);

    parent_class = (GObjectClass *) g_type_class_peek_parent (klass);

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
}

G_DEFINE_TYPE (Embed, embed, GTK_TYPE_MOZ_EMBED);

/* --- */

static nsresult
init_gecko (void)
{
  gtk_moz_embed_set_comp_path (MOZILLA_HOME);
  gtk_moz_embed_push_startup ();

  nsresult rv;
  nsCOMPtr<nsIPrefService> prefService (do_GetService (NS_PREFSERVICE_CONTRACTID, &rv));
  NS_ENSURE_SUCCESS (rv, rv);

  /* read our predefined default prefs */
  nsCOMPtr<nsILocalFile> file;
  rv = NS_NewNativeLocalFile(nsDependentCString(SHARE_DIR "/prefs.js"),
			     PR_TRUE, getter_AddRefs(file));
  NS_ENSURE_SUCCESS (rv, rv);
  
  rv = prefService->ReadUserPrefs(file);
  NS_ENSURE_SUCCESS (rv, rv);

  return rv;
}

/* --- */

static void
synopsis (void)
{
#ifdef THUMBNAILER
  g_print (_("Usage: %s [--size=N] URL outfile\n"), g_get_prgname ());
#elif defined(ENABLE_JPEG)
  g_print (_("Usage: %s [--with=N] [--type=png|jpeg] [--quality=Q] URL outfile\n"), g_get_prgname ());
#else
  g_print (_("Usage: %s [--width=N] URL outfile\n"), g_get_prgname ());
#endif
  exit (1);
}

int
main (int argc, char **argv)
{
  GtkWidget *window, *embed;
  GError *error = NULL;
  char *url;

#ifdef ENABLE_NLS
  /* Initialize the i18n stuff */
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  if (!gtk_init_with_args (&argc, &argv, NULL, entries, GETTEXT_PACKAGE, &error)) {
    synopsis ();
  }

#ifdef THUMBNAILER
  thumbnail = TRUE;

  if (size == -1) {
    synopsis ();
  }
  if (size != 32 && size != 64 && size != 128 && size != 256) {
    g_print ("Thumbnail size has to be 32, 64, 128 or 256!\n");
    return 1;
  }

  width = THUMBNAILERSIZE;
#else
  if (width == -1) {
    synopsis ();
  }
  if (width < MIN_WIDTH || width > MAX_WIDTH) {
    g_print ("Width must be between %d and %d!\n", MIN_WIDTH, MAX_WIDTH);
    return 1;
  }
#endif

#ifdef ENABLE_JPEG
  if (!type_string || g_ascii_strcasecmp (type_string, "PNG") == 0) {
    type = PNG;
  }
  else if (g_ascii_strcasecmp (type_string, "JPEG") == 0) {
    type = JPEG;
  } else {
    g_print ("Unknown image type '%s' specified!\n", type_string);
    return 1;
  }

  if (quality < 1 || quality > 100) {
    g_print ("Quality has to be between 1 and 100!\n");
    return 1;
  }
#endif

  if (g_strv_length (arguments) != 2) {
    synopsis ();
  }

  url = g_filename_to_utf8 (arguments[0], -1, NULL, NULL, NULL);
  filename = arguments[1];

  if (url == NULL) {
    g_print ("Could not convert URL to UTF-8!\n");
    return 1;
  }

  /* Initialised gecko */
  if (NS_FAILED (init_gecko ())) {
    g_print ("Failed to initialise gecko!\n");
    return 1;
  };

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  embed = GTK_WIDGET (g_object_new (TYPE_EMBED, NULL));
  gtk_widget_set_size_request (embed, width, HEIGHT);
  gtk_container_add (GTK_CONTAINER (window), embed);

  gtk_widget_show_all (window);
  gdk_window_hide (window->window);

  gtk_moz_embed_load_url (GTK_MOZ_EMBED (embed), url);

  /* FIXME is there a way to guarantee a kill after TIMEOUT secs? */
  timeout = g_timeout_add (TIMEOUT, (GSourceFunc) gtk_widget_destroy, window);

  gtk_main ();

  LOG ("Retval: %d\n", retval);
  return retval;
}
