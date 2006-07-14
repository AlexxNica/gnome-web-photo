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
#include "Embed.h"
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

typedef enum
{
  STATE_CLEAN,
  STATE_END,
  STATE_ERROR,
  STATE_NEXT,
  STATE_PRINT,
  STATE_WAIT,
  STATE_WORK,
} StateType;

static const char *STATES[] =  { "CLEAN", "END", "ERROR", "NEXT", "PRINT", "WAIT", "WORK" };

static StateType state;
static void state_change (StateType);

static Embed *gEmbed;

/* --- */

static const char *modes[] = { "photo", "thumbnail", "print" };
static const char *formats[] = { "png", "ppm" };

static int mode = MODE_INVALID;
static int timeout = 60;
static gboolean force = FALSE;
static int width = -1;
static int size = -1;
static int format = FORMAT_PNG;
static gboolean print_background = FALSE;
static char **arguments;
static char *uri;
static char *outfile;
static gboolean is_file;

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

  if (i == MODE_INVALID) {
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

  if (i == FORMAT_INVALID) {
    *error = g_error_new (G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
                          _("Unknown format '%s'"), value);
    return FALSE;
  }

  return TRUE;
}

static const GOptionEntry entries [] =
{
  { "mode", 'm', 0, G_OPTION_ARG_CALLBACK, (void*) parse_mode,
    N_("Operation mode [photo|thumbnail|print]"), NULL },
  { "timeout", 't', 0, G_OPTION_ARG_INT, &timeout,
    N_("The timeout in seconds, or 0 to disable timeout (default: 60)"), "T" },
  { "force", 'f', 0, G_OPTION_ARG_NONE, &force,
    N_("Force output when timeout expires, even if the page isn't loaded fully"), NULL },
  { "width", 'w', 0, G_OPTION_ARG_INT, &width,
    N_("The desired width of the image (default: 1024)"), "W" },
  { "size", 's', 0, G_OPTION_ARG_INT, &size,
    N_("The thumbnail size (default: 256)"), "S" },
  { "format", 0, 0, G_OPTION_ARG_CALLBACK, (void*) parse_format,
    N_("File format for output. Supported are 'png' and 'ppm' (default:png)"), N_("FORMAT") },
  { "print-background", 0, 0, G_OPTION_ARG_NONE, &print_background,
    N_("Print background images and colours (default: false)"), NULL },
  { "files", 0, 0, G_OPTION_ARG_NONE, &is_file,
    "", },
  { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &arguments, "", NULL },
  { NULL }
};

/* --- */

static guint timeout_id;

