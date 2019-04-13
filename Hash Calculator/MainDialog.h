
// MainDialog.h: 头文件
//

#pragma once
#include "HashListener.hpp"
#include "IOCP.hpp"

// MainDialog 对话框
class MainDialog : public CDialog, public HashListener
{
	// 构造
public:
	MainDialog(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HASHCALCULATOR_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	IOCP m_iocp;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_progress;
	BOOL m_type;
	CEdit m_edit;
	static VOID NTAPI ThreadProc(PTP_CALLBACK_INSTANCE Instance, PVOID Context);
	afx_msg void OnBnClickedButton1();
	void AppendLine();
	void AppendLine(CString line);
	void OnBegin(LPCTSTR path, File& f) override;
	void OnProgress(int rating) override;
	void OnCompleted(PBYTE pResult, DWORD len, DWORD time) override;
	void OnError(LPCTSTR path, LPCTSTR error) override;
	afx_msg void OnDropFiles(HDROP hDropInfo);
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
};
