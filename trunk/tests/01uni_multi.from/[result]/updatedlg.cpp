/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
 * $Revision: 4909 $
 * $Id: updatedlg.cpp 4909 2008-02-27 13:15:26Z mortenmacfly $
 * $HeadURL: http://svn.berlios.de/svnroot/repos/codeblocks/tags/8.02/src/plugins/contrib/devpak_plugin/updatedlg.cpp $
 */

#include "updatedlg.h"
#include <wx/xrc/xmlres.h>
#include <wx/msgdlg.h>
#include <wx/button.h>
#include <wx/gauge.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/file.h>
#include <wx/menu.h>
#include "devpakinstaller.h"
#include "crc32.h"

#include "manager.h"
#include "configmanager.h"
#include "globals.h"

int idNet = wxNewId();
int idPopupInstall = wxNewId();
int idPopupDownload = wxNewId();
int idPopupDownloadAndInstall = wxNewId();
int idPopupUninstall = wxNewId();

BEGIN_EVENT_TABLE(UpdateDlg, wxDialog)
    EVT_UPDATE_UI(-1, UpdateDlg::OnUpdateUI)
    EVT_TREE_SEL_CHANGED(XRCID("tvCategories"), UpdateDlg::OnTreeSelChanged)
    EVT_LIST_ITEM_SELECTED(XRCID("lvFiles"), UpdateDlg::OnFileSelected)
    EVT_LIST_ITEM_DESELECTED(XRCID("lvFiles"), UpdateDlg::OnFileDeSelected)
    EVT_LIST_ITEM_RIGHT_CLICK(XRCID("lvFiles"), UpdateDlg::OnFileRightClick)
    EVT_MENU(idPopupDownload, UpdateDlg::OnDownload)
    EVT_MENU(idPopupDownloadAndInstall, UpdateDlg::OnDownloadAndInstall)
    EVT_MENU(idPopupInstall, UpdateDlg::OnInstall)
    EVT_MENU(idPopupUninstall, UpdateDlg::OnUninstall)
    EVT_COMBOBOX(XRCID("cmbServer"), UpdateDlg::OnServerChange)
    EVT_COMBOBOX(XRCID("cmbFilter"), UpdateDlg::OnFilterChange)
    EVT_CHECKBOX(XRCID("chkCache"), UpdateDlg::OnServerChange)
    EVT_CBNET_CONNECT(idNet, UpdateDlg::OnConnect)
    EVT_CBNET_DISCONNECT(idNet, UpdateDlg::OnDisConnect)
    EVT_CBNET_PROGRESS(idNet, UpdateDlg::OnProgress)
    EVT_CBNET_ABORTED(idNet, UpdateDlg::OnAborted)
    EVT_CBNET_START_DOWNLOAD(idNet, UpdateDlg::OnDownloadStarted)
    EVT_CBNET_END_DOWNLOAD(idNet, UpdateDlg::OnDownloadEnded)
END_EVENT_TABLE()

UpdateDlg::UpdateDlg(wxWindow* parent)
    : m_Recs(0),
    m_RecsCount(0),
    m_CurrFileSize(0),
    m_LastBlockSize(0),
    m_HasUpdated(false),
    m_FirstTimeCheck(true),
    m_Net(this, idNet, _T("http://devpaks.sourceforge.net/"))
{
	//ctor
	wxXmlResource::Get()->LoadDialog(this, parent, _T("MainFrame"));
	CreateListColumns();
    FillServers();
	UpdateStatus(_("Ready"), 0);
}

UpdateDlg::~UpdateDlg()
{
	//dtor
	delete[] m_Recs;
	m_RecsCount = 0;
}

void UpdateDlg::EndModal(int retCode)
{
    if (!m_Net.IsConnected() || retCode != wxID_CANCEL)
    {
        wxDialog::EndModal(retCode);
        return;
    }

    if (m_Net.IsConnected())
        m_Net.Abort();
}

void UpdateDlg::CreateListColumns()
{
    wxListCtrl* lst = XRCCTRL(*this, "lvFiles", wxListCtrl);
    lst->InsertColumn(0, _("Title"));
    lst->InsertColumn(1, _("Version"));
    lst->InsertColumn(2, _("Installed"));
    lst->InsertColumn(3, _("Size"), wxLIST_FORMAT_RIGHT);
    lst->InsertColumn(4, _("Rev"));

    lst->SetColumnWidth(0, lst->GetSize().x - (64 * 3 + 40) - 6 ); // 1st column takes all remaining space
    lst->SetColumnWidth(1, 64);
    lst->SetColumnWidth(2, 64);
    lst->SetColumnWidth(3, 64);
    lst->SetColumnWidth(4, 40);
}

