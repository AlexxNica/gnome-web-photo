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

#include "Prefs.h"

#include <nsCOMPtr.h>
#include <nsIPrefService.h>
#include <nsIServiceManager.h>
#include <nsIServiceManager.h>
#include <nsILocalFile.h>
#include <nsString.h>

#include <pango/pango.h>
#include <gconf/gconf-client.h>

#ifdef GNOME_ENABLE_DEBUG
#define LOG printf
#else
#define LOG //
#endif

static const char *font_languages[] = {
  "ar",
  "el",
  "he",
  "ja",
  "ko",
  "th",
  "tr",
  "x-armn",
  "x-baltic",
  "x-beng",
  "x-cans",
  "x-central-euro",
  "x-cyrillic",
  "x-devanagari",
  "x-ethi",
  "x-geor",
  "x-gujr",
  "x-guru",
  "x-khmr",
  "x-mlym",
  "x-tamil",
  "x-unicode",
  "x-western",
  "zh-CN",
  "zh-HK",
  "zh-TW"
};

static const char *font_types[] = { "variable", "monospace" };
static const char *size_types[] = { "variable", "fixed" };

#define GNOME_VARIABLE_FONT_KEY   "/desktop/gnome/interface/document_font_name"
#define GNOME_MONOSPACE_FONT_KEY  "/desktop/gnome/interface/monospace_font_name"
#define EPHY_FONT_DIR             "/apps/epiphany/web"
#define DEFAULT_MIN_SIZE          7

static PRBool
ParsePangoFont (const char *aFont,
                char **_name,
                int *_size)
{
  PangoFontMask mask = (PangoFontMask) (PANGO_FONT_MASK_FAMILY | PANGO_FONT_MASK_SIZE);
  PRBool retval = PR_FALSE;

  PangoFontDescription *desc = pango_font_description_from_string (aFont);
  if (desc) {
    if ((pango_font_description_get_set_fields (desc) & mask) == mask) {
      *_size = PANGO_PIXELS (pango_font_description_get_size (desc));
      *_name = g_strdup (pango_font_description_get_family (desc));
      retval = *_name && *_size > 0;
    }

    pango_font_description_free (desc);
  }

  return retval;
}

PRBool
InitPrefs ()
{
  nsresult rv;
  nsCOMPtr<nsIPrefService> prefService (do_GetService (NS_PREFSERVICE_CONTRACTID, &rv));
  NS_ENSURE_SUCCESS (rv, PR_FALSE);

  nsCOMPtr<nsIPrefBranch> prefBranch;
  rv = prefService->GetBranch ("", getter_AddRefs (prefBranch));
  NS_ENSURE_SUCCESS (rv, PR_FALSE);

  /* read our predefined default prefs */
  nsCOMPtr<nsILocalFile> prefsFile;
  rv = NS_NewNativeLocalFile(nsDependentCString(SHARE_DIR "/prefs.js"),
			     PR_TRUE, getter_AddRefs (prefsFile));
  NS_ENSURE_SUCCESS (rv, PR_FALSE);
  
  rv = prefService->ReadUserPrefs(prefsFile);
  NS_ENSURE_SUCCESS (rv, PR_FALSE);

  GConfClient *client = gconf_client_get_default ();

  char *defaultFontVar = gconf_client_get_string (client, GNOME_VARIABLE_FONT_KEY, NULL);
  char *defaultFontMono = gconf_client_get_string (client, GNOME_MONOSPACE_FONT_KEY, NULL);
  if (!defaultFontVar || !defaultFontMono) return PR_FALSE;

  char *defaultFont[2] = { nsnull, nsnull };
  int defaultSize[2] = { 0, 0 };
  if (!ParsePangoFont (defaultFontVar, &defaultFont[0], &defaultSize[0]) ||
      !ParsePangoFont (defaultFontMono, &defaultFont[1], &defaultSize[1])) return PR_FALSE;
  g_free (defaultFontVar);
  g_free (defaultFontMono);

  for (PRUint32 i = 0; i < G_N_ELEMENTS (font_languages); ++i) {
    const char *lang = font_languages[i];
    char key[256], pref[256];

    for (PRUint32 t = 0; t < G_N_ELEMENTS (font_types); ++t) {
      snprintf (key, sizeof (key), "%s/font_%s_%s", EPHY_FONT_DIR, font_types[t], lang);
      snprintf (pref, sizeof (pref), "font.name.%s.%s", font_types[t], lang);

      char *font = gconf_client_get_string (client, key, NULL);
      if (font) {
        prefBranch->SetCharPref (pref, font);
        g_free (font);
      } else {
        prefBranch->SetCharPref (pref, defaultFont[t]);
      }
      
      snprintf (key, sizeof (key), "%s/%s_font_size_%s", EPHY_FONT_DIR, size_types[t], lang);
      snprintf (pref, sizeof (pref), "font.size.%s.%s", size_types[t], lang);

      int size = gconf_client_get_int (client, key, NULL);
      if (size > 0) {
        prefBranch->SetIntPref (pref, size);
      } else {
        prefBranch->SetIntPref (pref, defaultSize[t]);
      }
    }
  
    snprintf (key, sizeof (key), "%s/minimum_font_size_%s", EPHY_FONT_DIR, lang);
    snprintf (pref, sizeof (pref), "font.mimimum-size.%s", lang);

    int size = gconf_client_get_int (client, key, NULL);
    if (size > 0) {
      prefBranch->SetIntPref (pref, size);
    } else {
      prefBranch->SetIntPref (pref, DEFAULT_MIN_SIZE);
    }
  }

  g_object_unref (client);
  g_free (defaultFont[0]);
  g_free (defaultFont[1]);

  return PR_TRUE;
}
