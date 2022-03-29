// basic.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "main.h"
#include "monitor_helper.h"

std::wstring st = L"_MONITOR_HELPER_";

enum ThreadId
{
	kThreadUI
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	if (CreateMutex(NULL, TRUE, st.c_str()) != NULL)
	{
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			HWND hwnd = FindWindow(MonitorHelper::kClassName.c_str(), nullptr);
			ShowWindow(hwnd, SW_SHOWNORMAL);
			SetForegroundWindow(hwnd);
			return 0;;
		}
	}

	// �������߳�
	MainThread thread;

	// ִ�����߳�ѭ��
	thread.RunOnCurrentThreadWithLoop(nbase::MessageLoop::kUIMessageLoop);

	return 0;
}

void MainThread::Init()
{
	nbase::ThreadManager::RegisterThread(kThreadUI);

	// ��ȡ��Դ·������ʼ��ȫ�ֲ���
	std::wstring theme_dir = nbase::win32::GetCurrentModuleDirectory();
#ifdef _DEBUG
	// Debug ģʽ��ʹ�ñ����ļ�����Ϊ��Դ
	// Ĭ��Ƥ��ʹ�� resources\\themes\\default
	// Ĭ������ʹ�� resources\\lang\\zh_CN
	// �����޸���ָ�� Startup �����������
	ui::GlobalManager::Startup(theme_dir + L"resources\\", ui::CreateControlCallback(), true);
#else
	// Release ģʽ��ʹ����Դ�е�ѹ������Ϊ��Դ
	// ��Դ�����뵽��Դ�б����Ϊ THEME����Դ����Ϊ IDR_THEME
	// �����Դʹ�õ��Ǳ��ص� zip �ļ�������Դ�е� zip ѹ����
	// ����ʹ�� OpenResZip ��һ�����غ����򿪱��ص���Դѹ����
	ui::GlobalManager::OpenResZip(MAKEINTRESOURCE(IDR_THEME), L"THEME", "");
	// ui::GlobalManager::OpenResZip(L"resources.zip", "");
	ui::GlobalManager::Startup(L"resources\\", ui::CreateControlCallback(), true);
#endif

	ui::GlobalManager::EnableAutomation();

	// ����һ��Ĭ�ϴ�����Ӱ�ľ��д���
	MonitorHelper* window = new MonitorHelper();
	window->Create(NULL, MonitorHelper::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
	window->CenterWindow();
	window->ShowWindow(false);
}

void MainThread::Cleanup()
{
	ui::GlobalManager::Shutdown();
	SetThreadWasQuitProperly(true);
	nbase::ThreadManager::UnregisterThread();
}
