/*
This file is part of JSONViewer Plugin for Notepad++
Copyright (C)2011 Kapil Ratnani <kapil.ratnani@iiitb.net>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include "JSONDialog.h"
#include "Hyperlinks.h"

HANDLE g_hMod;
CHAR * curJSON=NULL;
JSONDialog jsonDialog;
//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;
//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE hModule)
{
	g_hMod=hModule;
	jsonDialog.init((HINSTANCE)g_hMod,nppData._nppHandle);
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
void commandMenuInit()
{
	// setCommand(int index,                      // zero based number to indicate the order of command
	//            TCHAR *commandName,             // the command name that you want to see in plugin menu
	//            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
	//            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
	//            bool check0nInit                // optional. Make this menu item be checked visually
	//            );

	ShortcutKey *openJSONsk=new ShortcutKey();
	openJSONsk->_isAlt=TRUE;
	openJSONsk->_isCtrl=TRUE;
	openJSONsk->_isShift=TRUE;
	openJSONsk->_key='J';

	ShortcutKey *formatJSONsk=new ShortcutKey();
	formatJSONsk->_isAlt=TRUE;
	formatJSONsk->_isCtrl=TRUE;
	formatJSONsk->_isShift=TRUE;
	formatJSONsk->_key='M';
	setCommand(0, TEXT("Show &JSON Viewer"), openJSONDialog,openJSONsk , false);
	setCommand(1, TEXT("&Format JSON"), formatSelectedJSON,formatJSONsk , false);
	setCommand(2, TEXT("&About"), openAboutDlg,NULL , false);
}

INT_PTR CALLBACK abtDlgProc(HWND hwndDlg,UINT uMsg,WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		ConvertStaticToHyperlink(hwndDlg,IDC_WEB);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwndDlg,wParam);
			return TRUE;
		case IDC_WEB:
			ShellExecute(hwndDlg, TEXT("open"),TEXT("https://sourceforge.net/projects/nppjsonviewer/"),NULL, NULL, SW_SHOWNORMAL);
			return TRUE;
		}
	}
	return FALSE;
}

void openAboutDlg()
{
	::CreateDialog((HINSTANCE)g_hMod,MAKEINTRESOURCE(IDD_ABOUTDLG),nppData._nppHandle,abtDlgProc);
}


//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	delete funcItem[0]._pShKey;
	delete funcItem[1]._pShKey;
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
	if (index >= nbFunc)
		return false;

	if (!pFunc)
		return false;

	lstrcpy(funcItem[index]._itemName, cmdName);
	funcItem[index]._pFunc = pFunc;
	funcItem[index]._init2Check = check0nInit;
	funcItem[index]._pShKey = sk;

	return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//

void showJSONDialog(char *json)
{
	jsonDialog.setParent(nppData._nppHandle);
	jsonDialog.setJSON(json);
	tTbData	data = {0};

	if (!jsonDialog.isCreated())
	{
		jsonDialog.create(&data);

		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_LEFT;

		data.pszModuleName = jsonDialog.getPluginFileName();
		data.pszName=L"JSON Viewer";

		// the dlgDlg should be the index of funcItem where the current function pointer is
		data.dlgID = 0;
		::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
	}
	jsonDialog.display();
}

void openJSONDialog()
{
	// Get the current scintilla
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if (which == -1)
		return;

	HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;
	size_t start = ::SendMessage(curScintilla, SCI_GETSELECTIONSTART, 0, 0);
	size_t end = ::SendMessage(curScintilla, SCI_GETSELECTIONEND, 0, 0);
	if (end < start)
	{
		size_t tmp = start;
		start = end;
		end = tmp;
	}

	size_t asciiTextLen = end - start;
	
	if (asciiTextLen == 0)
	{ 
		//MessageBox(nppData._nppHandle,TEXT("Please select a JSON string."),TEXT("JSON Viewer"),MB_OK|MB_ICONINFORMATION);
		return;
	}
	
	

	curJSON = new CHAR[asciiTextLen+1];

	::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)curJSON);
	showJSONDialog(curJSON);
	delete curJSON;
}

void formatSelectedJSON(){
	// Get the current scintilla
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if (which == -1)
		return;

	HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;
	size_t start = ::SendMessage(curScintilla, SCI_GETSELECTIONSTART, 0, 0);
	size_t end = ::SendMessage(curScintilla, SCI_GETSELECTIONEND, 0, 0);
	if (end < start)
	{
		size_t tmp = start;
		start = end;
		end = tmp;
	}

	size_t asciiTextLen = end - start;
	
	if (asciiTextLen == 0)
	{ 
		MessageBox(nppData._nppHandle,TEXT("Please select a JSON string."),TEXT("JSON Viewer"),MB_OK|MB_ICONINFORMATION);
		return;
	}

	curJSON = new CHAR[asciiTextLen+1];

	::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)curJSON);
	CHAR* fJson=json_format_string(curJSON);
	::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)fJson);
	
	free(fJson);
	delete curJSON;
}
