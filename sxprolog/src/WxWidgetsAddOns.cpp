/*
 *   This file is part of the CxProlog system

 *   WxWidgetsAddOns.cpp
 *   by Sergio Lopes, Henrique Oliveira, A.Miguel Dias - 2006/09/02
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 Sergio Lopes, Henrique Oliveira, A.Miguel Dias, CITI, DI/FCT/UNL

 *   it under the terms of the GNU General Public License as published by
 *   CxProlog is free software; you can redistribute it and/or modify
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.

 *   CxProlog is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


/*
 * LOCAL CHANGELOG (by A.Miguel Dias):
 *
 * 08/Dec/2007 Replaced all occurrences of MakeWxObj without LookupWxObj.
 *             The previous version was plainly wrong.
 * 24/Apr/2007 Replaced all occurrences of "void *" for proper types.
 *             Replaced all returned results "wxString *" for "wxString".
 *             Replaced all returned results "wxString &" for "wxString".
 * 04/Sep/2006 Discovered that there are bugs remaining in this code.
 *             The code in this file has still "alpha quality", specially
 *             on Linux where it has been barely tested.
 * 03/Sep/2006 Fixed some problems in the low level event handling of
 *             text windows. These problems occurred on Linux only,
 *             not on Windows.
 * 02/Sep/2006 Took all the wxWidgets code contributed by Sergio Lopes
 *             and Henrique Oliveira and created this big source file.
 */

/* THERE IS AN ONGOING UPGRADE TO VERSION 2.8.
   MOST OF THIS FILE IS DEACTIVATED.
 */

#if USE_WXWIDGETS
#include <wx/wx.h>
#endif

extern "C" {
#include "CxProlog.h"
#define PushH(v)			Push(H, v)
}

// DISABLED: must be redone
#if USE_WXWIDGETSx

#include <list>
using namespace std ;

#if !wxUSE_GUI
#	error "GUI support required CxProlog with wxWidgets GUI."
#endif


/***** Graphical Objects Class *****/

class WxGraphicalObject {
public:
	virtual void draw() {} ;
	virtual const char *label() { return ""; } ;
	virtual void destroy() {	delete this; } ;
    virtual ~WxGraphicalObject() {} ;
} ;

class WxPixel: public WxGraphicalObject {
	wxFrame *m_frame;
	int m_x;
	int m_y;
	int m_r;
	int m_g;
	int m_b;
public:
	WxPixel();
	WxPixel(wxFrame *frame, int x, int y, int r, int g, int b);
	void draw();
};

class WxLine: public WxGraphicalObject {
private:
	wxFrame *m_frame;
	int m_x0;
	int m_y0;
	int m_x1;
	int m_y1;
	int m_r;
	int m_g;
	int m_b;
public:
	WxLine();
	WxLine(wxFrame *frame, int x0, int y0, int x1, int y1, int r, int g, int b);
	void draw();
};

class WxCircle: public WxGraphicalObject {
private:
	wxFrame *m_frame;
	int m_x;
	int m_y;
	int m_radius;
	int m_r;
	int m_g;
	int m_b;
	bool m_filled;
public:
	WxCircle();
	WxCircle(wxFrame *frame, int x, int y, int radius, int r, int g, int b, bool filled);
	void draw();
};

class WxRectangle: public WxGraphicalObject {
private:
	wxFrame *m_frame;
	int m_x;
	int m_y;
	int m_width;
	int m_height;
	int m_r;
	int m_g;
	int m_b;
	bool m_filled;
public:
	WxRectangle();
	WxRectangle(wxFrame *frame, int x, int y, int width, int height, int r, int g, int b, bool filled);
	void draw();
};

class WxText: public WxGraphicalObject {
private:
	wxFrame *m_frame;
	const char *m_text;
	int m_x;
	int m_y;
public:
	WxText();
	WxText(wxFrame *frame, const char *text, int x, int y);
	void draw();
};

class WxButton: public WxGraphicalObject {
private:
	wxFrame *m_frame;
	wxButton *m_button;
	int m_x;
	int m_y;
	const char *m_text;
public:
	WxButton();
	WxButton(wxFrame *frame, int x, int y, const char *text);
	const char *label();
};

class WxList: public WxGraphicalObject {
private:
	wxFrame *m_frame;
	wxListBox *m_list;
	int m_x;
	int m_y;
public:
	WxList();
	WxList(wxFrame *frame, wxArrayString *items, int x, int y);
	wxString getSelectedItem();
};



/***** Graphical Frame Class *****/

class GfxEventHandler: public wxEvtHandler {
public:
	virtual bool ProcessEvent(wxEvent& event);

	wxFrame *window;
};

class WxGraphicalFrame: public wxFrame {
	DECLARE_DYNAMIC_CLASS(WxGraphicalFrame)
private:
	list<WxGraphicalObject *> graphicalObjects;
public:
	WxGraphicalFrame();
	WxGraphicalFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	~WxGraphicalFrame();

	WxGraphicalObject *gfxPutPixel(int x, int y, char r, char g, char b);
	WxGraphicalObject *gfxLine(int x0, int y0, int x1, int y1, char r, char g, char b);
	WxGraphicalObject *gfxCircle(int x,int y,int radius,char r,char g,char b);
	WxGraphicalObject *gfxCircleFilled(int x,int y,int radius, char r,char g,char b);
	WxGraphicalObject *gfxRectangle(int x,int y,int width,int height,char r,char g, char b);
	WxGraphicalObject *gfxRectangleFilled(int x,int y,int width,int height,char r,char g, char b);
	WxGraphicalObject *gfxDrawText(const char *text, int x, int y);
	WxGraphicalObject *gfxButton(const char *text, int x, int y);
	WxGraphicalObject *gfxList(wxArrayString *items, int x, int y);
	void gfxDeleteObject(WxGraphicalObject *object);

	void gfxClear();
	void gfxClose();
	void repaint();

	WxGraphicalObject *getButton(const wxString &buttonName);
};



/***** Text Frame Class *****/

#define	WX_RETURN_OK		1
#define	WX_RETURN_CANCEL	2
#define	WX_RETURN_YES		3
#define	WX_RETURN_NO		4

/* For passing information about menus! */
typedef struct MenuInfo_tag {
	wxString menu_name;
	wxArrayString menu_elements;
}MenuInfo;

class TextEventHandler: public wxEvtHandler {
public:
	virtual bool ProcessEvent(wxEvent& event);
	wxFrame *owner ;
};

class WxTextFrame:public wxFrame {
	DECLARE_DYNAMIC_CLASS(WxTextFrame)

private:
	// for text control. without this it's just a plain window per se.
	// it adds a text pane (white pane), where some text can be inserted.
	// it also adds a mouse-menu with copy/paste/delete/select actions.
	// quite cool :)
	wxTextCtrl *m_pTextCtrl;

	//this is for menu bar
	wxMenuBar *m_pMenuBar;

	//for file path
	wxString CurrentDocPath;

	//for extra menus that the prolog-user can define
	//we only use this to keep track of the menu names
	wxArrayString menuNames;

public:

	WxTextFrame();
	WxTextFrame(const wxChar *title,int xpos, int ypos, int width, int height);
	WxTextFrame(const wxChar *title,int xpos, int ypos, int width, int height, MenuInfo *menus, int countMenus);
	virtual ~WxTextFrame();

