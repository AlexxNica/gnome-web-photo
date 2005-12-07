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
#include "Prefs.h"
#include "Printer.h"
#include "Writer.h"

#define MIN_WIDTH	64
#define THUMBNAIL_WIDTH	1024
#define MAX_WIDTH	2048
#define HEIGHT		64

#define DEFAULT_THUMBNAIL_SIZE	256
#define DEFAULT_WIDTH		1024

/* no need to depend on libgnome just for this define */
#define GNOME_DOT_GNOME         ".gnome2"

#ifdef GNOME_ENABLE_DEBUG
#define LOG g_print
#else
#define LOG //
#endif

// #define DEBUG_SHOW_WINDOW

enum
{
  MODE_PHOTO,
  MODE_THUMBNAIL,
  MODE_PRINT,
  MODE_LAST,
  MODE_INVALID = MODE_LAST
};

enum
{
  FORMAT_PNG,
  FORMAT_PPM,
  FORMAT_LAST,
  FORMAT_INVALID = FORMAT_LAST
};

static const char *modes[] = { "photo", "thumbnail", "print" };
static const char *formats[] = { "png", "ppm" };

/* --- */

static int mode = MODE_INVALID;
static int timeout = 60;
static gboolean force = FALSE;
static int width = -1;
static int size = -1;
static int format = FORMAT_PNG;
static gboolean print_background = FALSE;
static char *url = NULL;
static char *infile = NULL;
static char *outfile = NULL;
static int retval = 1;

static gboolean
parse_mode (const gchar *option_name,
            const gchar *value,
            gpointer data,
            GError **error)
{
  g_assert (value != NULL);

  guint i;
  for (i = 0; i < MODE_LAST; ++i) {
    if (g_ascii_strcasecmp (value, modes[i]) == 0) {
      mode = i;
      break;
    }
  }

  if (mode == MODE_INVALID) {
    *error = g_error_new (G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
                          _("Unknown mode '%s'"), value);
    return FALSE;
  }

  return TRUE;
}

static gboolean
parse_format (const gchar *option_name,
              const gchar *value,
              gpointer data,
              GError **error)
{
  g_assert (value != NULL);

  guint i;
  for (i = 0; i < FORMAT_LAST; ++i) {
    if (g_ascii_strcasecmp (value, formats[i]) == 0) {
      format = i;
      break;
    }
  }

  if (format == FORMAT_INVALID) {
    *error = g_error_new (G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
                          _("Unknown format '%s'"), value);
    return FALSE;
  }

  return TRUE;
}

static GOptionEntry entries [] =
{
  { "mode", 'm', 0, G_OPTION_ARG_CALLBACK, (void*) parse_mode, N_("Operation mode [photo|thumbnail|print]"), NULL },
  { "timeout", 't', 0, G_OPTION_ARG_INT, &timeout, N_("The timeout in seconds (default: 60)"), "T" },
  { "force", 0, 0, G_OPTION_ARG_NONE, &force, N_("Force output when timeout expires, even if the page isn't loaded fully"), NULL },
  { "width", 'w', 0, G_OPTION_ARG_INT, &width, N_("The desired width of the image (default: 1024)"), "W" },
  { "size", 's', 0, G_OPTION_ARG_INT, &size, N_("The thumbnail size (default: 256)"), "S" },
  { "format", 0, 0, G_OPTION_ARG_CALLBACK, (void*) parse_format, N_("File format for output. Supported are 'png' and 'ppm' (default:png)"), NULL },
  { "print-background", 0, 0, G_OPTION_ARG_NONE, &print_background, N_("Print background images and colours (default: false)"), NULL },
  { "url", 'u', 0, G_OPTION_ARG_STRING, &url, N_("The URL"), N_("URL") },
  { "file", 'f', 0, G_OPTION_ARG_FILENAME, &infile, N_("The input file"), N_("FILE") },
  { "output-filename", 'o', 0, G_OPTION_ARG_FILENAME, &outfile, N_("The output file"), N_("FILE") },
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

  embed->state = 8;

  if (mode == MODE_PRINT) {
    Printer *printer = new Printer(mozembed, outfile, print_background);
    if (!printer) {
      return FALSE;
    }
    
    NS_ADDREF (printer);
    nsresult rv = printer->Print();
    NS_RELEASE (printer);
    if (NS_SUCCEEDED (rv)) {
      return FALSE;
    }
  } else {
    Writer *writer = nsnull;
    if (mode == MODE_PHOTO) {
      switch (format) {
        case FORMAT_PNG:
          writer = new PNGWriter (mozembed, outfile);
          break;
        case FORMAT_PPM:
          writer = new PPMWriter (mozembed, outfile);
          break;
        default:
          g_assert_not_reached ();
      }
    } else if (mode == MODE_THUMBNAIL) {
      writer = new ThumbnailWriter (mozembed, outfile, size);
    }
    if (!writer) {
      return FALSE;
    }
    
    retval = writer->Write() != PR_TRUE;
    delete writer;
  }

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

  LOG ("net_stop\n");

  embed->state |= 1;
  check_state (embed);
}

