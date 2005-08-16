/*
 *  Copyright (C) 2000-2004 Marco Pesenti Gritti
 *  Copyright (C) 2003, 2004, 2005 Christian Persch
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

#ifndef PRINTER_H
#define PRINTER_H

#include <nsCOMPtr.h>
#include <nsIWebBrowser.h>
#include <nsIWebBrowserPrint.h>
#include <nsIWebProgressListener.h>
#include <nsIPrintProgressParams.h>
#include <nsString.h>
#include <gtkmozembed.h>

class nsIPrintSettings;

class Printer : public nsIWebProgressListener,
		public nsIPrintProgressParams
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIWEBPROGRESSLISTENER
  NS_DECL_NSIPRINTPROGRESSPARAMS

  Printer(GtkMozEmbed*, const char*, PRBool);
  virtual ~Printer();

  nsresult Print();
  nsresult SetSettings(nsIPrintSettings*);

private:
  GtkMozEmbed *mEmbed;
  nsAutoString mFilename;
  PRBool mPrintBG;
};

#endif
