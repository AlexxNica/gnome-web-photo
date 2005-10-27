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

#include "Components.h"

#include <nsCOMPtr.h>
#include <nsIServiceManager.h>
#include <nsIComponentRegistrar.h>
#include <nsIGenericFactory.h>
#include <nsMemory.h>
#include <nsEmbedCID.h>
#include <nsIPromptService.h>
#include <stdio.h>

#ifdef GNOME_ENABLE_DEBUG
#define LOG(x) printf (x)
#else
#define LOG(x)
#endif

#define PROMPTER_CLASSNAME	"Dummy Prompt Service"
#define PROMPTER_CID		{ 0x228965b9, 0x95d5, 0x4ae2, {0xa6, 0x88, 0x6e, 0x1d, 0x34, 0xc7, 0x83, 0xab} }

class Prompter : public nsIPromptService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPROMPTSERVICE

  Prompter () { };

private:
  ~Prompter () { };
};

NS_IMPL_ISUPPORTS1(Prompter, nsIPromptService)

/* void alert (in nsIDOMWindow aParent, in wstring aDialogTitle, in wstring aText); */
NS_IMETHODIMP Prompter::Alert(nsIDOMWindow *aParent, const PRUnichar *aDialogTitle, const PRUnichar *aText)
{
  LOG ("Alert\n");
  return NS_OK;
}

/* void alertCheck (in nsIDOMWindow aParent, in wstring aDialogTitle, in wstring aText, in wstring aCheckMsg, inout boolean aCheckState); */
NS_IMETHODIMP Prompter::AlertCheck(nsIDOMWindow *aParent, const PRUnichar *aDialogTitle, const PRUnichar *aText, const PRUnichar *aCheckMsg, PRBool *aCheckState)
{
  LOG ("AlertCheck\n");
  *aCheckState = PR_FALSE;
  return NS_OK;
}

/* boolean confirm (in nsIDOMWindow aParent, in wstring aDialogTitle, in wstring aText); */
NS_IMETHODIMP Prompter::Confirm(nsIDOMWindow *aParent, const PRUnichar *aDialogTitle, const PRUnichar *aText, PRBool *_retval)
{
  LOG ("Confirm\n");
  *_retval = PR_FALSE;
  return NS_OK;
}

/* boolean confirmCheck (in nsIDOMWindow aParent, in wstring aDialogTitle, in wstring aText, in wstring aCheckMsg, inout boolean aCheckState); */
NS_IMETHODIMP Prompter::ConfirmCheck(nsIDOMWindow *aParent, const PRUnichar *aDialogTitle, const PRUnichar *aText, const PRUnichar *aCheckMsg, PRBool *aCheckState, PRBool *_retval)
{
  LOG ("ConfirmCheck\n");
  *aCheckState = PR_FALSE;
  *_retval = PR_FALSE;
  return NS_OK;
}

/* PRInt32 confirmEx (in nsIDOMWindow aParent, in wstring aDialogTitle, in wstring aText, in unsigned long aButtonFlags, in wstring aButton0Title, in wstring aButton1Title, in wstring aButton2Title, in wstring aCheckMsg, inout boolean aCheckState); */
NS_IMETHODIMP Prompter::ConfirmEx(nsIDOMWindow *aParent, const PRUnichar *aDialogTitle, const PRUnichar *aText, PRUint32 aButtonFlags, const PRUnichar *aButton0Title, const PRUnichar *aButton1Title, const PRUnichar *aButton2Title, const PRUnichar *aCheckMsg, PRBool *aCheckState, PRInt32 *_retval)
{
  LOG ("ConfirmEx\n");
#define FLAGS (BUTTON_TITLE_CANCEL | BUTTON_TITLE_NO | BUTTON_TITLE_DONT_SAVE)
  *aCheckState = PR_FALSE;
  *_retval = (aButtonFlags & FLAGS) ? 0 :
             (aButtonFlags & FLAGS * BUTTON_POS_1) ? 1 :
             (aButtonFlags & FLAGS * BUTTON_POS_2) ? 2 : -1;
             
  return NS_OK;
}

/* boolean prompt (in nsIDOMWindow aParent, in wstring aDialogTitle, in wstring aText, inout wstring aValue, in wstring aCheckMsg, inout boolean aCheckState); */
NS_IMETHODIMP Prompter::Prompt(nsIDOMWindow *aParent, const PRUnichar *aDialogTitle, const PRUnichar *aText, PRUnichar **aValue, const PRUnichar *aCheckMsg, PRBool *aCheckState, PRBool *_retval)
{
  LOG ("Prompt\n");
  *aCheckState = PR_FALSE;
  *_retval = PR_FALSE;
  return NS_OK;
}

/* boolean promptUsernameAndPassword (in nsIDOMWindow aParent, in wstring aDialogTitle, in wstring aText, inout wstring aUsername, inout wstring aPassword, in wstring aCheckMsg, inout boolean aCheckState); */
NS_IMETHODIMP Prompter::PromptUsernameAndPassword(nsIDOMWindow *aParent, const PRUnichar *aDialogTitle, const PRUnichar *aText, PRUnichar **aUsername, PRUnichar **aPassword, const PRUnichar *aCheckMsg, PRBool *aCheckState, PRBool *_retval)
{
  LOG ("PromptUsernameAndPassword\n");
  *aUsername = nsnull;
  *aPassword = nsnull;
  *_retval = PR_FALSE;
  return NS_OK;
}

