// Ule4JisDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Ule4Jis.h"
#include "Ule4JisDlg.h"
#include "KeyEmulator.h"
#include "USonJISStrategy.h"
#include "Constants.h"
#include "afxwin.h"
#include <winreg.h>
#include <taskschd.h>
#include <comdef.h>

#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")

#pragma warning(disable: 4311 4302)  // Suppress cast warnings

// Helper functions
static bool IsStartupEnabled();
static void SetStartup(bool enable);
static bool IsRunAsAdministrator();
static bool RestartAsAdministrator();
static bool SetStartupWithTaskScheduler(bool enable);
static bool IsRunAsAdminEnabled();
static void SetRunAsAdminEnabled(bool enable);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
private:
	CStatic urlLabel;
	HCURSOR handCursor;
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ABOUT_URL, urlLabel);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// Ule4JisDlg ダイアログ




Ule4JisDlg::Ule4JisDlg(bool startupMode, CWnd* pParent /*=NULL*/)
	: CDialog(Ule4JisDlg::IDD, pParent), startupMode(startupMode)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

Ule4JisDlg::~Ule4JisDlg()
{
}

void Ule4JisDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STARTUP, startupCheck);
	DDX_Control(pDX, IDC_RUN_AS_ADMIN, runAsAdminCheck);
}

BEGIN_MESSAGE_MAP(Ule4JisDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_HIDE, &Ule4JisDlg::OnBnClickedHide)
	ON_BN_CLICKED(IDC_STARTUP, &Ule4JisDlg::OnBnClickedStartup)
	ON_BN_CLICKED(IDC_RUN_AS_ADMIN, &Ule4JisDlg::OnBnClickedRunAsAdmin)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// Ule4JisDlg メッセージ ハンドラ

BOOL Ule4JisDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// TODO: 初期化をここに追加します。

	// add icon into task tray
	NOTIFYICONDATA &nid = this->notifyIconData;
	ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = this->m_hWnd;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = MSG_TASKTRAY_CALLBACK;
	nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	CString title;
	GetWindowText(title);
	_tcscpy_s(nid.szTip, sizeof(nid.szTip) / sizeof(TCHAR), title);

	if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
		MessageBox(_T("failed to initialize tasktray icon."), NULL, MB_OK | MB_ICONEXCLAMATION);
		::PostQuitMessage(-1);
		return FALSE;
	}

	// initialize emulator
	USonJISStrategy strategy;
	this->keyEmulator.reset(new KeyEmulator(&strategy));

	// load emulation state from registry
	bool emulationStarted = AfxGetApp()->GetProfileInt(_T("Settings"), _T("EmulationStarted"), 1) != 0;
	if (emulationStarted) {
		this->keyEmulator->start();
		changeTaskTrayIconToUS();
	} else {
		this->keyEmulator->end();
		changeTaskTrayIconToJIS();
	}

	// save current strategy type
	this->currentStrategy = USonJIS;

	// set startup checkbox
	this->startupCheck.SetCheck(IsStartupEnabled());

	// set run as admin checkbox
	this->runAsAdminCheck.SetCheck(IsRunAsAdminEnabled());

	if (startupMode || IsStartupEnabled()) {
		ShowWindow(SW_HIDE);
	}

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

void Ule4JisDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void Ule4JisDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR Ule4JisDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL Ule4JisDlg::DestroyWindow()
{
	// delete icon from tasktray
	::Shell_NotifyIcon(NIM_DELETE, &this->notifyIconData);

	return CDialog::DestroyWindow();
}

LRESULT Ule4JisDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case MSG_TASKTRAY_CALLBACK:
		// dispatch tasktray callback message
		switch (lParam) {
		case WM_RBUTTONUP:
			// show popup menu
			showTaskTrayPopupMenu();
			break;
		case WM_LBUTTONDBLCLK:
			ShowWindow(SW_SHOW);
			ShowWindow(SW_RESTORE);
			break;
		default:
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_TASKTRAY_START:
			this->keyEmulator->start();
			changeTaskTrayIconToUS();
			AfxGetApp()->WriteProfileInt(_T("Settings"), _T("EmulationStarted"), 1);
			break;
		case ID_TASKTRAY_STOP:
			this->keyEmulator->end();
			changeTaskTrayIconToJIS();
			AfxGetApp()->WriteProfileInt(_T("Settings"), _T("EmulationStarted"), 0);
			break;
		case ID_TASKTRAY_RESTART:
			this->keyEmulator->end();
			this->keyEmulator->start();
			break;
		case ID_TASKTRAY_EXIT:
			::PostQuitMessage(0);
			break;

		// https://github.com/kimi-soft/forked-ule4jis/commit/1c52200d2f721c87ef7aa4232681894a4da16f03
		case WM_WINDOWPOSCHANGING:
			onWindowPosChanging((WINDOWPOS*)lParam);
			break;

		default:
			break;
		}
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