	//Process the Action of opening a file
	virtual void OnMenuFileOpen(wxCommandEvent &event);
	//Process the Action of saving a file
	virtual void OnMenuFileSave(wxCommandEvent &event);
	//Process the Action of "saving as" a file
	//(saving the file with a different name)
	virtual void OnMenuFileSaveAs(wxCommandEvent &event);
	//Process the Action of exiting
	virtual void OnMenuFileQuit(wxCommandEvent &event);

	virtual bool ActionOpenFile();
	virtual bool ActionSave();
	virtual bool ActionSaveAs();

	virtual const wxString& getFilePath() const;
	virtual wxTextCtrl *getTextCtrl();

	//in order to know menu names from outside
	virtual const wxArrayString& getMenuNames() const;


	const wxString &txtEditorGetFilePath();
	int txtOpenFile();
	int txtSave();
	int txtSaveAs();
	const wxString txtEditorGetText(long from, long to);
	void txtAppendText(const char *text);
	const wxString txtGetSelectedText();
	void txtReplaceText(long from, long to, const char *text);
	void txtSetSelection(long from, long to);
	void txtClose();

};




/***** Graphical Objects Implementation *****/

/* Pixel */
WxPixel::WxPixel() {}

WxPixel::WxPixel(wxFrame *frame, int x, int y, int r, int g, int b) {
	m_frame = frame;
	m_x = x;
	m_y = y;
	m_r = r;
	m_g = g;
	m_b = b;
}

void WxPixel::draw() {
	if( m_frame != NULL) {
		wxClientDC dc((WxGraphicalFrame *)m_frame);
		dc.SetPen(wxPen( wxColour(m_r,m_g,m_b),1,wxSOLID ));
		dc.DrawPoint(m_x,m_y);
	}
}

/* Line */
WxLine::WxLine() {}

WxLine::WxLine(wxFrame *frame, int x0, int y0, int x1, int y1, int r, int g, int b) {
	m_frame = frame;
	m_x0 = x0;
	m_y0 = y0;
	m_x1 = x1;
	m_y1 = y1;
	m_r = r;
	m_g = g;
	m_b = b;
}

void WxLine::draw() {
	if( m_frame != NULL) {
		wxClientDC dc((WxGraphicalFrame *)m_frame);
		dc.SetPen(wxPen( wxColour(m_r,m_g,m_b),1,wxSOLID ));
		dc.DrawLine(m_x0,m_y0,m_x1,m_y1);
	}
}

/* Circle */
WxCircle::WxCircle() {}

WxCircle::WxCircle(wxFrame *frame, int x, int y, int radius, int r, int g, int b, bool filled) {
	m_frame = frame;
	m_x = x;
	m_y = y;
	m_radius = radius;
	m_r = r;
	m_g = g;
	m_b = b;
	m_filled = filled;
}

void WxCircle::draw() {
	if (m_filled) {
		if( m_frame != NULL) {
			wxClientDC dc((WxGraphicalFrame *)m_frame);
			dc.SetPen(wxPen(wxColour(m_r,m_g,m_b),1,wxSOLID));
			dc.SetBrush(wxBrush(wxColour(m_r,m_g,m_b),wxSOLID));
			dc.DrawCircle(m_x,m_y,m_radius);
		}
	}
	else {
		if(m_frame != NULL) {
			wxClientDC dc((WxGraphicalFrame *)m_frame);
			dc.SetPen(wxPen( wxColour(m_r,m_g,m_b),1,wxSOLID ));
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
			dc.DrawCircle(m_x,m_y,m_radius);
		}
	}
}

/* Rectangle */
WxRectangle::WxRectangle() {}

WxRectangle::WxRectangle(wxFrame *frame, int x, int y, int width, int height, int r, int g, int b, bool filled) {
	m_frame = frame;
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;
	m_r = r;
	m_g = g;
	m_b = b;
	m_filled = filled;
}

void WxRectangle::draw() {
	if (m_filled) {
		if(m_frame != NULL) {
			wxClientDC dc((WxGraphicalFrame *)m_frame);
			dc.SetPen(wxPen( wxColour(m_r,m_g,m_b),1,wxSOLID ));
			dc.SetBrush(wxBrush(wxColour(m_r,m_g,m_b),wxSOLID));
			dc.DrawRectangle(m_x,m_y,m_width,m_height);
		}
	}
	else {
		if(m_frame != NULL) {
			wxClientDC dc((WxGraphicalFrame *)m_frame);
			dc.SetPen(wxPen( wxColour(m_r,m_g,m_b),1,wxSOLID ));
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
			dc.DrawRectangle(m_x,m_y,m_width,m_height);
		}
	}
}

/* Text */
WxText::WxText() {}

WxText::WxText(wxFrame *frame, const char *text, int x, int y) {
	m_frame = frame;
	m_text = text;
	m_x = x;
	m_y = y;
}
#if 0
void WxText::draw() {
	if(m_frame != NULL) {
		wxClientDC dc((WxGraphicalFrame *)m_frame);
		dc.DrawText(m_text,m_x,m_y);
	}
}

WxButton::WxButton() {}

WxButton::WxButton(wxFrame *frame, int x, int y, const char *text) {
	m_frame = frame;
	m_x = x;
	m_y = y;
	m_text = text;
	m_button = new wxButton((WxGraphicalFrame *)frame, -1, text, wxPoint(x,y));
}

const char *WxButton::label() {
	return m_text;
}

WxList::WxList() {}

WxList::WxList(wxFrame *frame, wxArrayString *items, int x, int y) {
	m_frame = frame;
	m_x = x;
	m_y = y;

	wxArrayString *temp = items;
	int count = temp->GetCount();
	wxString *choices = new wxString[count];
	int i;

	for(i = 0; i < count; i++) {
		choices[i] = temp->Item(i);
	}

	m_list = new wxListBox((WxGraphicalFrame *)frame, -1, wxPoint(x,y),
		wxDefaultSize, count, choices, 0, wxDefaultValidator, wxT("listBox"));
}

wxString WxList::getSelectedItem() {
	int i = m_list->GetSelection();
	return m_list->GetString(i);
}



/***** Graphical Frame Implementation *****/

IMPLEMENT_DYNAMIC_CLASS(WxGraphicalFrame, wxWindow)

WxGraphicalFrame::WxGraphicalFrame() {}

WxGraphicalFrame::WxGraphicalFrame(const wxString& title, const wxPoint& pos, const wxSize& size):
		wxFrame((wxFrame*)NULL,1,title,pos,size) {
	GfxEventHandler *evt = new GfxEventHandler();
	evt->window = this;
	PushEventHandler(evt);
	Show(true);

}

WxGraphicalFrame::~WxGraphicalFrame() {
	WxDeleted(this);
}

void WxGraphicalFrame::repaint() {
	list<WxGraphicalObject *>::iterator iter;
	for(iter = graphicalObjects.begin(); iter != graphicalObjects.end(); iter++) {
		WxGraphicalObject *object = *iter;
		object->draw();
	}
}

WxGraphicalObject *WxGraphicalFrame::gfxPutPixel(int x, int y, char r, char g, char b) {
	WxPixel *pixel = new WxPixel(this,x,y,r,g,b);
	graphicalObjects.push_back(pixel);
	pixel->draw();
	return pixel;
}

WxGraphicalObject *WxGraphicalFrame::gfxLine(int x0, int y0, int x1, int y1, char r, char g, char b) {
	WxLine *line = new WxLine(this,x0,y0,x1,y1,r,g,b);
	graphicalObjects.push_back(line);
	line->draw();
	return line;
}

