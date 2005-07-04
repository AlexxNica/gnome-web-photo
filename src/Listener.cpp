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

#include "Listener.h"

#include <nsIWebBrowser.h>
#include <nsIWebBrowserSetup.h>
#include <nsIDOMWindow.h>
#include <nsIDOMWindow2.h>
#include <nsIDOMEventTarget.h>
#include <gtkmozembed_internal.h>

Listener::Listener (GtkMozEmbed *aEmbed)
: mEmbed(aEmbed)
, mAttached(PR_FALSE)
{
}

Listener::~Listener()
{
}

nsresult
Listener::Attach ()
{
  NS_ENSURE_STATE (mEmbed);

  nsresult rv = NS_ERROR_FAILURE;
  nsCOMPtr<nsIWebBrowser> browser;
  gtk_moz_embed_get_nsIWebBrowser (mEmbed, getter_AddRefs (browser));
  NS_ENSURE_TRUE (browser, rv);

  nsCOMPtr<nsIDOMWindow> domWin;
  rv = browser->GetContentDOMWindow (getter_AddRefs (domWin));
  NS_ENSURE_SUCCESS (rv, rv);

  nsCOMPtr<nsIDOMWindow2> domWin2 (do_QueryInterface (domWin, &rv));
  NS_ENSURE_SUCCESS (rv, rv);

  nsCOMPtr<nsIDOMEventTarget> target;
  rv = domWin2->GetWindowRoot (getter_AddRefs (target));
  NS_ENSURE_SUCCESS (rv, rv);

  rv = target->AddEventListener (NS_LITERAL_STRING ("load"), this, PR_TRUE);

  mAttached = PR_TRUE;

  nsCOMPtr<nsIWebBrowserSetup> setup (do_QueryInterface (browser, &rv));
  NS_ENSURE_SUCCESS (rv, rv);

  rv = setup->SetProperty (nsIWebBrowserSetup::SETUP_ALLOW_META_REDIRECTS, PR_FALSE);
  /* set this for now, since sizing doesn't work right with frames */
  rv |= setup->SetProperty (nsIWebBrowserSetup::SETUP_ALLOW_SUBFRAMES , PR_FALSE);
  rv |= setup->SetProperty (nsIWebBrowserSetup::SETUP_USE_GLOBAL_HISTORY , PR_FALSE);

  return rv;
}

NS_IMPL_ISUPPORTS2 (Listener, nsIDOMEventListener, nsIDOMLoadListener)

NS_IMETHODIMP
Listener::HandleEvent (nsIDOMEvent* aDOMEvent)
{
  return NS_OK;
}

NS_IMETHODIMP
Listener:: Load (nsIDOMEvent* aEvent)
{
  /* HACK!
   * We need to unset the env var now, because otherwise mozilla will try
   * to dump the image too.
   * http://lxr.mozilla.org/seamonkey/source/layout/base/nsDocumentViewer.cpp#1017
   * Do this on the first onload, not on the onload for the main DOM window!
   */
  g_unsetenv ("MOZ_FORCE_PAINT_AFTER_ONLOAD");

  g_signal_emit_by_name (mEmbed, "onload");
  return NS_OK;
}

NS_IMETHODIMP
Listener:: BeforeUnload (nsIDOMEvent* aEvent)
{
  return NS_OK;
}

NS_IMETHODIMP
Listener:: Unload (nsIDOMEvent* aEvent)
{
  return NS_OK;
}

NS_IMETHODIMP
Listener:: Abort (nsIDOMEvent* aEvent)
{
  return NS_OK;
}

NS_IMETHODIMP
Listener:: Error (nsIDOMEvent* aEvent)
{
  return NS_OK;
}

NS_IMETHODIMP
Listener::PageRestore (nsIDOMEvent* aEvent)
{
  return NS_OK;
}