void UpdateDlg::AddRecordToList(UpdateRec* rec)
{
    if (!rec)
        return;
    wxListCtrl* lst = XRCCTRL(*this, "lvFiles", wxListCtrl);
    int idx = lst->GetItemCount();
    lst->InsertItem(idx, rec->title);
    lst->SetItem(idx, 1, rec->version);
    lst->SetItem(idx, 2, rec->installed_version);
    lst->SetItem(idx, 3, rec->size);
    lst->SetItem(idx, 4, rec->revision);
}

wxString UpdateDlg::GetListColumnText(int idx, int col) {
    wxListCtrl* lst = XRCCTRL(*this, "lvFiles", wxListCtrl);
    int index = idx == -1 ? lst->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) : idx;
    wxListItem info;
    info.SetId(index);
    info.SetColumn(col);
    info.SetMask(wxLIST_MASK_TEXT);
    lst->GetItem(info);
    return info.GetText();
}

void UpdateDlg::SetListColumnText(int idx, int col, const wxString& text)
{
    wxListCtrl* lst = XRCCTRL(*this, "lvFiles", wxListCtrl);
    int index = idx == -1 ? lst->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) : idx;
    wxListItem it;
    it.m_itemId = index;
    it.m_col = col;
    it.m_mask = wxLIST_MASK_TEXT;
    it.m_text = text;
    lst->SetItem(it);
}

void UpdateDlg::UpdateStatus(const wxString& status, int curProgress, int maxProgress)
{
    wxStaticText* lbl = XRCCTRL(*this, "lblStatus", wxStaticText);
    if (lbl->GetLabel() != status)
        lbl->SetLabel(status);
    if (curProgress != -1)
        XRCCTRL(*this, "gauProgress", wxGauge)->SetValue(curProgress);
    if (maxProgress != -1)
        XRCCTRL(*this, "gauProgress", wxGauge)->SetRange(maxProgress);
}

void UpdateDlg::EnableButtons(bool update, bool abort)
{
    wxButton* btnCl = XRCCTRL(*this, "wxID_CANCEL", wxButton);

    btnCl->Enable(abort);
    // disable server list and cache checkbox while downloading
    XRCCTRL(*this, "cmbServer", wxComboBox)->Enable(!m_Net.IsConnected());
    XRCCTRL(*this, "chkCache", wxCheckBox)->Enable(!m_Net.IsConnected());

    wxYield();
}

void UpdateDlg::FillGroups()
{
    UpdateStatus(_("Parsing list of updates"), 0, m_RecsCount - 1);

    // get a list of unique group names
    wxArrayString groups;
    for (int i = 0; i < m_RecsCount; ++i)
    {
        for (unsigned int x = 0; x < m_Recs[i].groups.GetCount(); ++x)
        {
            if (m_Recs[i].groups[x].IsEmpty())
                continue;
            if (groups.Index(m_Recs[i].groups[x]) == wxNOT_FOUND)
            {
                if (FilterRec(&m_Recs[i]))
                    groups.Add(m_Recs[i].groups[x]);
            }
        }
    }

    // create the groups tree
    wxTreeCtrl* tree = XRCCTRL(*this, "tvCategories", wxTreeCtrl);
    tree->Freeze();
    tree->DeleteAllItems();
    wxTreeItemId root = tree->AddRoot(_("All categories"));
    for (unsigned int i = 0; i < groups.GetCount(); ++i)
    {
        tree->AppendItem(root, groups[i]);
    }
    tree->SortChildren(root);
    tree->Thaw();
    tree->Expand(root);
    tree->SelectItem(root); // this calls the event

    UpdateStatus(_("Done parsing list of updates"), 0);
}

void UpdateDlg::FillFiles(const wxTreeItemId& id)
{
    wxTreeCtrl* tree = XRCCTRL(*this, "tvCategories", wxTreeCtrl);
    wxListCtrl* lst = XRCCTRL(*this, "lvFiles", wxListCtrl);
    lst->Freeze();
    lst->ClearAll();
    CreateListColumns();

    wxString group = id == tree->GetRootItem() ? _T("") : tree->GetItemText(id);

    // add files belonging to group
    int counter = 0;
    for (int i = 0; i < m_RecsCount; ++i)
    {
        if (group.IsEmpty() || (!m_Recs[i].groups.IsEmpty() && m_Recs[i].groups.Index(group) != wxNOT_FOUND))
        {
            // filter
            if (FilterRec(&m_Recs[i]))
            {
                AddRecordToList(&m_Recs[i]);
                ++counter;
            }
        }
    }
    lst->Thaw();

    // select first item
    lst->SetItemState(0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
}

void UpdateDlg::FillFileDetails(const wxListItem& id)
{
    wxTextCtrl* txt = XRCCTRL(*this, "txtInfo", wxTextCtrl);
    txt->Clear();

    UpdateRec* cur = GetRecFromListView();
    if (!cur)
    {
        txt->Clear();
        EnableButtons();
        return;
    }
    txt->AppendText(_("Name: ") + cur->name + _T("\n"));
//    txt->AppendText(_("Server: ") + cur->remote_server + _T("\n"));
//    txt->AppendText(_("File: ") + cur->remote_file + _T("\n"));
    txt->AppendText(_("Version: ") + cur->version + _T("\n"));
    txt->AppendText(_("Size: ") + cur->size + _T("\n"));
    txt->AppendText(_("Date: ") + cur->date + _T("\n\n"));
    txt->AppendText(_("Description: \n"));
    txt->AppendText(cur->desc);

    txt->SetSelection(0, 0);
    txt->SetInsertionPoint(0);
}

void UpdateDlg::InternetUpdate(bool forceDownload)
{
    UpdateStatus(_("Please wait..."));
    m_HasUpdated = false;
    m_Net.SetServer(GetCurrentServer());

    EnableButtons(false);
    forceDownload = forceDownload || !XRCCTRL(*this, "chkCache", wxCheckBox)->GetValue();

    bool forceDownloadMirrors = forceDownload || !wxFileExists(GetMirrorsFilename());
    if (forceDownloadMirrors)
    {
        if (!m_Net.DownloadFile(_T("mirrors.cfg"), GetMirrorsFilename()))
        {
            UpdateStatus(_("Error downloading list of mirrors"), 0, 0);
            return;
        }
        else
        {
            FillServers();
            m_Net.SetServer(GetCurrentServer()); // update server based on mirrors
        }
    }

    wxString config = GetConfFilename();
    forceDownload = forceDownload || !wxFileExists(config);
    if (forceDownload && !m_Net.DownloadFile(_T("webupdate.conf"), config))
    {
        UpdateStatus(_("Error downloading list of updates"), 0, 0);
        return;
    }
    else
    {
        IniParser ini;
        if (!ini.ParseFile(config))
        {
            UpdateStatus(_("Failed to retrieve the list of updates"), 0, 0);
            return;
        }
        ini.Sort();

        if (m_Recs)
            delete[] m_Recs;

        // remember to delete[] m_Recs when we 're done with it!!!
        // it's our responsibility once given to us
        m_Recs = ReadConf(ini, &m_RecsCount, GetCurrentServer(), GetPackagePath());

        FillGroups();
    }
    EnableButtons();
    UpdateStatus(_("Ready"), 0, 0);

    m_HasUpdated = true;
}

void UpdateDlg::FillServers()
{
    wxComboBox* cmb = XRCCTRL(*this, "cmbServer", wxComboBox);
    cmb->Clear();
    m_Servers.Clear();

    IniParser ini;
    ini.ParseFile(GetMirrorsFilename());
    int group = ini.FindGroupByName(_T("WebUpdate mirrors"));
    for (int i = 0; group != -1 && i < ini.GetKeysCount(group); ++i)
    {
        cmb->Append(ini.GetKeyName(group, i));
        m_Servers.Add(ini.GetKeyValue(group, i));
    }
    if (cmb->GetCount() == 0)
    {
        cmb->Append(_("devpaks.org Community Devpaks"));
        m_Servers.Add(_T("http://devpaks.sourceforge.net/"));
    }
    cmb->SetSelection(0);
}

wxString UpdateDlg::GetConfFilename()
{
    int server_hash = GetTextCRC32(GetCurrentServer().mb_str());
    wxString config;
    config = ConfigManager::GetConfigFolder() + wxFILE_SEP_PATH;
    config.Printf(_T("%sdevpak_%x.conf"), config.c_str(), server_hash);
    return config;
}

wxString UpdateDlg::GetMirrorsFilename() const
{
    wxString config;
    config = ConfigManager::GetConfigFolder() + wxFILE_SEP_PATH + _T("devpak_mirrors.cfg");
    return config;
}

wxString UpdateDlg::GetCurrentServer() const
{
    return m_Servers[XRCCTRL(*this, "cmbServer", wxComboBox)->GetSelection()];
}

wxString UpdateDlg::GetBasePath() const
{
    return g_MasterPath + wxFILE_SEP_PATH;
}

wxString UpdateDlg::GetPackagePath() const
{
    return GetBasePath() + _T("Packages") + wxFILE_SEP_PATH;
}

bool UpdateDlg::FilterRec(UpdateRec* rec)
{
    if (!rec)
        return false;
    wxComboBox* cmb = XRCCTRL(*this, "cmbFilter", wxComboBox);
    switch (cmb->GetSelection())
    {
        case 0: // All
            return true;

        case 1: // Installed
            return rec->installed;

        case 2: // installed with update available
            return rec->installed && rec->version != rec->installed_version;

        case 3: // downloaded but not installed
            return rec->downloaded && !rec->installed;

        case 4: // not installed
            return !rec->downloaded && !rec->installed;

        default:
            return false;
    }
    return false; // doesn't reach here
}

void UpdateDlg::ApplyFilter()
{
    wxTreeCtrl* tree = XRCCTRL(*this, "tvCategories", wxTreeCtrl);

    FillGroups();
    FillFiles(tree->GetSelection());
    EnableButtons();
}

UpdateRec* UpdateDlg::GetRecFromListView()
{
    wxListCtrl* lst = XRCCTRL(*this, "lvFiles", wxListCtrl);
    int index = lst->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (index == -1)
        return 0;
    wxString title = lst->GetItemText(index);
    wxString version = GetListColumnText(index, 1);
    wxString revision = GetListColumnText(index, 4);
    return FindRec(title, version, revision, m_Recs, m_RecsCount);
}

void UpdateDlg::DownloadFile(bool dontInstall)
{
    UpdateStatus(_("Please wait..."));
    UpdateRec* rec = GetRecFromListView();
    if (!rec)
    {
        wxMessageBox(_("No file selected!"), _("Error"), wxICON_ERROR);
        UpdateStatus(_("Ready"), 0, 0);
        return;
    }

    if (rec->version == rec->installed_version)
    {
        if (wxMessageBox(_("You seem to have installed the latest version.\nAre you sure you want to proceed?"), _("Confirmation"), wxICON_QUESTION | wxYES_NO) == wxNO)
            return;
    }

    if (!CreateDirRecursively(GetPackagePath()))
    {
        wxMessageBox(_("Can't create directory ") + GetPackagePath(), _("Error"), wxICON_ERROR);
        return;
    }

    if (wxFileExists(GetPackagePath() + rec->local_file))
    {
        if (wxMessageBox(_("This file already exists!\nAre you sure you want to download it again?"), _("Confirmation"), wxICON_QUESTION | wxYES_NO) == wxNO &&
            rec->installable)
        {
            if (!dontInstall && wxMessageBox(_("Do you want to force-install it?"), _("Confirmation"), wxICON_QUESTION | wxYES_NO) == wxYES)
                InstallFile();
            return;
        }
    }

    m_Net.SetServer(rec->remote_server);

    EnableButtons(false);
    if (!m_Net.DownloadFile(rec->remote_file, GetPackagePath() + rec->local_file))
    {
        rec->downloaded = false;
        UpdateStatus(_("Error downloading file: ") + rec->remote_server + _T(" > ") + rec->remote_file, 0, 0);
        return;
    }
    else
        rec->downloaded = true;
    UpdateStatus(_("Ready"), 0, 0);
    EnableButtons();
}

void UpdateDlg::InstallFile()
{
    UpdateStatus(_("Please wait..."));
    UpdateRec* rec = GetRecFromListView();
    if (!rec)
    {
        wxMessageBox(_("No file selected!"), _("Error"), wxICON_ERROR);
        UpdateStatus(_("Ready"), 0, 0);
        return;
    }
    wxYield();

    if (rec->title == _T("WebUpdate Mirrors list"))
    {
        InstallMirrors(GetPackagePath() + rec->local_file);
        rec->installed = true;
        ApplyFilter();
        UpdateStatus(_("Ready"), 0, 0);
        return;
    }
    else if (!rec->installable)
    {
        UpdateStatus(_("Ready"), 0, 0);
        return;
    }

    if (!CreateDirRecursively(GetPackagePath()))
    {
        UpdateStatus(_("Ready"), 0, 0);
        wxMessageBox(_("Can't create directory ") + GetPackagePath(), _("Error"), wxICON_ERROR);
        return;
    }

    wxArrayString files;
    DevPakInstaller inst;
    if (inst.Install(rec->name, GetPackagePath() + rec->local_file, GetBasePath(), &files))
    {
//        wxFileName fname(GetPackagePath() + rec->local_file);
//        fname.SetExt("entry");
//        fname.SetName(rec->title);
//        CreateEntryFile(rec, fname.GetFullPath(), files);
        CreateEntryFile(rec, GetPackagePath() + rec->entry, files);
        wxMessageBox(_("DevPak installed"), _("Message"), wxICON_INFORMATION);

        // refresh installed_version
        rec->installed = true;
        rec->installed_version = rec->version;
        SetListColumnText(-1, 2, rec->installed_version);
    }
    else
    {
        wxMessageBox(_("DevPak was not installed.\nStatus:\n") + inst.GetStatus(), _("Error"), wxICON_ERROR);
    }
    UpdateStatus(_("Ready"), 0, 0);
}

void UpdateDlg::InstallMirrors(const wxString& file)
{
    if (!wxCopyFile(file, GetMirrorsFilename(), true))
        wxMessageBox(_("Can't install mirrors file: ") + file, _("Error"), wxICON_ERROR);
    else
    {
        wxRemoveFile(file);
        FillServers();
        m_Net.SetServer(GetCurrentServer()); // update server based on mirrors
        wxMessageBox(_("Mirrors installed"), _("Information"), wxICON_INFORMATION);
    }
}

void UpdateDlg::UninstallFile()
{
    UpdateStatus(_("Please wait..."));
    UpdateRec* rec = GetRecFromListView();
    if (!rec)
    {
        wxMessageBox(_("No file selected!"), _("Error"), wxICON_ERROR);
        UpdateStatus(_("Ready"), 0, 0);
        return;
    }
    wxYield();

    DevPakInstaller inst;
    if (inst.Uninstall(GetPackagePath() + rec->entry))
    {
        wxMessageBox(_("DevPak uninstalled"), _("Message"), wxICON_INFORMATION);

        // refresh installed_version
        rec->installed_version.Clear();
        rec->installed = false;
        SetListColumnText(-1, 2, rec->installed_version);
    }
    else
    {
        wxMessageBox(_("DevPak was not uninstalled.\nStatus:\n") + inst.GetStatus(), _("Error"), wxICON_ERROR);
    }
}

void UpdateDlg::CreateEntryFile(UpdateRec* rec, const wxString& filename, const wxArrayString& files)
{
    wxString entry;
    entry << _T("[Setup]\n");
    entry << _T("AppName=") << rec->name << _T("\n");
    entry << _T("AppVersion=") << rec->version << _T("\n");

    entry << _T("\n");
    entry << _T("[Files]\n");
    for (unsigned int i = 0; i < files.GetCount(); ++i)
    {
        entry << files[i] << _T("\n");
    }

    wxFile f(filename, wxFile::write);
    if (f.IsOpened())
    {
        f.Write(entry.mb_str(wxConvUTF8),entry.Length());
    }
}

void UpdateDlg::OnFileRightClick(wxListEvent& event)
{
//    LOGSTREAM << "pt.x=" << event.GetPoint().x << ", pt.y=" << event.GetPoint().y << '\n';
    UpdateRec* rec = GetRecFromListView();
    if (!rec)
        return;

    wxMenu popup;
    popup.Append(idPopupDownloadAndInstall, _("Download && install"));
    popup.AppendSeparator();
    popup.Append(idPopupDownload, _("Download"));
    popup.Append(idPopupInstall, _("Install"));
    popup.AppendSeparator();
    popup.Append(idPopupUninstall, _("Uninstall"));

    bool canDl = !rec->downloaded || rec->version != rec->installed_version;
    bool canInst = rec->downloaded && (!rec->installed || rec->version != rec->installed_version);

    popup.Enable(idPopupDownload, canDl);
    popup.Enable(idPopupInstall, canInst);
    popup.Enable(idPopupDownloadAndInstall, canInst || canDl);
    popup.Enable(idPopupUninstall, rec->installed);

    wxListCtrl* lst = XRCCTRL(*this, "lvFiles", wxListCtrl);
    lst->PopupMenu(&popup, event.GetPoint());
}

void UpdateDlg::OnFileDeSelected(wxListEvent& event)
{
    wxListItem id;
    FillFileDetails(id);
    EnableButtons();
}

void UpdateDlg::OnFileSelected(wxListEvent& event)
{
    FillFileDetails(event.GetItem());
    EnableButtons();
}

void UpdateDlg::OnTreeSelChanged(wxTreeEvent& event)
{
    FillFiles(event.GetItem());
    EnableButtons();
}

void UpdateDlg::OnDownload(wxCommandEvent& event)
{
    DownloadFile(true);
}

void UpdateDlg::OnInstall(wxCommandEvent& event)
{
    InstallFile();
}

void UpdateDlg::OnUninstall(wxCommandEvent& event)
{
    UninstallFile();
}

void UpdateDlg::OnDownloadAndInstall(wxCommandEvent& event)
{
    DownloadFile();
}

void UpdateDlg::OnServerChange(wxCommandEvent& event)
{
    InternetUpdate();
}

void UpdateDlg::OnFilterChange(wxCommandEvent& event)
{
    ApplyFilter();
}

void UpdateDlg::OnConnect(wxCommandEvent& event)
{
    XRCCTRL(*this, "wxID_CANCEL", wxButton)->SetLabel(_("Abort"));
    EnableButtons();
}

void UpdateDlg::OnDisConnect(wxCommandEvent& event)
{
    XRCCTRL(*this, "wxID_CANCEL", wxButton)->SetLabel(_("Close"));
    EnableButtons();
}

void UpdateDlg::OnProgress(wxCommandEvent& event)
{
    int prg = -1;
    if (m_CurrFileSize != 0)
        prg = event.GetInt() * 100 / m_CurrFileSize;
    UpdateStatus(_("Downloading: ") + event.GetString(), prg);

    wxStaticText* lbl = XRCCTRL(*this, "lblProgress", wxStaticText);

    wxString msg;
    msg.Printf(_("%s of %s"), GetSizeString(event.GetInt()).c_str(), GetSizeString(m_CurrFileSize).c_str());
    lbl->SetLabel(msg);
}

void UpdateDlg::OnAborted(wxCommandEvent& event)
{
    UpdateStatus(_("Download aborted: ") + event.GetString(), 0, 0);
    XRCCTRL(*this, "lblProgress", wxStaticText)->SetLabel(_T(""));
    m_LastBlockSize = 0;
}

void UpdateDlg::OnDownloadStarted(wxCommandEvent& event)
{
    m_CurrFileSize = event.GetInt();
    UpdateStatus(_("Download started: ") + event.GetString(), 0, 100);
    XRCCTRL(*this, "lblProgress", wxStaticText)->SetLabel(_T(""));
    m_LastBlockSize = 0;
}

void UpdateDlg::OnDownloadEnded(wxCommandEvent& event)
{
    UpdateStatus(_("Download finished: ") + event.GetString());
    XRCCTRL(*this, "lblProgress", wxStaticText)->SetLabel(_T(""));
    m_LastBlockSize = 0;

    if (m_HasUpdated && event.GetInt() == 0)
    {
        UpdateRec* rec = GetRecFromListView();
        if (rec)
        {
            if (rec->bytes != m_CurrFileSize)
                wxMessageBox(_("File size mismatch for ") + event.GetString() + _("!\n\n"
                            "This, usually, means one of three things:\n"
                            "1) The reported size in the update list is wrong. The DevPak might still be valid.\n"
                            "2) The file's location returned a web error-page. Invalid DevPak...\n"
                            "3) The file is corrupt...\n\n"
                            "You can try to install it anyway. If it is not a valid DevPak, the operation will fail."),
                            _("Warning"), wxICON_WARNING);
        }
        if (rec && rec->installable && wxMessageBox(_("Do you want to install ") + event.GetString() + _(" now?"), _("Confirmation"), wxICON_QUESTION | wxYES_NO) == wxYES)
            InstallFile();
        else if (rec && rec->title == _T("WebUpdate Mirrors list"))
            InstallMirrors(GetPackagePath() + rec->local_file);
    }
    m_CurrFileSize = 0;
}

void UpdateDlg::OnUpdateUI(wxUpdateUIEvent& event)
{
    // hack to display the download message *after* the dialog has been shown...
    if (m_FirstTimeCheck)
    {
        m_FirstTimeCheck = false; // no more, just once
        wxString config = GetConfFilename();
        if (wxFileExists(config))
            InternetUpdate();
        else
        {
            if (wxMessageBox(_("A list of updates needs to be downloaded.\nDo you want to do this now?"), _("Confirmation"), wxICON_QUESTION | wxYES_NO) == wxYES)
                InternetUpdate(true);
        }
    }
}