WxGraphicalObject *WxGraphicalFrame::gfxCircle(int x,int y,int radius,char r,char g,char b) {
	WxCircle *circle = new WxCircle(this,x,y,radius,r,g,b,false);
	graphicalObjects.push_back(circle);
	circle->draw();
	return circle;
}

WxGraphicalObject *WxGraphicalFrame::gfxCircleFilled(int x,int y,int radius,char r,char g,char b) {
	WxCircle *circle = new WxCircle(this,x,y,radius,r,g,b,true);
	graphicalObjects.push_back(circle);
	circle->draw();
	return circle;
}

WxGraphicalObject *WxGraphicalFrame::gfxRectangle(int x,int y,int width,int height,char r,char g, char b) {
	WxRectangle *rectangle = new WxRectangle(this,x,y,width,height,r,g,b,false);
	graphicalObjects.push_back(rectangle);
	rectangle->draw();
	return rectangle;
}

WxGraphicalObject *WxGraphicalFrame::gfxRectangleFilled(int x,int y,int width,int height,char r,char g, char b) {
	WxRectangle *rectangle = new WxRectangle(this,x,y,width,height,r,g,b,true);
	graphicalObjects.push_back(rectangle);
	rectangle->draw();
	return rectangle;
}

WxGraphicalObject *WxGraphicalFrame::gfxDrawText(const char *text, int x, int y) {
	WxText *txt = new WxText(this,text,x,y);
	graphicalObjects.push_back(txt);
	txt->draw();
	return txt;
}

WxGraphicalObject *WxGraphicalFrame::gfxButton(const char *text, int x, int y) {
	WxButton *button = new WxButton(this,x,y,text);
	graphicalObjects.push_back(button);
	return button;
}

WxGraphicalObject *WxGraphicalFrame::gfxList(wxArrayString *items, int x, int y) {
	WxList *list = new WxList(this,items,x,y);
	graphicalObjects.push_back(list);
	return list;
}

void WxGraphicalFrame::gfxDeleteObject(WxGraphicalObject *object) {
	object->destroy();
	graphicalObjects.remove(object);
	wxClientDC dc(this);
	dc.Clear();
	repaint();
}

void WxGraphicalFrame::gfxClear() {
	wxClientDC dc(this);
	dc.Clear();

	list<WxGraphicalObject *>::iterator iter;
	for(iter = graphicalObjects.begin(); iter != graphicalObjects.end(); iter++) {
		WxGraphicalObject *object = *iter;
		object->destroy();
	}
	graphicalObjects.clear();
}

void WxGraphicalFrame::gfxClose() {
	Destroy();
}

WxGraphicalObject *WxGraphicalFrame::getButton(const wxString &buttonName) {
	list<WxGraphicalObject *>::iterator iter;
	for(iter = graphicalObjects.begin(); iter != graphicalObjects.end(); iter++) {
		WxGraphicalObject *object = *iter;
		if(strcmp(buttonName.c_str(), object->label()) == 0)
			return object;
	}
	return (WxGraphicalObject *)0;
}

bool GfxEventHandler::ProcessEvent(wxEvent& event) {
	WXTYPE wxType = event.GetEventType();

	if(wxType == wxEVT_LEFT_DOWN ) {
	/* Left Mouse Button Event */
		wxMouseEvent *mouse = (wxMouseEvent *)&event;
		int x = mouse->GetX();
		int y = mouse->GetY();
		WxGraphicalFrame *frame = (WxGraphicalFrame *)event.GetEventObject();
		WxPostEvent("so.si.i", "event", frame, "mouse_down", x, y);
		return false;
	}
	else if(wxType == wxEVT_PAINT) {
	/* Paint Event */
		WxGraphicalFrame *frame = (WxGraphicalFrame *)event.GetEventObject();
		frame->repaint();
		return false;
	}
	else if(wxType == wxEVT_CLOSE_WINDOW) {
	/* Closing Event of the Window */
		WxGraphicalFrame *frame = (WxGraphicalFrame *)event.GetEventObject();
		WxPostEvent("so.s", "event", frame, "close_window");
		return false;
	}
	else if(wxType == wxEVT_COMMAND_BUTTON_CLICKED) {
	/* Button Event */
		const wxString& buttonName = ((wxButton *)event.GetEventObject())->GetLabel(); /* @@@ const added */
		WxGraphicalFrame *frame = (WxGraphicalFrame *)window;
		WxGraphicalObject *button = frame->getButton(buttonName);
		WxPostEvent("so.so", "event", frame, "button", button);
		return true;
	}

	wxEvtHandler* next = this->GetNextHandler() ;
	return next->ProcessEvent(event);
}



/***** Text Frame Implementation *****/

IMPLEMENT_DYNAMIC_CLASS(WxTextFrame, wxWindow)

WxTextFrame::WxTextFrame(){};
//we can remove this constructor later, if not needed

WxTextFrame::WxTextFrame(const wxChar *title,int xpos, int ypos,
						 int width, int height):
		wxFrame((wxFrame*)NULL, -1, title,
			wxPoint(xpos,ypos), wxSize(width,height))
{
	m_pTextCtrl = new wxTextCtrl(this,-1,
						wxString("Type some text..."),
						wxDefaultPosition, wxDefaultSize,
						wxTE_MULTILINE);
	Layout();

	//Status Bar
	CreateStatusBar(3);
	SetStatusText("Ready",3);

	TextEventHandler *evt = new TextEventHandler();
	PushEventHandler(evt);

	Show(true);
}

