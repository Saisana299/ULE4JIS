// Ule4Jis.cpp : アプリケーションのクラス動作を定義します。
//

#include "stdafx.h"
#include "Ule4Jis.h"
#include "Ule4JisDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 前方宣言
static bool IsRunAsAdministrator();
static bool RestartAsAdministrator();
static bool IsRunAsAdminEnabled();


// Ule4JisApp

BEGIN_MESSAGE_MAP(Ule4JisApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// Ule4JisApp コンストラクション

Ule4JisApp::Ule4JisApp() : m_hMutex(NULL)
{
	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}


// 唯一の Ule4JisApp オブジェクトです。

Ule4JisApp theApp;


// Ule4JisApp 初期化

BOOL Ule4JisApp::InitInstance()
{
	// アプリケーション マニフェストが visual スタイルを有効にするために、
	// ComCtl32.dll Version 6 以降の使用を指定する場合は、
	// Windows XP に InitCommonControlsEx() が必要です。さもなければ、ウィンドウ作成はすべて失敗します。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// アプリケーションで使用するすべてのコモン コントロール クラスを含めるには、
	// これを設定します。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// 標準初期化
	// これらの機能を使わずに最終的な実行可能ファイルの
	// サイズを縮小したい場合は、以下から不要な初期化
	// ルーチンを削除してください。
	// 設定が格納されているレジストリ キーを変更します。
	// TODO: 会社名または組織名などの適切な文字列に
	// この文字列を変更してください。
	SetRegistryKey(_T("Ule4Jis"));

	// prevent multiple boot
	m_hMutex = ::CreateMutex(NULL, TRUE, m_pszExeName);
	if (::GetLastError() == ERROR_ALREADY_EXISTS) {
		CString msg;
		msg.LoadString(IDS_MSG_MULTIPLE_BOOT_ERROR);
		::MessageBox(NULL, msg, NULL, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	// 管理者として実行が必要な場合、自動的に再起動
	if (IsRunAsAdminEnabled() && !IsRunAsAdministrator()) {
		// /startupフラグがある場合は再起動しない（無限ループを防ぐ）
		if (_tcsstr(m_lpCmdLine, _T("/startup")) == NULL) {
			// Mutexを解放してから再起動
			ReleaseMutex();
			
			if (RestartAsAdministrator()) {
				return FALSE;
			}
		}
	}

	bool startupMode = (_tcsstr(m_lpCmdLine, _T("/startup")) != NULL);

	if (startupMode) {
		// 自動起動時はダイアログを隠してバックグラウンドで動作
		Ule4JisDlg* dlg = new Ule4JisDlg(true);
		m_pMainWnd = dlg;
		dlg->Create(Ule4JisDlg::IDD);
		dlg->ShowWindow(SW_HIDE);
		return TRUE;
	} else {
		// 手動起動時はダイアログを表示
		Ule4JisDlg dlg(false);
		m_pMainWnd = &dlg;
		INT_PTR nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			// TODO: ダイアログが <OK> で消された時のコードを
			//  記述してください。
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: ダイアログが <キャンセル> で消された時のコードを
			//  記述してください。
		}

		// ダイアログは閉じられました。アプリケーションのメッセージ ポンプを開始しないで
		//  アプリケーションを終了するために FALSE を返してください。
		return FALSE;
	}
}

// ヘルパー関数の実装
static bool IsRunAsAdministrator()
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

static bool RestartAsAdministrator()
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

static bool IsRunAsAdminEnabled()
{
	return AfxGetApp()->GetProfileInt(_T("Settings"), _T("RunAsAdmin"), 0) != 0;
}

// Mutexを解放
void Ule4JisApp::ReleaseMutex()
{
	if (m_hMutex != NULL) {
		::CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
}
