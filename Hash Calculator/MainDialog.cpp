
// MainDialog.cpp: 实现文件
//

#include "stdafx.h"
#include "Hash Calculator.h"
#include "MainDialog.h"
#include "afxdialogex.h"
#include "resource.h"
#include "Hash.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_UPDATE_VALUE WM_USER + 1

// MainDialog 对话框


MainDialog::MainDialog(CWnd * pParent /*=nullptr*/)
	: CDialog(IDD_HASHCALCULATOR_DIALOG, pParent)
	, m_type(FALSE)
	, m_edit()
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void MainDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Radio(pDX, IDC_RADIO1, m_type);
	DDX_Control(pDX, IDC_EDIT1, m_edit);
}

BEGIN_MESSAGE_MAP(MainDialog, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &MainDialog::OnBnClickedButton1)
	ON_WM_DROPFILES()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


// MainDialog 消息处理程序

BOOL MainDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_progress.SetRange(0, 100);
	m_progress.SetPos(0);
	m_iocp.Create();
	TrySubmitThreadpoolCallback(ThreadProc, this, NULL);
	if (IsUserAdmin())
	{
		ChangeMessageFilterForDrop(m_hWnd);
	}
	theApp.m_event.Set();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void MainDialog::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR MainDialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void MainDialog::OnBnClickedButton1()
{
	CFileDialog dialog(true,
		NULL, NULL, OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_FILEMUSTEXIST,
		L"所有文件 (*.*)|*.*|",
		this);
	if (dialog.DoModal() == IDOK)
	{
		auto pos = dialog.GetStartPosition();
		while (pos)
		{
			CString str = dialog.GetNextPathName(pos);
			LPCTSTR path = new TCHAR[str.GetLength() + 1];
			MoveMemory((void*)path, str.GetString(), (str.GetLength() + 1) * sizeof(TCHAR));
			m_iocp.Post(0, 0, (LPOVERLAPPED)path);
		}
	}
	m_edit.SetFocus();
}

void HashFile(LPTSTR path, LPCTSTR pszAlgId, HashListener* pListener)
{
	File f(path, GENERIC_READ, FALSE, FILE_FLAG_OVERLAPPED);
	if (!f.GetHandle())
	{
		pListener->OnError(path, L"文件打开失败");
		return;
	}
	LONGLONG length = f.Length();
	if (!length)
	{
		pListener->OnError(path, L"文件大小为 0");
		return;
	}
	DWORD time = GetTickCount();
	pListener->OnBegin(path, f);
	try
	{
		using namespace std;
		Hash hash(pszAlgId);
		constexpr DWORD len = 1048576;
		unique_ptr<BYTE> pFirst(new BYTE[len]);
		unique_ptr<BYTE> pSecond(new BYTE[len]);
		OVERLAPPED oFirst{};
		OVERLAPPED oSecond{};
		LONGLONG index = 0;
		BOOL bRead = f.ReadAsync(pFirst.get(), len, &oFirst);
		do
		{
			if (!bRead) f.WaitFor();
			index += oFirst.InternalHigh;
			oSecond.Pointer = (PVOID)index;
			bRead = f.ReadAsync(pSecond.get(), len, &oSecond);
			hash.Update(pFirst.get(), (DWORD)oFirst.InternalHigh);
			pListener->OnProgress((int)(index * 100 / length));
			if (index == length) break;

			if (!bRead) f.WaitFor();
			index += oSecond.InternalHigh;
			oFirst.Pointer = (PVOID)index;
			bRead = f.ReadAsync(pFirst.get(), len, &oFirst);
			hash.Update(pSecond.get(), (DWORD)oSecond.InternalHigh);
			pListener->OnProgress((int)(index * 100 / length));
		} while (index < length);
		auto value = hash.GetValue();
		pListener->OnCompleted(value.get(), hash.GetLength(), GetTickCount() - time);
	}
	catch (const HashException& e)
	{
		pListener->OnError(path, e.Message);
	}

}