static void
take_picture (Embed *embed)
{
  GtkMozEmbed *mozembed = GTK_MOZ_EMBED (embed);
  gboolean success = FALSE;

  if (mode == MODE_PRINT) {
    Printer *printer = new Printer(mozembed, outfile, print_background);
    if (printer) {
      NS_ADDREF (printer);
      nsresult rv = printer->Print();
      NS_RELEASE (printer);
      if (NS_SUCCEEDED (rv)) {
        success = TRUE;

        /* FIXME: add another timeout while waiting for print-done? */

        state_change (STATE_PRINT);
      } else {
        state_change (STATE_ERROR);
      }
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

    if (writer) {
      success = writer->Write();
      delete writer;

      state_change (STATE_CLEAN);
    } else {
      state_change (STATE_ERROR);
    }
  }

  if (!success) {
    g_print ("Failed to take picture of uri '%s'\n", uri);
  }
}

static void
embed_ready_cb (Embed *embed)
{
  if (timeout_id != 0) {
    g_source_remove (timeout_id);
    timeout_id = 0;
  }

  if (state == STATE_CLEAN) {
    state_change (STATE_NEXT);
  } else {
    state_change (STATE_WORK);
  }
}

static void
print_done_cb (Embed *embed,
               gboolean success)
{
  if (!success) {
    g_warning ("Failed to print '%s'\n", uri);
  }

  state_change (STATE_CLEAN);
}

static gboolean
timeout_cb (void)
{
  timeout_id = 0;

  if (force) {
    g_print ("Load timed out; consider increasing the timeout (use 0 for no timeout)\n");
    state_change (STATE_WORK);
  } else {
    g_print ("Load timed out; consider using --force or increasing the timeout (use 0 for no timeout)\n");
    state_change (STATE_CLEAN);
  }

  return FALSE;
}

static gboolean
state_dispatch (void)
{
  LOG ("Dispatch state %s\n", STATES[state]);

  switch (state) {
    case STATE_NEXT:
      if (*arguments) {
        uri = *(arguments++);
        outfile = *(arguments++);
        g_assert (uri && outfile);

        LOG ("Now processing: URI '%s' => output file '%s'\n", uri, outfile);

        state = STATE_WAIT;
        embed_load (gEmbed, uri);

        if (timeout > 0) {
          timeout_id = g_timeout_add (timeout * 1000, (GSourceFunc) timeout_cb, NULL);
        }
      } else {
        state_change (STATE_END);
      }
      break;
    case STATE_WORK:
      take_picture (gEmbed);
      break;
    case STATE_CLEAN:
      embed_load (gEmbed, "about:blank");
      break;
    case STATE_WAIT:
    case STATE_PRINT:
      break;
    case STATE_END:
    case STATE_ERROR:
      gtk_main_quit ();
      break;
    default:
      g_assert_not_reached ();      
  }

  /* don't run again */
  return FALSE;
}

static void
state_change (StateType new_state)
{
  LOG ("state_change old-state %s new-state %s\n", STATES[state], STATES[new_state]);

  state = new_state;
  g_idle_add ((GSourceFunc) state_dispatch, NULL);
}

static nsresult
gecko_startup (void)
{
  /* BUG ALERT! If we don't have a profile, Gecko will crash on https sites and
   * when trying to open the password manager. The prefs will be set up so that
   * no cookies or passwords etc. will be persisted.
   */
  char *profile;
  profile = g_build_filename (g_get_home_dir (), GNOME_DOT_GNOME, NULL);
  gtk_moz_embed_set_profile_path (profile, "gnome-web-photo");
  g_free (profile);

#ifdef HAVE_GECKO_1_9
  gtk_moz_embed_set_path (GECKO_HOME);
#else
  gtk_moz_embed_set_comp_path (GECKO_HOME);
#endif

  /* Fire up the beast! */
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

static void
gecko_shutdown (void)
{
  gtk_moz_embed_pop_startup ();
}

/* --- */

static void G_GNUC_NORETURN
synopsis (void)
{
  char *name = g_get_prgname ();

  g_print (_("Usage: %s [--mode=photo|thumbnail|print] [...]\n"), name);

  switch (mode) {
    case MODE_PHOTO:
      g_print (_("Usage: %s [-t TIMEOUT] [--force] [--format FORMAT] [-w WIDTH] [--files] URI|FILE OUTFILE [...]\n"), name);
      break;
    case MODE_THUMBNAIL:
      g_print (_("Usage: %s [-t TIMEOUT] [--force] [-w WIDTH] -s SIZE [--files] URI|FILE OUTFILE [...]\n"), name);
      break;
    case MODE_PRINT:
      g_print (_("Usage: %s [-t TIMEOUT] [--force] [-w WIDTH] [--print-background] [--files] URI|FILE OUTFILE [...]\n"), name);
      break;
    default:
      break;
  }

  exit (1);
}

int
main (int argc, char **argv)
{
  GtkWidget *window;
  GdkScreen *screen;
  GError *error = NULL;
  int i, len;

#ifdef ENABLE_NLS
  /* Initialize the i18n stuff */
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  if (!gtk_init_with_args (&argc, &argv, NULL,
       			   (GOptionEntry*) entries,
			   GETTEXT_PACKAGE, &error)) {
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

  /* Check url/input filenames */
  if (!arguments || (g_strv_length (arguments) % 2) != 0) {
    g_print ("Missing arguments!\n");
    synopsis ();
  }

  /* Replace filenames with URIs */
  if (is_file) {
    for (i = 0; arguments[i]; i += 2) {
      char *new_uri;

      new_uri = g_filename_to_uri (arguments[i], NULL, &error);
      if (!new_uri) {
        g_print ("Error converting filename to URI: %s\n", error->message);
        g_error_free (error);
        return 1;
      }

      g_free (arguments[i]);
      arguments[i] = new_uri;
    }
  }

  /* Arg checking complete, now let's get started! */

  /* Initialised gecko */
  nsresult rv = gecko_startup ();
  if (NS_FAILED (rv)) {
    g_print ("Failed to initialise gecko (rv = %x)!\n", rv);
    return 1;
  };

  /* Create window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_role (GTK_WINDOW (window), "gnome-web-photo-hidden-window");
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);

  gEmbed = EMBED (g_object_new (TYPE_EMBED, NULL));
  g_signal_connect (gEmbed, "ready", G_CALLBACK (embed_ready_cb), NULL);
  g_signal_connect (gEmbed, "print-done", G_CALLBACK (print_done_cb), NULL);
  gtk_widget_set_size_request (GTK_WIDGET (gEmbed), width, HEIGHT);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (gEmbed));

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

  state_change (STATE_NEXT);

  gtk_main ();

  g_assert (state == STATE_END || state == STATE_ERROR);

  gtk_widget_destroy (window);

  gecko_shutdown ();

  return state == STATE_ERROR ? 1 : 0;
}