// https://github.com/kimi-soft/forked-ule4jis/commit/1c52200d2f721c87ef7aa4232681894a4da16f03
void Ule4JisDlg::onWindowPosChanging(WINDOWPOS* lpwndpos) {
	CDialog::OnWindowPosChanging(lpwndpos);

	// Hidden launch dialog
	if (lpwndpos != NULL) {
		lpwndpos->flags &= ~SWP_SHOWWINDOW;
	}
}

void Ule4JisDlg::changeTaskTrayIconToUS() {
	this->notifyIconData.hIcon = ::AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	::Shell_NotifyIcon(NIM_MODIFY, &this->notifyIconData);
}

void Ule4JisDlg::changeTaskTrayIconToJIS() {
	this->notifyIconData.hIcon = ::AfxGetApp()->LoadIcon(IDR_ICON_JIS);
	::Shell_NotifyIcon(NIM_MODIFY, &this->notifyIconData);
}

void Ule4JisDlg::showTaskTrayPopupMenu() {
	CPoint point;
	GetCursorPos(&point);

	CMenu menu;
	menu.LoadMenu(IDR_MENU_TASKTRAY);

	CMenu *subMenu = menu.GetSubMenu(0);

	// set menu state
	if (this->keyEmulator->isStarted()) {
		subMenu->EnableMenuItem(ID_TASKTRAY_START, MF_GRAYED);
	} else {
		subMenu->EnableMenuItem(ID_TASKTRAY_STOP, MF_GRAYED);
	}

	//if (this->currentStrategy == USonJIS) {
	//	subMenu->GetSubMenu(0)->EnableMenuItem(ID_STRATEGY_USONJIS, MF_GRAYED);
	//} else {
	//	subMenu->GetSubMenu(0)->EnableMenuItem(ID_STRATEGY_JISONUS, MF_GRAYED);
	//}

	SetForegroundWindow();
	subMenu->TrackPopupMenu(TPM_BOTTOMALIGN | TPM_RIGHTALIGN, point.x, point.y, this);
	PostMessage(WM_NULL);
}

void Ule4JisDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: ここにメッセージ ハンドラ コードを追加します。
	if (nType == SIZE_MINIMIZED) {
		ShowWindow(SW_HIDE);
	}
}

void Ule4JisDlg::OnBnClickedHide()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	ShowWindow(SW_MINIMIZE);
}

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  ここで DC の属性を変更してください。

	// TODO:  既定値を使用したくない場合は別のブラシを返します。

	// set color 'blue' to draw URL text.
	if (pWnd == &this->urlLabel) {
		pDC->SetTextColor(RGB(0, 0, 0xFF));
	}

	return hbr;
}

BOOL CAboutDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam) == IDC_ABOUT_URL) {
		if (HIWORD(wParam) == STN_CLICKED) {
			HINSTANCE result = ::ShellExecute(NULL, _T("open"), DEZZ_NETWORKS_URL, NULL, NULL, SW_SHOWNORMAL);
			if (reinterpret_cast<INT_PTR>(result) <= 32) {
				// error. but since this is not critical problem, i ignore this :P
			}
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}

BOOL CAboutDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// set hand cursor if a pointer is over url-label
	if (pWnd == &this->urlLabel) {
		SetCursor(this->handCursor);
		return TRUE;
	}

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ここに初期化を追加してください

	// get hand cursor handle
	this->handCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND));

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void Ule4JisDlg::OnBnClickedStartup()
{
	bool enable = this->startupCheck.GetCheck() == BST_CHECKED;
	
	if (IsRunAsAdminEnabled()) {
		// 管理者として実行モードの場合はタスクスケジューラを使用
		if (!SetStartupWithTaskScheduler(enable)) {
			MessageBox(_T("タスクスケジューラの設定に失敗しました。"), _T("エラー"), MB_OK | MB_ICONERROR);
			this->startupCheck.SetCheck(!enable);
		}
	} else {
		// 通常モードではレジストリを使用
		SetStartup(enable);
	}
}