/* boolean promptPassword (in nsIDOMWindow aParent, in wstring aDialogTitle, in wstring aText, inout wstring aPassword, in wstring aCheckMsg, inout boolean aCheckState); */
NS_IMETHODIMP Prompter::PromptPassword(nsIDOMWindow *aParent, const PRUnichar *aDialogTitle, const PRUnichar *aText, PRUnichar **aPassword, const PRUnichar *aCheckMsg, PRBool *aCheckState, PRBool *_retval)
{
  LOG ("PromptPassword\n");
  *aPassword = nsnull;
  *_retval = PR_FALSE;
  return NS_OK;
}

/* boolean select (in nsIDOMWindow aParent, in wstring aDialogTitle, in wstring aText, in PRUint32 aCount, [array, size_is (aCount)] in wstring aSelectList, out long aOutSelection); */
NS_IMETHODIMP Prompter::Select(nsIDOMWindow *aParent, const PRUnichar *aDialogTitle, const PRUnichar *aText, PRUint32 aCount, const PRUnichar **aSelectList, PRInt32 *aOutSelection, PRBool *_retval)
{
  LOG ("Select\n");
  *aOutSelection = 0;
  *_retval = PR_FALSE;
  return NS_OK;
}

#ifdef HAVE_MOZILLA_PSM

#include <nsIBadCertListener.h>

#define NSSDIALOGS_CLASSNAME "Dummy NSS Dialogs"
#define NSSDIALOGS_CID { 0x128e643e, 0x8b28, 0x4eea, { 0x8d, 0xe7, 0x22, 0x4f, 0x5a, 0xa0, 0x56, 0x54 } }

class NSSDialogs : public nsIBadCertListener
{
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIBADCERTLISTENER

    NSSDialogs () { }
    ~NSSDialogs () { }
};

NS_IMPL_THREADSAFE_ISUPPORTS1 (NSSDialogs, nsIBadCertListener)

/* boolean confirmUnknownIssuer (in nsIInterfaceRequestor socketInfo, in nsIX509Cert cert, out short certAddType); */
NS_IMETHODIMP
NSSDialogs::ConfirmUnknownIssuer(nsIInterfaceRequestor *socketInfo,
				 nsIX509Cert *cert,
				 PRInt16 *certAddType,
				 PRBool *_retval)
{
  LOG ("ConfirmUnknownIssuer\n");
  *certAddType = nsIBadCertListener::ADD_TRUSTED_FOR_SESSION;
  *_retval = PR_TRUE;
  return NS_OK;
}

/* boolean confirmMismatchDomain (in nsIInterfaceRequestor socketInfo, in AUTF8String targetURL, in nsIX509Cert cert); */
NS_IMETHODIMP
NSSDialogs::ConfirmMismatchDomain(nsIInterfaceRequestor *socketInfo,
				  const nsACString & targetURL,
				  nsIX509Cert *cert,
				  PRBool *_retval)
{
  LOG ("ConfirmMismatchDomain\n");
  *_retval = PR_TRUE;
  return NS_OK;
}

/* boolean confirmCertExpired (in nsIInterfaceRequestor socketInfo, in nsIX509Cert cert); */
NS_IMETHODIMP
NSSDialogs::ConfirmCertExpired(nsIInterfaceRequestor *socketInfo,
			       nsIX509Cert *cert,
			       PRBool *_retval)
{
  LOG ("ConfirmCertExpired\n");
  *_retval = PR_TRUE;
  return NS_OK;
}

/* void notifyCrlNextupdate (in nsIInterfaceRequestor socketInfo, in AUTF8String targetURL, in nsIX509Cert cert); */
NS_IMETHODIMP
NSSDialogs::NotifyCrlNextupdate(nsIInterfaceRequestor *socketInfo,
				const nsACString & targetURL,
				nsIX509Cert *cert)
{
  LOG ("NotifyCrlNextupdate\n");
  return NS_OK;
}

#endif /* HAVE_MOZILLA_PSM */

/* -------------------------------------------------------------------------- */

NS_GENERIC_FACTORY_CONSTRUCTOR(Prompter)

#ifdef HAVE_MOZILLA_PSM
NS_GENERIC_FACTORY_CONSTRUCTOR(NSSDialogs)
#endif

static const nsModuleComponentInfo sAppComps[] =
{
  {
    PROMPTER_CLASSNAME,
    PROMPTER_CID,
    NS_PROMPTSERVICE_CONTRACTID,
    PrompterConstructor
  },
#ifdef HAVE_MOZILLA_PSM
  {
    NSSDIALOGS_CLASSNAME,
    NSSDIALOGS_CID,
    NS_BADCERTLISTENER_CONTRACTID,
    NSSDialogsConstructor
  },
#endif /* HAVE_MOZILLA_PSM */
};

PRBool
RegisterComponents ()
{
  PRBool retval = PR_FALSE;
  nsresult rv;
  nsCOMPtr<nsIComponentRegistrar> cr;
  rv = NS_GetComponentRegistrar(getter_AddRefs(cr));
  NS_ENSURE_SUCCESS (rv, retval);

  retval = PR_TRUE;

  for (PRUint32 i = 0; i < NS_ARRAY_LENGTH(sAppComps); ++i) {
    nsCOMPtr<nsIGenericFactory> componentFactory;
    rv = NS_NewGenericFactory(getter_AddRefs(componentFactory), &(sAppComps[i]));
    if (NS_FAILED(rv) || !componentFactory) {
      printf ("Failed to make a factory for %s\n", sAppComps[i].mDescription);
      retval = PR_FALSE;
      continue;  /* don't abort registering other components */
    }

    rv = cr->RegisterFactory(sAppComps[i].mCID,
                             sAppComps[i].mDescription,
                             sAppComps[i].mContractID,
                             componentFactory);
    if (NS_FAILED(rv)) {
      printf ("Failed to register %s\n", sAppComps[i].mDescription);
      retval = PR_FALSE;
    }
  }

  return retval;
}
