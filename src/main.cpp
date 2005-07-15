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

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtkmozembed.h>
#include <nsCOMPtr.h>
#include <nsIPrefService.h>
#include <nsIServiceManager.h>
#include <nsIStyleSheetService.h>
#include <nsILocalFile.h>
#include <nsIURI.h>
#include <nsNetUtil.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "Components.h"
#include "Listener.h"
#include "Writer.h"

#define MIN_WIDTH	64
#define THUMBNAIL_WIDTH	1024
#define MAX_WIDTH	2048
#define HEIGHT		64
#define TIMEOUT		60 * 1000 /* 60 seconds */

#define DEFAULT_THUMBNAIL_SIZE	256
#define DEFAULT_WIDTH		1024

/* no need to depend on libgnome just for this define */
#define GNOME_DOT_GNOME         ".gnome2"

#ifdef GNOME_ENABLE_DEBUG
#define LOG g_print
#else
#define LOG //
#endif

/* --- */

static char **arguments = NULL;
static int width = -1;
static int size = -1;
static gboolean thumbnail = FALSE;
static char *filename;
static int retval = 1;

static GOptionEntry entries [] =
{
#ifdef THUMBNAILER
  { "size", 's', 0, G_OPTION_ARG_INT, &size, N_("The thumbnail size (default: 256)"), "S" },
#else
  { "width", 'w', 0, G_OPTION_ARG_INT, &width, N_("The desired width of the image (default: 1024)"), "W" },
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

  Writer *writer;
  if (thumbnail) {
    writer = new ThumbnailWriter (mozembed, filename, size);
  } else {
    writer = new PNGWriter (mozembed, filename);
  }
  if (!writer) return FALSE;

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
  if (NS_FAILED (embed->listener->Attach ())) {
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
  /* BUG ALERT! If we don't have a profile, Gecko will crash on https sites and
   * when trying to open the password manager. The prefs will be set up so that
   * no cookies or passwords etc. will be persisted.
   */
  char *profile;
  profile = g_build_filename (g_get_home_dir (), GNOME_DOT_GNOME, NULL);
  gtk_moz_embed_set_profile_path (profile, "gnome-web-photo");
  g_free (profile);

  gtk_moz_embed_set_comp_path (MOZILLA_HOME);
  gtk_moz_embed_push_startup ();

  RegisterComponents ();

  nsresult rv;
  nsCOMPtr<nsIPrefService> prefService (do_GetService (NS_PREFSERVICE_CONTRACTID, &rv));
  NS_ENSURE_SUCCESS (rv, rv);

  /* read our predefined default prefs */
  nsCOMPtr<nsILocalFile> prefsFile;
  rv = NS_NewNativeLocalFile(nsDependentCString(SHARE_DIR "/prefs.js"),
			     PR_TRUE, getter_AddRefs(prefsFile));
  NS_ENSURE_SUCCESS (rv, rv);
  
  rv = prefService->ReadUserPrefs(prefsFile);
  NS_ENSURE_SUCCESS (rv, rv);

  nsCOMPtr<nsIStyleSheetService> sheetService (do_GetService ("@mozilla.org/content/style-sheet-service;1", &rv));
  NS_ENSURE_SUCCESS (rv, rv);

  nsCOMPtr<nsILocalFile> styleFile;
  rv = NS_NewNativeLocalFile(nsDependentCString(SHARE_DIR "/style.css"),
                             PR_TRUE, getter_AddRefs(styleFile));
  NS_ENSURE_SUCCESS (rv, rv);

  nsCOMPtr<nsIFile> file (do_QueryInterface (styleFile, &rv));
  NS_ENSURE_SUCCESS (rv, rv);
  
  nsCOMPtr<nsIURI> styleURI;
  rv = NS_NewFileURI (getter_AddRefs (styleURI), styleFile);
  NS_ENSURE_SUCCESS (rv, rv);

  rv = sheetService->LoadAndRegisterSheet (styleURI, nsIStyleSheetService::AGENT_SHEET);
  NS_ENSURE_SUCCESS (rv, rv);

  return rv;
}

/* --- */

static void
synopsis (void)
{
#ifdef THUMBNAILER
  g_print (_("Usage: %s [-s size] URL outfile\n"), g_get_prgname ());
#else
  g_print (_("Usage: %s [-w width] URL outfile\n"), g_get_prgname ());
#endif
  exit (1);
}

int
main (int argc, char **argv)
{
  GtkWidget *window, *embed;
  GdkScreen *screen;
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
    size = DEFAULT_THUMBNAIL_SIZE;
  }
  if (size != 32 && size != 64 && size != 128 && size != 256) {
    g_print ("Thumbnail size has to be 32, 64, 128 or 256!\n");
  }

  width = THUMBNAIL_WIDTH;
#else
  if (width == -1) {
    width = DEFAULT_WIDTH;
  }
  if (width < MIN_WIDTH || width > MAX_WIDTH) {
    g_print ("Width must be between %d and %d!\n", MIN_WIDTH, MAX_WIDTH);
    return 1;
  }
#endif

  if (arguments == NULL || g_strv_length (arguments) != 2) {
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

  /* Move the window off screen */
  screen = gtk_widget_get_screen (GTK_WIDGET (window));
  gtk_window_move (GTK_WINDOW (window),
		   gdk_screen_get_width (screen) + 100,
		   gdk_screen_get_height (screen) + 100);
  gtk_widget_show_all (window);
  gdk_window_hide (window->window);

  gtk_moz_embed_load_url (GTK_MOZ_EMBED (embed), url);

  /* FIXME is there a way to guarantee a kill after TIMEOUT secs? */
  g_timeout_add (TIMEOUT, (GSourceFunc) gtk_widget_destroy, window);

  gtk_main ();

  return retval;
}
