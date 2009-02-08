#ifndef CONF_H
#define CONF_H

#include <wx/string.h>
#include <wx/dynarray.h>
#include "cbiniparser.h"

struct UpdateRec
{
    wxString entry;
    wxString title;
    wxString name;
    wxString desc;
    wxString remote_server;
    wxString remote_file;
    wxString local_file;
    wxArrayString groups;
    wxString install;
    wxString version;
    wxString installed_version;
    long int bytes;
    float kilobytes;
    float megabytes;
    wxString size;
    wxString date;
    bool installable;
    bool downloaded;
    bool installed;
};

extern wxString g_MasterPath;

UpdateRec* ReadConf(const IniParser& ini, int* recCount, const wxString& currentServer, const wxString& appPath);
UpdateRec* FindRecByTitle(const wxString& title, UpdateRec* list, int count);
// utility
wxString GetSizeString(int bytes);

#endif // CONF_H