WxTextFrame::WxTextFrame(const wxChar *title,int xpos, int ypos, int width,
						 int height, MenuInfo *menus, int countMenus):
		wxFrame((wxFrame*)NULL, -1, title,
			wxPoint(xpos,ypos), wxSize(width,height))
{
	int t,x,mnuCount = 0;
	MenuInfo *tmp = menus;

	m_pTextCtrl = new wxTextCtrl(this,-1,wxString("Type some text..."),
				wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	Layout();

	//Menus
	//menu bar
	this->m_pMenuBar=new wxMenuBar();

	mnuCount=0;
	for(t=0;t<countMenus;t++) {
		wxMenu *new_menu = new wxMenu();

		for(x=0;x < (int)tmp[t].menu_elements.GetCount();x++) {
			if(tmp[t].menu_elements.Item(x).Cmp("|") == 0) {
				new_menu->AppendSeparator();
				continue;
			}
			new_menu->Append(mnuCount, tmp[t].menu_elements.Item(x));
			menuNames.Add(tmp[t].menu_elements.Item(x));
			mnuCount++;
		}

		this->m_pMenuBar->Append(new_menu, tmp[t].menu_name);
	}

	SetMenuBar(m_pMenuBar);

	//Status Bar
	CreateStatusBar(3);
	SetStatusText("Ready",3);

	TextEventHandler *evt = new TextEventHandler();
	evt->owner = this ;
	PushEventHandler(evt);

	Show(true);
}

WxTextFrame::~WxTextFrame() {
	WxDeleted(this);
}

void WxTextFrame::OnMenuFileOpen(wxCommandEvent& event) {
	wxFileDialog *OpenDialog = new wxFileDialog(
			this,
			"Choose a file to open",
			"",
			"",
			"Text files (*.txt)|*.txt|C++ Source Files (*.cpp,*.cxx) |*.cpp"
			",*.cxx|C Source Files (*.c)|*.c|C Header Files (*.h)|*.h",
			wxOPEN,
			wxDefaultPosition);

	if(OpenDialog->ShowModal() == wxID_OK) {
		CurrentDocPath = OpenDialog->GetPath();
		m_pTextCtrl->LoadFile(CurrentDocPath);
		SetTitle(wxString("Edit - ") << OpenDialog->GetFilename());
	}
}

void WxTextFrame::OnMenuFileSave(wxCommandEvent& event) {
	if( CurrentDocPath == "" ) {	/* @@@ was: CurrentDocPath.IsEmpty() */
		OnMenuFileSaveAs(event);
	}
	else {
		m_pTextCtrl->SaveFile(CurrentDocPath);
	}

}

void WxTextFrame::OnMenuFileSaveAs(wxCommandEvent& event) {
	wxFileDialog *SaveDialog = new wxFileDialog(
			this,
			"Save File As _?",
			"",
			"",
			"Text files (*.txt)|*.txt|C++ Source Files (*.cpp,*.cxx)| *.cpp,"
			"*.cxx|C Source Files (*.c)|*.c|C Header Files (*.h)|*.h",
			wxSAVE | wxOVERWRITE_PROMPT,
			wxDefaultPosition);

	//if the user clicked ok...
	if(SaveDialog->ShowModal() == wxID_OK) {
		CurrentDocPath = SaveDialog->GetPath();
		m_pTextCtrl->SaveFile(CurrentDocPath);
		SetTitle(wxString("Edit - ") << SaveDialog->GetFilename());
	}
}

void WxTextFrame::OnMenuFileQuit(wxCommandEvent& event) {
	Close(true);
}

bool WxTextFrame::ActionOpenFile() {
	wxFileDialog *OpenDialog = new wxFileDialog(
			this,
			"Choose a file to open",
			"",
			"",
			"Text files (*.txt)|*.txt|C++ Source Files (*.cpp,*.cxx)| *.cpp,"
			"*.cxx|C Source Files (*.c)|*.c|C Header Files (*.h)|*.h",
			wxOPEN,
			wxDefaultPosition);

	if(OpenDialog->ShowModal() == wxID_OK) {
		CurrentDocPath = OpenDialog->GetPath();
		m_pTextCtrl->LoadFile(CurrentDocPath);
		SetTitle(wxString("Edit - ") << OpenDialog->GetFilename());
		return true;
	}
	return false;
}

bool WxTextFrame::ActionSave() {
	if( CurrentDocPath == "" ) {	/* @@@ was: CurrentDocPath.IsEmpty() */
		return this->ActionSaveAs();
	}
	else {
		m_pTextCtrl->SaveFile(CurrentDocPath);
		return true;
	}
}

bool WxTextFrame::ActionSaveAs() {
	wxFileDialog *SaveDialog = new wxFileDialog(
			this,
			"Save File As _?",
			"",
			"",
			"Text files (*.txt)|*.txt|C++ Source Files (*.cpp,*.cxx)| *.cpp,"
			"*.cxx|C Source Files (*.c)|*.c|C Header Files (*.h)|*.h",
			wxSAVE | wxOVERWRITE_PROMPT,
			wxDefaultPosition);

	//if the user clicked ok...
	if(SaveDialog->ShowModal() == wxID_OK) {
		CurrentDocPath = SaveDialog->GetPath();
		m_pTextCtrl->SaveFile(CurrentDocPath);
		SetTitle(wxString("Edit - ") << SaveDialog->GetFilename());
		return true;
	}
	return false;
}

const wxString &WxTextFrame::getFilePath() const {
	return this->CurrentDocPath;
}

wxTextCtrl *WxTextFrame::getTextCtrl() {
	return this->m_pTextCtrl;
}

const wxArrayString& WxTextFrame::getMenuNames() const {
	return this->menuNames;
}

const wxString &WxTextFrame::txtEditorGetFilePath() {
	return getFilePath();
}

int WxTextFrame::txtOpenFile() {
	if(ActionOpenFile())
		return WX_RETURN_OK;
	else
		return WX_RETURN_CANCEL;
}

int WxTextFrame::txtSave() {
	if(ActionSave())
		return WX_RETURN_OK;
	else
		return WX_RETURN_CANCEL;
}

int WxTextFrame::txtSaveAs() {
	if(ActionSaveAs())
		return WX_RETURN_OK;
	else
		return WX_RETURN_CANCEL;
}

const wxString WxTextFrame::txtEditorGetText(long from, long to) {
	return getTextCtrl()->GetRange(from,to);
}

void WxTextFrame::txtAppendText(const char *text) {
	getTextCtrl()->AppendText(text);
}

const wxString WxTextFrame::txtGetSelectedText() {
	return getTextCtrl()->GetStringSelection();
}

void WxTextFrame::txtReplaceText(long from, long to, const char *text) {
	getTextCtrl()->Replace(from,to,text);
}

void WxTextFrame::txtSetSelection(long from, long to) {
	getTextCtrl()->SetSelection(from,to);
}

void WxTextFrame::txtClose() {
	Destroy() ; // @@@ amd was Close(TRUE) ;
}

/* Event handler */
bool TextEventHandler::ProcessEvent(wxEvent& event) {
	WXTYPE wxType = event.GetEventType();
	int type = event.GetId();
//	WxTextFrame *frame = (WxTextFrame *)event.GetEventObject(); @@@ amd
	WxTextFrame *frame = (WxTextFrame *)owner;

	if(wxType == wxEVT_COMMAND_MENU_SELECTED) {
		/* Selection of user-defined Menu Option Event */
		const wxChar *menuName = frame->getMenuNames().Item(type).c_str();
		/* event(Window,usermenu(MenuName)) */
		WxPostEvent("so.ss", "event", frame, "usermenu", menuName);

		return false;
	}
	else if(wxType == wxEVT_CLOSE_WINDOW) {
	/* Window Closing Event
	 * This includes the window closing button (red cross on the upper right
	 * corner) and the closing option from the automatic menu on the windows
	 * upper left corner.
	 */

		/* event(Window,close_window) */
		WxPostEvent("so.s", "event", frame, "close_window");


		return false;
	}

	wxEvtHandler* next = this->GetNextHandler() ;
	return next->ProcessEvent(event);
}


/* ------------------------------------------------------------------*/

/* GENERAL PURPOSE PREDICATES */

/* Auxiliary functions */
Pt getFileMessg(int isSave, const char *msg) {
	wxFileDialog *dlgOne;

	if(isSave) // save
		dlgOne = new wxFileDialog((wxFrame*)NULL, msg,"","","",wxSAVE |
								wxFILE_MUST_EXIST,wxDefaultPosition);
	else //default == open
		dlgOne = new wxFileDialog((wxFrame*)NULL, msg,"","","",wxOPEN |
								wxFILE_MUST_EXIST,wxDefaultPosition);

	if(dlgOne->ShowModal() == wxID_OK)
		return MakeTempAtom(cCharPt(dlgOne->GetPath().c_str()));
	else
		return nil;
}

Pt getMultipleFiles() {
	wxArrayString paths;
	wxFileDialog *dlgMany=new wxFileDialog((wxFrame*)NULL,
					"Please select one or more files.",
					"","","",
					wxFILE_MUST_EXIST |
					wxMULTIPLE,wxDefaultPosition);
	if(dlgMany->ShowModal() == wxID_OK) {
		dlgMany->GetPaths(paths);

		register Pt list ;
		int n = paths.Count() ;
		// CheckFreeSpaceOnStacks(2 * n) ;
		list = tNilAtom ;
		while( n-- ) {
			PushH(MakeTempAtom(cCharPt(paths.Item(n).c_str()))) ;
			PushH(list) ;
			list = TagList(H - 2) ;
		}
		return list ;
	}
	else return nil ;
}

Pt getMultipleFilesMessg(const char *msg) {
	wxArrayString paths;
	wxFileDialog *dlgMany=new wxFileDialog((wxFrame*)NULL, msg,"","","",
							wxFILE_MUST_EXIST |	wxMULTIPLE,
							wxDefaultPosition);
	if(dlgMany->ShowModal() == wxID_OK) {

		dlgMany->GetPaths(paths);

		register Pt list ;
		int n = paths.Count() ;
		// CheckFreeSpaceOnStacks(2 * n) ;
		list = tNilAtom ;
		while( n-- ) {
			PushH(MakeTempAtom(cCharPt(paths.Item(n).c_str()))) ;
			PushH(list) ;
			list = TagList(H - 2) ;
		}
		return list ;
	}
	else
		return nil ;
}

Pt getDirectory(const char *msg) {
	wxDirDialog *dlgDir=new wxDirDialog((wxFrame*)NULL, msg,wxGetCwd());

	if(dlgDir->ShowModal() == wxID_OK)
		return MakeTempAtom(cCharPt(dlgDir->GetPath().c_str()));
	else
		return nil;
}

void doAlert(const char *msg) {
	wxMessageDialog *dlgAlert = new wxMessageDialog((wxFrame*)NULL, msg,
													"Message Box",
													wxOK | wxICON_INFORMATION,
													wxDefaultPosition);
	dlgAlert->ShowModal();
}

int doChoice(const char *msg, int boolOK, int boolCancel, int boolYesNo) {
	/*Return codes:
		0 --> Error. It should never happen!
		1 --> OK
		2 --> CANCEL
		3 --> YES
		4 --> NO
	*/
	int result;
	long style = wxICON_INFORMATION ;

	/* NOTE: YesNo does not work with OK, but works with Cancel? */
	if(boolOK)
		style = style | wxOK;
	if(boolCancel)
		style = style | wxCANCEL;
	if(boolYesNo)
		style = style | wxYES_NO;

	wxMessageDialog *dlgAlert = new wxMessageDialog((wxFrame*)NULL, msg,
												"Message Box",style,
												wxDefaultPosition);
	result = dlgAlert->ShowModal();

	if(result == wxID_OK) {
		return 1;
	}
	else if(result == wxID_CANCEL) {
		return 2;
	}
	else if(result == wxID_YES) {
		return 3;
	}
	else if(result == wxID_NO) {
		return 4;
	}
	else
		return 0;
	/*Observed results (in wxWidgets docs, this is *NOT* mentioned) */
	/*parameter(s) -> result(s):	*/
	/*OK				-> OK			*/
	/*CANCEL			-> OK			*/
	/*YESNO				-> YES NO		*/
	/*OK CANCEL			-> OK  CANCEL	*/
	/*OK YESNO			-> OK			*/
	/*YESNO CANCEL		-> YES NO CANCEL*/
	/*YESNO CANCEL	OK	-> OK CANCEL	*/
}

Pt GetText(const char *msg) {
	wxTextEntryDialog *dlgText = new wxTextEntryDialog((wxFrame*)NULL, msg,
													"Please enter text","",
													wxOK | wxCENTRE,
													wxDefaultPosition);

	if(dlgText->ShowModal() == wxID_OK)
		return MakeTempAtom(cCharPt(dlgText->GetValue().c_str()));
	else
		return nil;
}

/* Global variables for communication between each predicate's functions */
static AtomPt a0, a1;
static Pt t, t0;
static PInt result;
static int boolOK, boolCANCEL, boolYESNO;

/* gui_file_chooser_simple_msg */
static void PGuiFileChooserSimpleMsgINIT() {
	/*-->if the first argument is 'save', it creates a file dialog with the
		text 'save' on the top bar and 'save' written on the main button.
	  -->for other thing different from 'save', it writes the text on text on
	    the bar and 'load' on the principal button.
	*/
	a0 = XTestAtom(X0);
}
static void PGuiFileChooserSimpleMsgGUI() {
	if(!strcmp(AtomName(a0),"save"))
		t = getFileMessg(1,AtomName(a0));
	else
		t = getFileMessg(0,AtomName(a0));
}
static void PGuiFileChooserSimpleMsgEND() {
	if(t == nil)
		DoFail();
	MustBe( UnifyWithAtomic(X1, t) ) ;
}

/* gui_file_chooser_multiple_simple */
static void PGuiFileChooserMultipleSimpleINIT() {}
static void PGuiFileChooserMultipleSimpleGUI() {
	t = getMultipleFiles();
}
static void PGuiFileChooserMultipleSimpleEND() {
	if(t == nil)
		DoFail();
	MustBe( Unify(X0, t) ) ;
}

/* gui_file_chooser_multiple_msg */
static void PGuiFileChooserMultipleMsgINIT() {
	a0 = XTestAtom(X0);
}
static void PGuiFileChooserMultipleMsgGUI() {
	t = getMultipleFilesMessg(AtomName(a0));
}
static void PGuiFileChooserMultipleMsgEND() {
	if( t == nil )
		DoFail();
	MustBe( Unify(X1, t) ) ;
}

/* gui_directory_chooser_msg */
static void PGuiDirectoryChooserINIT() {
	a0 = XTestAtom(X0);
}
static void PGuiDirectoryChooserGUI() {
	t = getDirectory(AtomName(a0));
}
static void PGuiDirectoryChooserEND() {
	if( t == nil )
		DoFail();
	MustBe( Unify(X1, t) ) ;
}

/* gui_alert */
static void PGuiAlertINIT() {
	a0 = XTestAtom(X0);
}
static void PGuiAlertGUI() {
	doAlert(AtomName(a0));
}
static void PGuiAlertEND() {}

/* gui_choice */
static void PGuiChoiceINIT() {
	a0 = XTestAtom(X0);
}
static void PGuiChoiceGUI() {
	result = doChoice(AtomName(a0), 0, 0, 1);
}
static void PGuiChoiceEND() {
	if(result == 3) {
		MustBe( UnifyWithAtomic(X1, MakeTempAtom("yes")) ) ;
	}
	else { /*result == 4*/
		MustBe( UnifyWithAtomic(X1, MakeTempAtom("no")) ) ;
	}
	/*	3 --> YES
		4 --> NO*/
}

/* gui_choice_list */
static void PGuiChoiceListINIT() {
	t0 = Drf(X0) ;
	a1 = XTestAtom(X1);
	boolOK = 0;
	boolCANCEL = 0;
	boolYESNO = 0;
}
static void PGuiChoiceListGUI() {
	if(IsList(t0)) {
		for( t0 = Drf(t0) ; IsList(t0) ; t0 = Drf(XListTail(t0)) ) {
			AtomPt name = XTestAtom(Drf(XListHead(t0)));
			if(strcmp(AtomName(name),"yesno")==0) {
				boolYESNO=1;
				continue;
			}
			if(strcmp(AtomName(name),"ok")==0) {
				boolOK=1;
				continue;
			}
			if(strcmp(AtomName(name),"cancel")==0) {
				boolCANCEL=1;
				continue;
			}
		}
	}

	if(boolOK + boolCANCEL + boolYESNO == 0)//default!
		result=doChoice("default", 1, 0, 0);
	else
		result=doChoice(AtomName(a1), boolOK, boolCANCEL, boolYESNO);
}
static void PGuiChoiceListEND() {
	/*doChoice returns:
			0 --> Error. It should never happen!
			1 --> OK
			2 --> CANCEL
			3 --> YES
			4 --> NO
	*/
	if(result == 1) {
		MustBe( UnifyWithAtomic(X1, MakeTempAtom("ok")) ) ;
	}
	else if (result == 2) {
		MustBe( UnifyWithAtomic(X1, MakeTempAtom("cancel")) ) ;
	}
	else if (result == 3) {
		MustBe( UnifyWithAtomic(X1, MakeTempAtom("yes")) ) ;
	}
	else /*result == 4*/ {
		MustBe( UnifyWithAtomic(X1, MakeTempAtom("no")) ) ;
	}
}

/* gui_choice_yes_no_cancel */
static void PGuiChoiceYesNoCancelINIT() {
	a0 = XTestAtom(X0);
}
static void PGuiChoiceYesNoCancelGUI() {
	result = doChoice(AtomName(a0), 0, 1, 1);
}
static void PGuiChoiceYesNoCancelEND() {
	if (result == 2) {
		MustBe( UnifyWithAtomic(X1, MakeTempAtom("cancel")) ) ;
	}
	else if (result == 3) {
		MustBe( UnifyWithAtomic(X1, MakeTempAtom("yes")) ) ;
	}
	else /*result == 4*/ {
		MustBe( UnifyWithAtomic(X1, MakeTempAtom("no")) ) ;
	}
}

/* gui_choice_ok_cancel */
static void PGuiChoiceOkCancelINIT() {
	a0 = XTestAtom(X0);
}
static void PGuiChoiceOkCancelGUI() {
	result = doChoice(AtomName(a0), 1, 1, 0);
}
static void PGuiChoiceOkCancelEND() {
	if(result == 1) {
		MustBe( UnifyWithAtomic(X1, MakeTempAtom("ok")) ) ;
	}
	else /*result == 2*/ {
		MustBe( UnifyWithAtomic(X1, MakeTempAtom("cancel")) ) ;
	}
}

/* gui_get_text */
static void PGuiGetTextINIT() {
	a0 = XTestAtom(X0);

}
static void PGuiGetTextGUI() {
	t = GetText(AtomName(a0));
}
static void PGuiGetTextEND() {
	if( t == nil )
		DoFail();
	MustBe( UnifyWithAtomic(X1, t) ) ;
}


/* GRAPHICAL WINDOW PREDICATES */

/* Global variables for communication between each predicate's functions */
static PInt x,y,x0,y0,x1,y1,width,height,radius,r,g,b;
static WxGraphicalFrame *gframe;
static WxGraphicalObject *obj;
static wxArrayString items;

/* gui_gfx_create(+Title,+X,+Y,+Width,+Length,-Window) */
static void PGuiGfxCreateINIT() {
	a0 = XTestAtom(X0);
	x = XTestInt(X1);
	y = XTestInt(X2);
	width = XTestInt(X3);
	height = XTestInt(X4);
}
static void PGuiGfxCreateGUI() {
	gframe = new WxGraphicalFrame(AtomName(a0),wxPoint(x,y),wxSize(width,height));
}
static void PGuiGfxCreateEND() {
	BindVarWithExtra(X5, LookupWxObj(gframe));
}

/* gui_gfx_putpixel(+Window,+X,+Y,+R,+G,+B,-Pixel) */
static void PGuiGfxPutPixelINIT() {
	gframe = (WxGraphicalFrame *)XTestWxObj(X0);
	x = XTestInt(X1);
	y = XTestInt(X2);
	r = XTestInt(X3);
	g = XTestInt(X4);
	b = XTestInt(X5);
}
static void PGuiGfxPutPixelGUI() {
	obj = gframe->gfxPutPixel(x, y, r, g, b);
}
static void PGuiGfxPutPixelEND() {
	BindVarWithExtra(X6, LookupWxObj(obj));
}

/* gui_gfx_line(+Window,+X0,+Y0,+X1,+Y1,+R,+G,+B,-Line) */
static void PGuiGfxLineINIT() {
	gframe = (WxGraphicalFrame *)XTestWxObj(X0);
	x0 = XTestInt(X1);
	y0 = XTestInt(X2);
	x1 = XTestInt(X3);
	y1 = XTestInt(X4);
	r = XTestInt(X5);
	g = XTestInt(X6);
	b = XTestInt(X7);
}
static void PGuiGfxLineGUI() {
	obj = gframe->gfxLine(x0,y0,x1,y1,r,g,b);
}
static void PGuiGfxLineEND() {
	BindVarWithExtra(Xc(8), LookupWxObj(obj));
}

/* gui_gfx_circle(+Window,+X,+Y,+Radius,+R,+G,+B,-Circle) */
static void PGuiGfxCircleINIT() {
	gframe = (WxGraphicalFrame *)XTestWxObj(X0);
	x = XTestInt(X1) ;
	y = XTestInt(X2) ;
	radius = XTestInt(X3) ;
	r = XTestInt(X4) ;
	g = XTestInt(X5) ;
	b = XTestInt(X6) ;
}
static void PGuiGfxCircleGUI() {
	obj = gframe->gfxCircle(x,y,radius,r,g,b);
}
static void PGuiGfxCircleEND() {
	BindVarWithExtra(X7, LookupWxObj(obj));
}

/* gui_gfx_rectangle(+Window,+X0,+Y0,+X1,+Y1,+R,+G,+B,-Rectangle) */
static void PGuiGfxRectangleINIT() {
	gframe = (WxGraphicalFrame *)XTestWxObj(X0);
	x = XTestInt(X1) ;
	y = XTestInt(X2) ;
	width = XTestInt(X3) ;
	height = XTestInt(X4) ;
	r = XTestInt(X5) ;
	g = XTestInt(X6) ;
	b = XTestInt(X7) ;
}
static void PGuiGfxRectangleGUI() {
	obj = gframe->gfxRectangle(x,y,width,height,r,g,b);
}
static void PGuiGfxRectangleEND() {
	BindVarWithExtra(Xc(8), LookupWxObj(obj));
}

/* gui_gfx_circle_filled(+Window,+X,+Y,+Radius,+R,+G,+B,-Circle) */
static void PGuiGfxCircleFilledINIT() {
	gframe = (WxGraphicalFrame *)XTestWxObj(X0);
	x = XTestInt(X1) ;
	y = XTestInt(X2) ;
	radius = XTestInt(X3) ;
	r = XTestInt(X4) ;
	g = XTestInt(X5) ;
	b = XTestInt(X6) ;
}
static void PGuiGfxCircleFilledGUI() {
	obj = gframe->gfxCircleFilled(x,y,radius,r,g,b);
}
static void PGuiGfxCircleFilledEND() {
	BindVarWithExtra(X7, LookupWxObj(obj));
}

/* gui_gfx_rectangle_filled(+Window,+X0,+Y0,+X1,+Y1,+R,+G,+B,-Rectangle) */
static void PGuiGfxRectangleFilledINIT() {
	gframe = (WxGraphicalFrame *)XTestWxObj(X0);
	x = XTestInt(X1) ;
	y = XTestInt(X2) ;
	width = XTestInt(X3) ;
	height = XTestInt(X4) ;
	r = XTestInt(X5) ;
	g = XTestInt(X6) ;
	b = XTestInt(X7) ;
}
static void PGuiGfxRectangleFilledGUI() {
	obj = gframe->gfxRectangleFilled(x,y,width,height,r,g,b);
}
static void PGuiGfxRectangleFilledEND() {
	BindVarWithExtra(Xc(8), LookupWxObj(obj));
}

/* gui_gfx_draw_text(+Window,+Text,+X,+Y,-Text) */
static void PGuiGfxDrawTextINIT() {
	gframe = (WxGraphicalFrame *)XTestWxObj(X0);
	a1 = XTestAtom(X1);
	x = XTestInt(X2) ;
	y = XTestInt(X3) ;
}
static void PGuiGfxDrawTextGUI() {
	obj = gframe->gfxDrawText(AtomName(a1),x,y);
}
static void PGuiGfxDrawTextEND() {
	BindVarWithExtra(X4, LookupWxObj(obj));
}

/* gui_gfx_button(+Window,+Text,+X,+Y,-Text) */
static void PGuiGfxButtonINIT() {
	gframe = (WxGraphicalFrame *)XTestWxObj(X0);
	a1 = XTestAtom(X1);
	x = XTestInt(X2) ;
	y = XTestInt(X3) ;
}
static void PGuiGfxButtonGUI() {
	obj = gframe->gfxButton(AtomName(a1),x,y);
}
static void PGuiGfxButtonEND() {
	BindVarWithExtra(X4, LookupWxObj(obj));
}

/* gui_gfx_list(+Window,+Items,+X,+Y,-List) */
static void PGuiGfxListINIT() {
	gframe = (WxGraphicalFrame *)XTestWxObj(X0);
	t0 = Drf(X1);
	x = XTestInt(X2) ;
	y = XTestInt(X3) ;

	if(IsList(t0)) {
		for(; IsList(t0); t0 = Drf(XListTail(t0))) {
			AtomPt item = XTestAtom(Drf(XListHead(t0)));
			items.Add(AtomName(item));
		}
	}
}
static void PGuiGfxListGUI() {
	obj = ((WxGraphicalFrame *)gframe)->gfxList(&items,x,y);
}
static void PGuiGfxListEND() {
	items.Clear();
	BindVarWithExtra(X4, LookupWxObj(obj));
}

/* gui_gfx_list_getitem(+List,-Item) */
static void PGuiGfxListGetItemINIT() {
	obj = (WxGraphicalObject *)XTestWxObj(X0);
}
static void PGuiGfxListGetItemGUI() {
	wxString string = (((WxList *)obj)->getSelectedItem());
	t = MakeTempAtom(cCharPt(string.c_str()));
}
static void PGuiGfxListGetItemEND() {
	MustBe( UnifyWithAtomic(X1, t) ) ;
}

/* gui_gfx_delete_object(+Window,+Object) */
static void PGuiGfxDeleteObjectINIT() {
	gframe = (WxGraphicalFrame *)XTestWxObj(X0);
	obj = (WxGraphicalObject *)XTestWxObj(X1);
}
static void PGuiGfxDeleteObjectGUI() {
	gframe->gfxDeleteObject(obj);
}
static void PGuiGfxDeleteObjectEND() {}

/* gui_gfx_clear(+Window) */
static void PGuiGfxClearINIT() {
	gframe = (WxGraphicalFrame *)XTestWxObj(X0);
}
static void PGuiGfxClearGUI() {
	gframe->gfxClear();
}
static void PGuiGfxClearEND() {}

/* gui_gfx_close(+Window) */
static void PGuiGfxCloseINIT() {
	gframe = (WxGraphicalFrame *)XTestWxObj(X0);
}
static void PGuiGfxCloseGUI() {
	gframe->gfxClose();
}
static void PGuiGfxCloseEND() {}


/* TEXT WINDOW PREDICATES */

/* Global variables for communication between each predicate's functions */
static Pt menus, tin;
static PInt from, to;
static WxTextFrame *tframe;
static MenuInfo menu_infos[20];
static int i;
static AtomPt a3;
static Pt txt;

/* gui_txt_create(+Title,+X,+Y,+Width,+Length,+Menus,-Window) */
static void PGuiTextCreateINIT() {
	a0 = XTestAtom(X0);
	x = XTestInt(X1);
	y = XTestInt(X2);
	width = XTestInt(X3);
	height = XTestInt(X4);
	menus = Drf(X5);
	t0 = Drf(menus);
	i = 0;
	if(IsList(t0)) {
		for( t0 = Drf(t0) ; IsList(t0) ; t0 = Drf(XListTail(t0)) ) {
			tin = Drf(XListHead(t0));
			if(IsList(tin)) {
				AtomPt menuName = XTestAtom(Drf(XListHead(tin)));
				menu_infos[i].menu_name << AtomName(menuName);

				tin = Drf(XListTail(tin));
				for(tin = Drf(tin) ; IsList(tin) ; tin = Drf(XListTail(tin))) {
					AtomPt name = XTestAtom(Drf(XListHead(tin)));
					menu_infos[i].menu_elements.Add(AtomName(name));
				}
				i++;
			}
		}
	}

}
static void PGuiTextCreateGUI() {
	tframe = new WxTextFrame(AtomName(a0),x,y,width,height,menu_infos,i);
}
static void PGuiTextCreateEND() {
	BindVarWithExtra(X6, LookupWxObj(tframe));
}

/* gui_text_get_file_path(+Window,-FilePath)*/
static void PGuiTextGetFilePathINIT() {
	tframe = (WxTextFrame *)XTestWxObj(X0);
}
static void PGuiTextGetFilePathGUI() {
	const wxString &string = tframe->txtEditorGetFilePath();
	txt = MakeTempAtom(cCharPt(string.c_str()));
}
static void PGuiTextGetFilePathEND() {
	if(txt == nil )
		DoFail();

	MustBe(Unify(X1, txt)) ;
}

/* gui_text_open_file(+Window) */
static void PGuiTextOpenFileINIT() {
	tframe = (WxTextFrame *)XTestWxObj(X0);
}
static void PGuiTextOpenFileGUI() {
	result = tframe->txtOpenFile();
}
static void PGuiTextOpenFileEND() {
	if(result) {
		if(result == WX_RETURN_CANCEL)
			DoFail();
	}
	else
		TypeError("TextWindow", (Pt)tframe) ;
}

/* gui_text_save_file(+Window) */
static void PGuiTextSaveFileINIT() {
	tframe = (WxTextFrame *)XTestWxObj(X0);
}
static void PGuiTextSaveFileGUI() {
	result = tframe->txtSave();
}
static void PGuiTextSaveFileEND() {
	if(result) {
		if(result == WX_RETURN_CANCEL)
			DoFail();
	}
	else
		TypeError("TextWindow", (Pt)tframe) ;
}

/* gui_text_save_file_as(+Window) */
static void PGuiTextSaveFileAsINIT() {
	tframe = (WxTextFrame *)XTestWxObj(X0);
}
static void PGuiTextSaveFileAsGUI() {
	result = tframe->txtSaveAs();
}
static void PGuiTextSaveFileAsEND() {
	if(result) {
		if(result == WX_RETURN_CANCEL)
			DoFail();
	}
	else
		TypeError("TextWindow", (Pt)tframe) ;
}

/* gui_text_get_text(+Window,+From,+To,-Text) */
static void PGuiTextGetTextINIT() {
	tframe = (WxTextFrame *)XTestWxObj(X0);
	from = XTestInt(X1);
	to = XTestInt(X2);
}
static void PGuiTextGetTextGUI() {
	const wxString &string = tframe->txtEditorGetText(from,to);
	txt = MakeTempAtom(cCharPt(string.c_str()));
}
static void PGuiTextGetTextEND() {
	if(txt == nil)
		DoFail();

	MustBe(Unify(X3, txt)) ;
}

/* gui_text_append(+Window,+Text) */
static void PGuiTextAppendINIT() {
	tframe = (WxTextFrame *)XTestWxObj(X0);
	a1 = XTestAtom(X1);
}
static void PGuiTextAppendGUI() {
	tframe->txtAppendText(AtomName(a1));
}
static void PGuiTextAppendEND() {}

/* gui_text_get_selected_text(+Window,-Text) */
static void PGuiTextGetSelectedTextINIT() {
	tframe = (WxTextFrame *)XTestWxObj(X0);
}
static void PGuiTextGetSelectedTextGUI() {
	const wxString &string = tframe->txtGetSelectedText();
	txt = MakeTempAtom(cCharPt(string.c_str()));
}
static void PGuiTextGetSelectedTextEND() {
	if( txt == nil )
		DoFail();

	MustBe( Unify(X1, txt) ) ;
}

/* gui_text_replace(+Window,+From,+To,+Text) */
static void PGuiTextReplaceINIT() {
	tframe = (WxTextFrame *)XTestWxObj(X0);
	from = XTestInt(X1);
	to = XTestInt(X2);
	a3 = XTestAtom(X3);
}
static void PGuiTextReplaceGUI() {
	tframe->txtReplaceText(from, to, AtomName(a3));
}
static void PGuiTextReplaceEND() {}

/* gui_text_set_selection(+Window,+From,+To) */
static void PGuiTextSetSelectionINIT() {
	tframe = (WxTextFrame *)XTestWxObj(X0);
	from = XTestInt(X1);
	to = XTestInt(X2);
}
static void PGuiTextSetSelectionGUI() {
	tframe->txtSetSelection(from, to);
}
static void PGuiTextSetSelectionEND() {}

/* gui_text_close(+Window) */
static void PGuiTextCloseINIT() {
	tframe = (WxTextFrame *)XTestWxObj(X0);
}
static void PGuiTextCloseGUI() {
	tframe->txtClose();
}
static void PGuiTextCloseEND() {}

#endif 
	
/* Init: predicate installation */
void WxAddOnsInit() {
	/* GENERAL PURPOSE PREDICATES */
	InstallWxGuiBuiltinPred("wxgui_file_chooser_simple_msg", 2, PGuiFileChooserSimpleMsg) ;
	InstallWxGuiBuiltinPred("wxgui_file_chooser_multiple_simple", 1, PGuiFileChooserMultipleSimple) ;
	InstallWxGuiBuiltinPred("wxgui_file_chooser_multiple_msg", 2, PGuiFileChooserMultipleMsg) ;
	InstallWxGuiBuiltinPred("wxgui_directory_chooser_msg", 2, PGuiDirectoryChooser) ;
	InstallWxGuiBuiltinPred("wxgui_alert", 1, PGuiAlert) ;
	InstallWxGuiBuiltinPred("wxgui_choice", 2, PGuiChoice) ;
	InstallWxGuiBuiltinPred("wxgui_choice_list", 3, PGuiChoiceList) ;
	InstallWxGuiBuiltinPred("wxgui_choice_yes_no_cancel", 2, PGuiChoiceYesNoCancel) ;
	InstallWxGuiBuiltinPred("wxgui_choice_ok_cancel", 2, PGuiChoiceOkCancel) ;
	InstallWxGuiBuiltinPred("wxgui_get_text", 2, PGuiGetText) ;

	/* GRAPHICAL WINDOW PREDICATES */
	InstallWxGuiBuiltinPred("wxgui_gfx_create", 6, PGuiGfxCreate) ;
	InstallWxGuiBuiltinPred("wxgui_gfx_putpixel", 7, PGuiGfxPutPixel) ;
	InstallWxGuiBuiltinPred("wxgui_gfx_line", 9, PGuiGfxLine) ;
	InstallWxGuiBuiltinPred("wxgui_gfx_circle", 8, PGuiGfxCircle) ;
	InstallWxGuiBuiltinPred("wxgui_gfx_rectangle", 9, PGuiGfxRectangle) ;
	InstallWxGuiBuiltinPred("wxgui_gfx_circle_filled", 8, PGuiGfxCircleFilled) ;
	InstallWxGuiBuiltinPred("wxgui_gfx_rectangle_filled", 9, PGuiGfxRectangleFilled) ;
	InstallWxGuiBuiltinPred("wxgui_gfx_draw_text", 5, PGuiGfxDrawText) ;
	InstallWxGuiBuiltinPred("wxgui_gfx_button", 5, PGuiGfxButton) ;
	InstallWxGuiBuiltinPred("wxgui_gfx_list", 5, PGuiGfxList) ;
	InstallWxGuiBuiltinPred("wxgui_gfx_list_getitem", 2, PGuiGfxListGetItem) ;
	InstallWxGuiBuiltinPred("wxgui_gfx_delete_object", 2, PGuiGfxDeleteObject) ;
	InstallWxGuiBuiltinPred("wxgui_gfx_clear", 1, PGuiGfxClear) ;
	InstallWxGuiBuiltinPred("wxgui_gfx_close", 1, PGuiGfxClose) ;

	/* TEXT WINDOW PREDICATES */
	InstallWxGuiBuiltinPred("wxgui_text_create", 7, PGuiTextCreate) ;
	InstallWxGuiBuiltinPred("wxgui_text_get_file_path", 2, PGuiTextGetFilePath) ;
	InstallWxGuiBuiltinPred("wxgui_text_open_file", 1, PGuiTextOpenFile) ;
	InstallWxGuiBuiltinPred("wxgui_text_save_file", 1, PGuiTextSaveFile) ;
	InstallWxGuiBuiltinPred("wxgui_text_save_file_as", 1, PGuiTextSaveFileAs) ;
	InstallWxGuiBuiltinPred("wxgui_text_get_text", 4, PGuiTextGetText) ;
	InstallWxGuiBuiltinPred("wxgui_text_append", 2, PGuiTextAppend) ;
	InstallWxGuiBuiltinPred("wxgui_text_get_selected_text", 2, PGuiTextGetSelectedText) ;
	InstallWxGuiBuiltinPred("wxgui_text_replace", 4, PGuiTextReplace) ;
	InstallWxGuiBuiltinPred("wxgui_text_set_selection", 3, PGuiTextSetSelection) ;
	InstallWxGuiBuiltinPred("wxgui_text_close", 1, PGuiTextClose) ;
}

#else /* USE_WXWIDGETS */

void WxAddOnsInit() {
	/* Nothing */
}

#endif /* USE_WXWIDGETS */