static void
embed_onload (Embed *embed)
{
  LOG ("onload\n");

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

  gtk_moz_embed_set_comp_path (GECKO_HOME);
  gtk_moz_embed_push_startup ();

  if (!RegisterComponents ()) {
    return NS_ERROR_FAILURE;
  }

  if (!InitPrefs ()) {
    return NS_ERROR_FAILURE;
  }

  nsresult rv = NS_OK;

  if (mode != MODE_PRINT) {
    /* This prevents us from printing all pages, so only do it for photo/thumbnail */
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
  }

  return rv;
}

/* --- */

static void G_GNUC_NORETURN
synopsis (void)
{
  char *name = g_get_prgname ();

  g_print (_("Usage: %s [--mode=photo|thumbnail|print] [...]\n"), name);

  switch (mode) {
    case MODE_PHOTO:
      g_print (_("Usage: %s [-t timeout] [-f] [-p] [-w width] [-u URL|-f file] -o FILENAME\n"), name);
      break;
    case MODE_THUMBNAIL:
      g_print (_("Usage: %s [-t timeout] [-f] [-p] [-w width] -s SIZE [-u URL|-f file] -o FILENAME\n"), name);
      break;
    case MODE_PRINT:
      g_print (_("Usage: %s [-t timeout] [-f] [-w width] [--print-background] [-u URL|-f file] -o FILENAME\n"), name);
      break;
    default:
      break;
  }

  exit (1);
}
  
static GtkWidget *window;
static GtkWidget *embed;

static gboolean
timeout_cb (void)
{
  g_print ("Loading timed out; maybe try --force or increase the timeout with --timeout=N\n");
  if (force) {
    ((Embed*)embed)->state = 8;
    g_idle_add_full (G_PRIORITY_LOW,
                      (GSourceFunc) embed_take_picture, embed, NULL);
  } else {
    gtk_widget_destroy (window);
  }

  return FALSE;
}

