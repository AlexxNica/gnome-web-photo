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

#include "Listener.h"

#include <nsIWebBrowser.h>
#include <nsIDOMWindow.h>
#include <nsIDOMWindow2.h>
#include <nsIDOMEventTarget.h>
#include <gtkmozembed_internal.h>

Listener::Listener (GtkMozEmbed *aEmbed)
: mEmbed(aEmbed)
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

  return target->AddEventListener (NS_LITERAL_STRING ("load"), this, PR_TRUE);
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