void Ule4JisDlg::OnBnClickedRunAsAdmin()
{
	bool enable = this->runAsAdminCheck.GetCheck() == BST_CHECKED;
	SetRunAsAdminEnabled(enable);

	// スタートアップが有効な場合は設定を更新
	if (this->startupCheck.GetCheck() == BST_CHECKED) {
		if (enable) {
			// レジストリからタスクスケジューラに切り替え
			SetStartup(false);
			if (!SetStartupWithTaskScheduler(true)) {
				MessageBox(_T("タスクスケジューラの設定に失敗しました。"), _T("エラー"), MB_OK | MB_ICONERROR);
				this->runAsAdminCheck.SetCheck(false);
				SetRunAsAdminEnabled(false);
				SetStartup(true);
			}
		} else {
			// タスクスケジューラからレジストリに切り替え
			SetStartupWithTaskScheduler(false);
			SetStartup(true);
		}
	}

	// 管理者権限が有効になった場合、再起動を提案
	if (enable && !IsRunAsAdministrator()) {
		int result = MessageBox(
			_T("管理者権限で実行するには、アプリケーションを再起動する必要があります。\n今すぐ再起動しますか？"),
			_T("確認"),
			MB_YESNO | MB_ICONQUESTION
		);

		if (result == IDYES) {
			// Mutexを解放してから再起動
			((Ule4JisApp*)AfxGetApp())->ReleaseMutex();
			
			if (RestartAsAdministrator()) {
				PostQuitMessage(0);
			} else {
				MessageBox(_T("管理者権限での再起動に失敗しました。"), _T("エラー"), MB_OK | MB_ICONERROR);
			}
		}
	}
}

// Helper functions
bool IsStartupEnabled()
{
	// まずレジストリをチェック
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
		DWORD type, size;
		if (RegQueryValueEx(hKey, _T("Ule4Jis"), NULL, &type, NULL, &size) == ERROR_SUCCESS) {
			RegCloseKey(hKey);
			return true;
		}
		RegCloseKey(hKey);
	}

	// タスクスケジューラもチェック
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
		return false;
	}

	bool taskExists = false;
	ITaskService* pService = NULL;
	hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER,
		IID_ITaskService, (void**)&pService);

	if (SUCCEEDED(hr)) {
		hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
		if (SUCCEEDED(hr)) {
			ITaskFolder* pRootFolder = NULL;
			hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);

			if (SUCCEEDED(hr)) {
				IRegisteredTask* pRegisteredTask = NULL;
				hr = pRootFolder->GetTask(_bstr_t(L"Ule4Jis"), &pRegisteredTask);
				if (SUCCEEDED(hr)) {
					taskExists = true;
					pRegisteredTask->Release();
				}
				pRootFolder->Release();
			}
		}
		pService->Release();
	}

	if (hr != RPC_E_CHANGED_MODE) {
		CoUninitialize();
	}

	return taskExists;
}

void SetStartup(bool enable)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
		if (enable) {
			TCHAR path[MAX_PATH];
			GetModuleFileName(NULL, path, MAX_PATH);
			_tcscat_s(path, _T(" /startup"));
			RegSetValueEx(hKey, _T("Ule4Jis"), 0, REG_SZ, (BYTE*)path, static_cast<DWORD>((_tcslen(path) + 1) * sizeof(TCHAR)));
		} else {
			RegDeleteValue(hKey, _T("Ule4Jis"));
		}
		RegCloseKey(hKey);
	}
}

// 管理者権限で実行中かチェック
bool IsRunAsAdministrator()
{
	BOOL isAdmin = FALSE;
	PSID adminGroup = NULL;
	SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

	if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup))
	{
		CheckTokenMembership(NULL, adminGroup, &isAdmin);
		FreeSid(adminGroup);
	}

	return isAdmin != FALSE;
}

// 管理者権限で再起動
bool RestartAsAdministrator()
{
	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);

	// コマンドライン引数を取得
	LPCTSTR cmdLine = GetCommandLine();
	LPCTSTR args = _tcschr(cmdLine, _T(' '));
	if (args == NULL) {
		args = _T("");
	}

	SHELLEXECUTEINFO sei = { sizeof(sei) };
	sei.lpVerb = _T("runas");
	sei.lpFile = path;
	sei.lpParameters = args;
	sei.nShow = SW_NORMAL;

	if (ShellExecuteEx(&sei)) {
		return true;
	}
	return false;
}