VOID NTAPI  MainDialog::ThreadProc(PTP_CALLBACK_INSTANCE Instance, PVOID Context)
{
	MainDialog* pThis = (MainDialog*)Context;
	DWORD ddd = 0;
	ULONG_PTR lll = NULL;
	LPTSTR path = NULL;
	LPCTSTR alg = NULL;
	while (pThis->m_iocp.Get(&ddd, &lll, (LPOVERLAPPED*)& path))
	{
		pThis->SendMessage(WM_UPDATE_VALUE);
		switch (pThis->m_type)
		{
		case 0:
			alg = BCRYPT_MD5_ALGORITHM;;
			break;
		case 1:
			alg = BCRYPT_SHA1_ALGORITHM;;;
			break;
		case 2:
			alg = BCRYPT_SHA256_ALGORITHM;;;
			break;
		case 3:
			alg = BCRYPT_SHA512_ALGORITHM;;;
			break;
		}
		HashFile(path, alg, pThis);
		delete[] path;
	}
}

void MainDialog::AppendLine()
{
	auto len = m_edit.GetWindowTextLengthW();
	if (!len) return;
	m_edit.SetSel(len, len);
	m_edit.ReplaceSel(L"\r\n");
}

void MainDialog::AppendLine(CString line)
{
	auto len = m_edit.GetWindowTextLengthW();
	m_edit.SetSel(len, len);
	m_edit.ReplaceSel(line + L"\r\n");
}

void MainDialog::OnBegin(LPCTSTR path, File& f)
{
	AppendLine();
	CString str;
	str.Format(L"文件：%s", path);
	AppendLine(str);
	auto len = f.Length();
	LPTSTR sizeBuffer = new TCHAR[100];
	StrFormatByteSize64(len, sizeBuffer, 100);
	if (len >= 1024)
	{
		str.Format(L"大小：%s (%lld 字节)", sizeBuffer, len);
	}
	else
	{
		str.Format(L"大小：%s", sizeBuffer);
	}
	AppendLine(str);
	FILETIME fileTime{};
	f.GetFileTime(NULL, NULL, &fileTime);
	FILETIME localTime{};
	FileTimeToLocalFileTime(&fileTime, &localTime);
	SYSTEMTIME sysTime{};
	FileTimeToSystemTime(&localTime, &sysTime);
	LPTSTR dateBuffer = new TCHAR[100];
	GetDateFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, 0, &sysTime, L"yyyy-MM-dd", dateBuffer, 100, NULL);
	LPTSTR timeBuffer = new TCHAR[100];
	GetTimeFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, 0, &sysTime, L"HH:mm:ss", timeBuffer, 100);
	str.Format(L"时间：%s %s", dateBuffer, timeBuffer);
	AppendLine(str);
	delete[] sizeBuffer;
	delete[] dateBuffer;
	delete[] timeBuffer;
}

void MainDialog::OnProgress(int rating)
{
	m_progress.SetPos(rating);
}

void MainDialog::OnCompleted(PBYTE pResult, DWORD len, DWORD time)
{
	LPCTSTR str = NULL;
	switch (m_type)
	{
	case 0:
		str = L"MD5：";
		break;
	case 1:
		str = L"SHA1：";
		break;
	case 2:
		str = L"SHA256：";
		break;
	case 3:
		str = L"SHA512：";
		break;
	}
	CString hex = BinToHex(pResult, len, true);
	hex = str + hex;
	AppendLine(hex);
	CString t;
	t.Format(L"用时：%.2lf s", ((double)time) / 1000);
	AppendLine(t);
}

void MainDialog::OnError(LPCTSTR path, LPCTSTR error)
{
	m_progress.SetPos(0);
	AppendLine();
	CString str;
	str.Format(L"文件：%s", path);
	AppendLine(str);
	AppendLine(error);
}

void MainDialog::OnDropFiles(HDROP hDropInfo)
{
	int count = DragQueryFile(hDropInfo, -1, NULL, 0);
	for (int i = 0; i < count; i++)
	{
		int len = DragQueryFile(hDropInfo, i, NULL, 0) + 1;
		LPTSTR pFile = new TCHAR[len];
		DragQueryFile(hDropInfo, i, pFile, len);
		m_iocp.Post(0, 0, (LPOVERLAPPED)pFile);
	}
	DragFinish(hDropInfo);
	__super::OnDropFiles(hDropInfo);
}


BOOL MainDialog::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	switch (message)
	{
	case WM_UPDATE_VALUE:
		UpdateData(true);
		return true;
	}
	return __super::OnWndMsg(message, wParam, lParam, pResult);
}


void MainDialog::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 567;
	lpMMI->ptMinTrackSize.y = 410;

	__super::OnGetMinMaxInfo(lpMMI);
}
