/*
  mkvmerge GUI -- utility for splicing together matroska files
      from component media subtypes

  mux_dialog.h

  Written by Moritz Bunkus <moritz@bunkus.org>
  Parts of this code were written by Florian Wager <root@sirelvis.de>

  Distributed under the GPL
  see the file COPYING for details
  or visit http://www.gnu.org/copyleft/gpl.html
*/

/*!
    \file
    \version $Id$
    \brief muxing dialog
    \author Moritz Bunkus <moritz@bunkus.org>
*/

#ifndef __MUX_DIALOG_H
#define __MUX_DIALOG_H

#include "wx/dialog.h"
#include "wx/process.h"

#define ID_B_MUX_OK                       17000
#define ID_B_MUX_SAVELOG                  17001
#define ID_B_MUX_ABORT                    17002
#define ID_B_MUX_MINIMIZE                 17003

class mux_dialog: public wxDialog {
  DECLARE_CLASS(mux_dialog);
  DECLARE_EVENT_TABLE();
protected:
  long pid;
  wxStaticText *st_label;
  wxGauge *g_progress;
  wxProcess *process;
  wxString log, opt_file_name;
  wxButton *b_ok, *b_save_log, *b_abort;
  wxTextCtrl *tc_output, *tc_warnings, *tc_errors;
public:

  mux_dialog(wxWindow *parent);
  ~mux_dialog();

  void update_window(wxString text);
  void update_gauge(long value);

  void on_ok(wxCommandEvent &evt);
  void on_save_log(wxCommandEvent &evt);
  void on_abort(wxCommandEvent &evt);
  void on_minimize(wxCommandEvent &evt);
};

class mux_process: public wxProcess {
public:
  mux_dialog *dlg;

  mux_process(mux_dialog *mdlg);
  virtual void OnTerminate(int pid, int status);
};

#endif // __MUX_DIALOG_H