// タスクスケジューラを使用してスタートアップ登録
bool SetStartupWithTaskScheduler(bool enable)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
		return false;
	}

	bool result = false;
	ITaskService* pService = NULL;
	hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER,
		IID_ITaskService, (void**)&pService);

	if (SUCCEEDED(hr)) {
		hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
		if (SUCCEEDED(hr)) {
			ITaskFolder* pRootFolder = NULL;
			hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);

			if (SUCCEEDED(hr)) {
				if (enable) {
					// タスクを作成
					ITaskDefinition* pTask = NULL;
					hr = pService->NewTask(0, &pTask);

					if (SUCCEEDED(hr)) {
						// トリガー設定（ログオン時）
						ITriggerCollection* pTriggerCollection = NULL;
						hr = pTask->get_Triggers(&pTriggerCollection);

						if (SUCCEEDED(hr)) {
							ITrigger* pTrigger = NULL;
							hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
							if (SUCCEEDED(hr)) {
								ILogonTrigger* pLogonTrigger = NULL;
								hr = pTrigger->QueryInterface(IID_ILogonTrigger, (void**)&pLogonTrigger);
								if (SUCCEEDED(hr)) {
									pLogonTrigger->put_Id(_bstr_t(L"LogonTriggerId"));
									pLogonTrigger->Release();
								}
								pTrigger->Release();
							}
							pTriggerCollection->Release();
						}

						// アクション設定
						IActionCollection* pActionCollection = NULL;
						hr = pTask->get_Actions(&pActionCollection);

						if (SUCCEEDED(hr)) {
							IAction* pAction = NULL;
							hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
							if (SUCCEEDED(hr)) {
								IExecAction* pExecAction = NULL;
								hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
								if (SUCCEEDED(hr)) {
									TCHAR path[MAX_PATH];
									GetModuleFileName(NULL, path, MAX_PATH);
									pExecAction->put_Path(_bstr_t(path));
									pExecAction->put_Arguments(_bstr_t(L"/startup"));
									pExecAction->Release();
								}
								pAction->Release();
							}
							pActionCollection->Release();
						}

						// プリンシパル設定（最高の特権で実行）
						IPrincipal* pPrincipal = NULL;
						hr = pTask->get_Principal(&pPrincipal);
						if (SUCCEEDED(hr)) {
							pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
							pPrincipal->Release();
						}

						// 設定
						ITaskSettings* pSettings = NULL;
						hr = pTask->get_Settings(&pSettings);
						if (SUCCEEDED(hr)) {
							pSettings->put_StartWhenAvailable(VARIANT_TRUE);
							pSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
							pSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE);
							pSettings->Release();
						}

						// タスクを登録
						IRegisteredTask* pRegisteredTask = NULL;
						hr = pRootFolder->RegisterTaskDefinition(
							_bstr_t(L"Ule4Jis"),
							pTask,
							TASK_CREATE_OR_UPDATE,
							_variant_t(),
							_variant_t(),
							TASK_LOGON_INTERACTIVE_TOKEN,
							_variant_t(L""),
							&pRegisteredTask);

						if (SUCCEEDED(hr)) {
							result = true;
							if (pRegisteredTask) pRegisteredTask->Release();
						}

						pTask->Release();
					}
				}
				else {
					// タスクを削除
					hr = pRootFolder->DeleteTask(_bstr_t(L"Ule4Jis"), 0);
					result = SUCCEEDED(hr);
				}

				pRootFolder->Release();
			}
		}
		pService->Release();
	}

	if (hr != RPC_E_CHANGED_MODE) {
		CoUninitialize();
	}
	return result;
}

// 管理者として実行オプションが有効か確認
bool IsRunAsAdminEnabled()
{
	return AfxGetApp()->GetProfileInt(_T("Settings"), _T("RunAsAdmin"), 0) != 0;
}

// 管理者として実行オプションを設定
void SetRunAsAdminEnabled(bool enable)
{
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("RunAsAdmin"), enable ? 1 : 0);
}

void Ule4JisDlg::OnDestroy()
{
	// タスクトレイアイコンを削除
	Shell_NotifyIcon(NIM_DELETE, &notifyIconData);

	// キーボードフックを停止
	if (keyEmulator) {
		keyEmulator->end();
	}

	// アプリケーションを終了
	PostQuitMessage(0);

	CDialog::OnDestroy();
}
