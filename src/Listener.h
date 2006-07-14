/*
 *  Copyright (C) 2003, 2004, 2005 Christian Persch
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

#ifndef LISTENER_H
#define LISTENER_H

#include <nsCOMPtr.h>
#include <nsIDOMLoadListener.h>
#include <nsString.h>
#include <gtkmozembed.h>

class Listener : public nsIDOMLoadListener
{
public:
  NS_DECL_ISUPPORTS

  /* nsIDOMEventListener */
  NS_IMETHOD HandleEvent(nsIDOMEvent* aEvent);

  /* nsIDOMLoadListener */
  NS_IMETHOD Load(nsIDOMEvent* aEvent);
  NS_IMETHOD BeforeUnload(nsIDOMEvent* aEvent);
  NS_IMETHOD Unload(nsIDOMEvent* aEvent);
  NS_IMETHOD Abort(nsIDOMEvent* aEvent);
  NS_IMETHOD Error(nsIDOMEvent* aEvent);
  NS_IMETHOD PageRestore(nsIDOMEvent* aEvent);

  Listener(GtkMozEmbed*);
  virtual ~Listener();

  nsresult Attach();

private:
  GtkMozEmbed *mEmbed;
  PRBool mAttached;
};

#endif
