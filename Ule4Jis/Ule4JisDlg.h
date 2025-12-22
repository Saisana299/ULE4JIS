// Ule4JisDlg.h : ヘッダー ファイル
//

#pragma once

#include "KeyEmulator.h"
#include <memory>

// Ule4JisDlg ダイアログ
class Ule4JisDlg : public CDialog
{
// コンストラクション
public:
	Ule4JisDlg(bool startupMode = false, CWnd* pParent = NULL);	// 標準コンストラクタ
	virtual ~Ule4JisDlg();

// ダイアログ データ
	enum { IDD = IDD_ULE4JP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート

private:
	// added
	bool startupMode;
	enum Strategy { USonJIS, JISonUS };
	NOTIFYICONDATA notifyIconData;
	Strategy currentStrategy;

	void showTaskTrayPopupMenu();
	void changeTaskTrayIconToUS();
	void changeTaskTrayIconToJIS();

	// https://github.com/kimi-soft/forked-ule4jis/commit/1c52200d2f721c87ef7aa4232681894a4da16f03
	void onWindowPosChanging(WINDOWPOS* lpwndpos);

// 実装
protected:
	HICON m_hIcon;
	std::unique_ptr<KeyEmulator> keyEmulator;
	CButton startupCheck;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStartup();
	afx_msg void OnDestroy();
	virtual BOOL DestroyWindow();
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedHide();
};