int
main (int argc, char **argv)
{
  GdkScreen *screen;
  GError *error = NULL;

#ifdef ENABLE_NLS
  /* Initialize the i18n stuff */
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  if (!gtk_init_with_args (&argc, &argv, NULL, entries, GETTEXT_PACKAGE, &error)) {
    g_print ("%s\n", error->message);
    g_error_free (error);
    synopsis ();
  }

  /* Mode not explicitly specified, derive from invocation filename */
  if (mode == MODE_INVALID) {
    char *program = g_get_prgname ();
    g_assert (program != NULL);

    if (g_ascii_strcasecmp (program, "gnome-web-thumbnail") == 0) {
      mode = MODE_THUMBNAIL;
    } else if (g_ascii_strcasecmp (program, "gnome-web-print") == 0) {
      mode = MODE_PRINT;
    } else {
      mode = MODE_PHOTO;
    }
  
    LOG ("Mode detection: %s -> %d\n", program, mode);
  }

  /* Check format */
  if (mode != MODE_PHOTO && format != FORMAT_PNG) {
    g_print ("--format can only be used with --mode=photo\n");
    return 1;
  }
  if (mode == MODE_INVALID) {
    char *program = g_get_prgname ();
    g_assert (program != NULL);

    if (g_ascii_strcasecmp (program, "gnome-web-thumbnail") == 0) {
      mode = MODE_THUMBNAIL;
    } else if (g_ascii_strcasecmp (program, "gnome-web-print") == 0) {
      mode = MODE_PRINT;
    } else {
      mode = MODE_PHOTO;
    }
  
    LOG ("Mode detection: %s -> %d\n", program, mode);
  }

  /* Check size */
  if (mode == MODE_THUMBNAIL) {
    if (size == -1) {
      size = DEFAULT_THUMBNAIL_SIZE;
    }
    if (size != 32 && size != 64 && size != 96 && size != 128 && size != 256) {
      g_print ("--size can only be 32, 64, 96, 128 or 256!\n");
      return 1;
    }
  } else if (size != -1) {
    g_print ("--size is only available in thumbnail mode!\n");
    synopsis ();
  }

  /* Check width */
  if (width == -1) {
    if (mode == MODE_THUMBNAIL) {
      width = THUMBNAIL_WIDTH;
    } else {
      width = DEFAULT_WIDTH;
    }
  }
  if (width < MIN_WIDTH || width > MAX_WIDTH) {
    g_print ("--width out of bounds; must be between %d and %d!\n", MIN_WIDTH, MAX_WIDTH);
    return 1;
  }
  if (mode == MODE_THUMBNAIL && (width % 32) != 0) {
    g_print ("--width must be a multiple of 32 in thumbnail mode!\n");
    return 1;
  }

  /* Check --print-background */
  if (mode != MODE_PRINT && print_background) {
    g_print ("--print-background is only available in print mode!\n");
    synopsis ();
  }

  /* Check url/input filename */
  if ((url == NULL && infile == NULL) || (url != NULL && infile != NULL)) {
    synopsis ();
  }

  if (infile != NULL) {
    url = g_filename_to_uri (infile, NULL, NULL);
    if (url == NULL) {
      g_print ("Could not convert filename '%s' to URL!\n", infile);
      return 1;
    }
  }

  /* Check output filename */
  if (outfile == NULL) {
    g_print ("--output-filename must be specified!\n");
    synopsis ();
  }

  /* Arg checking complete, now let's get started! */

  /* Initialised gecko */
  if (NS_FAILED (init_gecko ())) {
    g_print ("Failed to initialise gecko!\n");
    return 1;
  };

  /* Create window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  gtk_window_set_role (GTK_WINDOW (window), "gnome-web-photo-hidden-window");
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);

  embed = GTK_WIDGET (g_object_new (TYPE_EMBED, NULL));
  gtk_widget_set_size_request (embed, width, HEIGHT);
  gtk_container_add (GTK_CONTAINER (window), embed);

#ifndef DEBUG_SHOW_WINDOW
  /* Move the window off screen */
  screen = gtk_widget_get_screen (GTK_WIDGET (window));
  gtk_window_move (GTK_WINDOW (window),
		   gdk_screen_get_width (screen) + 100,
		   gdk_screen_get_height (screen) + 100);
  gtk_widget_show_all (window);
  gdk_window_hide (window->window);
#else
  gtk_widget_show_all (window);
#endif

  gtk_moz_embed_load_url (GTK_MOZ_EMBED (embed), url);

  /* FIXME is there a way to guarantee a kill after TIMEOUT secs? */
  g_print ("timeout: %d\n", timeout);
  g_timeout_add (CLAMP (timeout, 1, 3600) * 1000,
		 (GSourceFunc) timeout_cb, window);

  gtk_main ();

  return retval;
}
