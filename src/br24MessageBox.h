/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Navico BR24 Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2013 by Kees Verruijt         canboat@verruijt.net *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#ifndef _BR24MESSAGEBOX_H_
#define _BR24MESSAGEBOX_H_

#include "br24radar_pi.h"

class br24MessageBox : public wxDialog
{
    DECLARE_CLASS(br24MessageBox)
    DECLARE_EVENT_TABLE()

public:

    br24MessageBox();

    ~br24MessageBox();
    void Init();

    bool Create(wxWindow *parent, br24radar_pi *pi, wxWindowID id = wxID_ANY,
        const wxString& caption = _("Radar"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE & ~(wxMAXIMIZE_BOX)
        );

    void CreateControls();
    void UpdateMessage(bool haveOpenGL, bool haveGPS, bool haveHeading, bool haveVariation, bool haveRadar, bool haveData);
    void SetErrorMessage(wxString &msg);
    void SetRadarIPAddress(wxString &msg);
    void SetMcastIPAddress(wxString &msg);
    void SetHeadingInfo(wxString &msg);
    void SetVariationInfo(wxString &msg);
    void SetRadarInfo(wxString &msg);
    wxBoxSizer        *topSizeM;

private:
    void OnClose(wxCloseEvent& event);
    void OnIdOKClick(wxCommandEvent& event);
    void OnMove(wxMoveEvent& event);
    void OnSize(wxSizeEvent& event);

    void OnMessageBackButtonClick(wxCommandEvent& event);

    wxWindow           *m_parent;
    br24radar_pi       *m_pi;
    wxBoxSizer         *nmeaSizer;
    wxBoxSizer         *infoSizer;


    wxBoxSizer         *messageBox;   // Contains NO HDG and/or NO GPS
    wxStaticBox        *ipBox;
    wxStaticBox        *nmeaBox;
    wxStaticBox        *infoBox;

    bool                wantShowMessage; // If true, don't hide messagebox automatically

    // MessageBox
    wxButton           *bMsgBack;
    wxStaticText       *tMessage;
    wxStaticText       *offMessage;
    wxCheckBox         *cbOpenGL;
    wxCheckBox         *cbBoatPos;
    wxCheckBox         *cbHeading;
    wxCheckBox         *cbVariation;
    wxCheckBox         *cbRadar;
    wxCheckBox         *cbData;
    wxStaticText       *tStatistics;

};

#endif

// vim: sw=4:ts=8: