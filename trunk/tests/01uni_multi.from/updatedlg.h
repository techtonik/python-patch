#ifndef UPDATEDLG_H
#define UPDATEDLG_H

#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include "cbnetwork.h"
#include "conf.h"

class UpdateDlg : public wxDialog
{
	public:
		UpdateDlg(wxWindow* parent);
		virtual ~UpdateDlg();

		void EndModal(int retCode);
	protected:
        void OnFileSelected(wxListEvent& event);
        void OnFileDeSelected(wxListEvent& event);
        void OnFileRightClick(wxListEvent& event);
        void OnTreeSelChanged(wxTreeEvent& event);
        void OnDownload(wxCommandEvent& event);
        void OnInstall(wxCommandEvent& event);
        void OnUninstall(wxCommandEvent& event);
        void OnDownloadAndInstall(wxCommandEvent& event);
        void OnUpdate(wxCommandEvent& event);
        void OnServerChange(wxCommandEvent& event);
        void OnFilterChange(wxCommandEvent& event);
        void OnConnect(wxCommandEvent& event);
        void OnDisConnect(wxCommandEvent& event);
        void OnProgress(wxCommandEvent& event);
        void OnAborted(wxCommandEvent& event);
        void OnDownloadStarted(wxCommandEvent& event);
        void OnDownloadEnded(wxCommandEvent& event);
        void OnUpdateUI(wxUpdateUIEvent& event);
	private:
        void InternetUpdate(bool forceDownload = false);
        void DownloadFile(bool dontInstall = false);
        void InstallFile();
        void UninstallFile();
        void InstallMirrors(const wxString& file);
        void CreateEntryFile(UpdateRec* rec, const wxString& filename, const wxArrayString& files);
        void EnableButtons(bool update = true, bool abort = true);
        void FillServers();
        void FillGroups();
        void FillFiles(const wxTreeItemId& id);
        void FillFileDetails(const wxListItem& id);
        void UpdateStatus(const wxString& status, int curProgress = -1, int maxProgress = -1);
        UpdateRec* GetRecFromListView();
        void CreateListColumns();
        void AddRecordToList(UpdateRec* rec);
        void SetListColumnText(int idx, int col, const wxString& text);

        wxString GetConfFilename();
        wxString GetMirrorsFilename() const;
        wxString GetCurrentServer() const;
        wxString GetBasePath() const;
        wxString GetPackagePath() const;
        bool FilterRec(UpdateRec* rec);
        void ApplyFilter();

        UpdateRec* m_Recs;
        wxArrayString m_Servers;
        int m_RecsCount;
        int m_CurrFileSize;
        int m_LastBlockSize; // for bps
        bool m_HasUpdated;
        bool m_FirstTimeCheck;
        cbNetwork m_Net;
        DECLARE_EVENT_TABLE();
};

#endif // UPDATEDLG_H
