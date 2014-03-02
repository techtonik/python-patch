/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
 * $Revision: 4909 $
 * $Id: conf.cpp 4909 2008-02-27 13:15:26Z mortenmacfly $
 * $HeadURL: http://svn.berlios.de/svnroot/repos/codeblocks/tags/8.02/src/plugins/contrib/devpak_plugin/conf.cpp $
 */

#include "conf.h"
#include <wx/intl.h>
#include <wx/url.h>
#include <wx/filename.h>
#include <globals.h>

wxString g_MasterPath;

wxString GetSizeString(int bytes)
{
    wxString ret;
    float kilobytes = (float)bytes / 1024.0f;
    float megabytes = kilobytes / 1024.0f;
    if (megabytes >= 1.0f)
        ret.Printf(_("%.2f MB"), megabytes);
    else if (kilobytes >= 1.0f)
        ret.Printf(_("%.2f KB"), kilobytes);
    else
        ret.Printf(_("%ld bytes"), bytes);
    return ret;
}

UpdateRec* ReadConf(const IniParser& ini, int* recCount, const wxString& currentServer, const wxString& appPath)
{
    *recCount = 0;
    int groupsCount = ini.GetGroupsCount();
    if (groupsCount == 0)
        return 0;

    UpdateRec* list = new UpdateRec[ini.GetGroupsCount()];
    for (int i = 0; i < groupsCount; ++i)
    {
    	UpdateRec& rec = list[i];

    	rec.title = ini.GetGroupName(i);

    	// fix title
    	// devpaks.org has changed the title to contain some extra info
        // e.g.: [libunicows   Library version: 1.1.1   Devpak revision: 1sid]
    	// we don't need this extra info, so if we find it we remove it
    	int pos = rec.title.Find(_T("Library version:"));
    	if (pos != -1)
    	{
    		rec.title.Truncate(pos);
    		rec.title = rec.title.Trim(false);
    		rec.title = rec.title.Trim(true);
    	}

    	rec.name = ini.GetKeyValue(i, _T("Name"));
    	rec.desc = ini.GetKeyValue(i, _T("Description"));
    	rec.remote_file = ini.GetKeyValue(i, _T("RemoteFilename"));
    	rec.local_file = ini.GetKeyValue(i, _T("LocalFilename"));
    	rec.groups = GetArrayFromString(ini.GetKeyValue(i, _T("Group")), _T(","));
    	rec.install = ini.GetKeyValue(i, _T("InstallPath"));
    	rec.version = ini.GetKeyValue(i, _T("Version"));
        ini.GetKeyValue(i, _T("Size")).ToLong(&rec.bytes);
    	rec.date = ini.GetKeyValue(i, _T("Date"));
    	rec.installable = ini.GetKeyValue(i, _T("Execute")) == _T("1");

        // read .entry file (if exists)
        rec.entry = (!rec.name.IsEmpty() ? rec.name : wxFileName(rec.local_file).GetName()) + _T(".entry");
        IniParser p;
        p.ParseFile(appPath + rec.entry);
        rec.installed_version = p.GetValue(_T("Setup"), _T("AppVersion"));

        rec.downloaded = wxFileExists(appPath + _T("/") + rec.local_file);
        rec.installed = !rec.installed_version.IsEmpty();

        // calculate size
        rec.size = GetSizeString(rec.bytes);

        // fix-up
        if (rec.name.IsEmpty())
            rec.name = rec.title;
        rec.desc.Replace(_T("<CR>"), _T("\n"));
        rec.desc.Replace(_T("<LF>"), _T("\r"));
        wxURL url(rec.remote_file);
        if (!url.GetServer().IsEmpty())
        {
            rec.remote_server = url.GetScheme() + _T("://") + url.GetServer();
            int pos = rec.remote_file.Find(url.GetServer());
            if (pos != wxNOT_FOUND)
                rec.remote_file.Remove(0, pos + url.GetServer().Length() + 1);
        }
        else
            rec.remote_server = currentServer;
    }

    *recCount = groupsCount;
    return list;
}

UpdateRec* FindRecByTitle(const wxString& title, UpdateRec* list, int count)
{
    for (int i = 0; i < count; ++i)
    {
        if (list[i].title == title)
            return &list[i];
    }
    return 0;
}
