#include "stdafx.h"
#include "monitor_helper.h"
#include "resource.h"
#include "util.h"
#include "app_config.h"

#include <shellapi.h>
#include <strsafe.h>
#include <future>

using namespace nim_comp;


UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;
class __declspec(uuid("9D0B8B92-4E1C-488e-A1E1-2331AFCE2CB5")) PrinterIcon;
GUID iconGuid = __uuidof(PrinterIcon);

const std::wstring MonitorHelper::kClassName = L"MonitorHelper";

bool isKeyboardWakeUp = true;
bool isMonitorPowerOn = true;


MonitorHelper::MonitorHelper()
{
}


MonitorHelper::~MonitorHelper()
{
}

std::wstring MonitorHelper::GetSkinFolder()
{
	return L"monitor_helper";
}

std::wstring MonitorHelper::GetSkinFile()
{
	return L"monitor_helper.xml";
}

std::wstring MonitorHelper::GetWindowClassName() const
{
	return kClassName;
}

void MonitorHelper::PopMenu()
{
	POINT point;
	GetCursorPos(&point);
	HMENU hMenu = LoadMenu(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_MENU1));
	HMENU hPopup = GetSubMenu(hMenu, 0);
	SetForegroundWindow(GetHWND());
	TrackPopupMenuEx(hPopup, TPM_LEFTALIGN, point.x, point.y, GetHWND(), NULL);
	PostMessage(WM_NULL, 0, 0);
	DestroyMenu(hMenu);
}

LRESULT MonitorHelper::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_INPUT:
		isKeyboardWakeUp = true;
		break;

	case WM_POWERBROADCAST:
		if (wParam == PBT_POWERSETTINGCHANGE) {
			POWERBROADCAST_SETTING* powerSetting = (POWERBROADCAST_SETTING*)lParam;
			if (powerSetting->PowerSetting == GUID_CONSOLE_DISPLAY_STATE || powerSetting->PowerSetting == GUID_MONITOR_POWER_ON) {
				// 关闭屏幕广播
				if (powerSetting->Data[0] == PowerMonitorOff)
				{
					isMonitorPowerOn = false;
					isKeyboardWakeUp = false;
				}
				// 开启屏幕广播
				else if (powerSetting->Data[0] == PowerMonitorOn)
				{
					isMonitorPowerOn = true;
					if (AppConfig::disableMouseWakeUpDisplay && !isKeyboardWakeUp)
					{
						this->ShutdownMonitor();
					}
					
				}
			}
		}
		break;

	case WMAPP_NOTIFYCALLBACK:
		switch (LOWORD(lParam))
		{
		case WM_LBUTTONUP:
			ShutdownMonitor();
			break;

		case WM_CONTEXTMENU:
			PopMenu();
			break;
		}
		break;

	case WM_COMMAND:
		switch (wParam)
		{
		case IDM_MAIN_VIEW:
			this->ShowWindow();
			SetForegroundWindow(GetHWND());
			break;
		case IDM_DISPLAYOFF:
			ShutdownMonitor();
			break;
		case IDM_LOCK:
			this->LockCompoter();
			break;
		case IDM_LOCK_PLUS:
			ShutdownMonitor();
			this->LockCompoter();
			break;
		case IDM_EXIT:
			this->Close();
			break;
		default:
			break;
		}
	}

	return __super::HandleMessage(uMsg, wParam, lParam);
}

void MonitorHelper::InitWindow()
{
	AppConfig::LoadData();

	this->InitEventBind();

	this->SetIcon(IDI_BASIC);
	this->RegisterDisplayStateEvent();
	this->AddNotificationIcon();

	if (AppConfig::disableMouseWakeUpDisplay)
	{
		this->RegisitKeyBordEvent();
	}
}

void MonitorHelper::LockCompoter()
{
	if (!LockWorkStation())
	{
		Toast::ShowToast(L"锁定失败，请重试！", 5000, NULL);
		while (!isMonitorPowerOn)
		{
			keybd_event(VK_LEFT, 0, 0, 0);
			keybd_event(VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
			keybd_event(VK_RIGHT, 0, 0, 0);
			keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
			Sleep(300);
		}
	}
}


bool MonitorHelper::RegisitKeyBordEvent()
{
	RAWINPUTDEVICE rid;  //设备信息
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x06; //键盘   rid.usUsagePage = 0x01; rid.usUsage = 0x02; 为鼠标
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = GetHWND();
	
	return RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));
}

bool MonitorHelper::UnRegisitKeyBordEvent()
{
	RAWINPUTDEVICE rid;  //设备信息
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x06; //键盘   rid.usUsagePage = 0x01; rid.usUsage = 0x02; 为鼠标
	rid.dwFlags = RIDEV_REMOVE;
	rid.hwndTarget = NULL;

	return RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));
}

void MonitorHelper::ShutdownMonitor()
{
	isKeyboardWakeUp = false;
	// SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
	SendMessage(WM_SYSCOMMAND, SC_MONITORPOWER, 2);
}

bool MonitorHelper::MinimizeWindow(ui::EventArgs* args)
{
	this->ShowWindow(false);
	return true;
}

bool MonitorHelper::InitEventBind()
{
	creatDesktopLinkBtn = dynamic_cast<ui::Button*>(FindControl(L"create_shortcut"));
	creatDesktopLinkBtn->AttachClick(nbase::Bind(&MonitorHelper::OnCreateShortCutEvent, this, std::placeholders::_1));

	ui::Button* closeBtn = dynamic_cast<ui::Button*>(FindControl(L"minimize_window"));
	closeBtn->AttachClick(nbase::Bind(&MonitorHelper::MinimizeWindow, this, std::placeholders::_1));

	startupWithWindowsChebox = dynamic_cast<ui::CheckBox*>(FindControl(L"startup"));
	startupWithWindowsChebox->AttachSelect(nbase::Bind(&MonitorHelper::OnSetStartupWithWindows, this, std::placeholders::_1));
	startupWithWindowsChebox->AttachUnSelect(nbase::Bind(&MonitorHelper::OnSetStartupWithWindows, this, std::placeholders::_1));
	startupWithWindowsChebox->Selected(AppConfig::startupWithWindows);

	disableMouseMoveWakeUpCheBox = dynamic_cast<ui::CheckBox*>(FindControl(L"disable_mouse_wake_up"));
	disableMouseMoveWakeUpCheBox->AttachSelect(nbase::Bind(&MonitorHelper::OnSetDisableMouseWakeUpDisplay, this, std::placeholders::_1));
	disableMouseMoveWakeUpCheBox->AttachUnSelect(nbase::Bind(&MonitorHelper::OnSetDisableMouseWakeUpDisplay, this, std::placeholders::_1));
	disableMouseMoveWakeUpCheBox->Selected(AppConfig::disableMouseWakeUpDisplay);
	
	return true;
}

bool MonitorHelper::OnSetStartupWithWindows(ui::EventArgs* args)
{
	if (startupWithWindowsChebox->IsSelected())
	{
		SetStartupWithWindows();
	}
	else
	{
		CancleStartupWithWindows();
	}
	bool ret = SaveIntData(AppConfig::KEY_STARTUP_WITH_WINDOWS, startupWithWindowsChebox->IsSelected());
	AppConfig::LoadData();
	return ret;
}

bool MonitorHelper::OnSetDisableMouseWakeUpDisplay(ui::EventArgs* args)
{
	if (disableMouseMoveWakeUpCheBox->IsSelected())
	{
		this->RegisitKeyBordEvent();
	}
	else
	{
		this->UnRegisitKeyBordEvent();
	}
	bool ret = SaveIntData(AppConfig::KEY_DISABLE_MOUSE_WAKE_UP_DISPLAY, disableMouseMoveWakeUpCheBox->IsSelected());
	AppConfig::LoadData();
	return ret;
}

HPOWERNOTIFY MonitorHelper::RegisterDisplayStateEvent() {
	if (isWin7)
	{
		return RegisterPowerSettingNotification(this->GetHWND(), &GUID_MONITOR_POWER_ON, DEVICE_NOTIFY_WINDOW_HANDLE);
	}
	else if (IsGreaterThanOrEqualToWin8())
	{
		return RegisterPowerSettingNotification(this->GetHWND(), &GUID_CONSOLE_DISPLAY_STATE, DEVICE_NOTIFY_WINDOW_HANDLE);
	}
}

void MonitorHelper::UnRegisterDisplayStateEvent() {
	UnregisterPowerSettingNotification(hPowerNotify);
}


bool MonitorHelper::OnCreateShortCutEvent(ui::EventArgs* args)
{
	CreatShortCutToDesktop();
	return true;
}

bool MonitorHelper::OnShoudownDisplayClick(ui::EventArgs* args)
{
	ShutdownMonitor();
	return true;
}


bool MonitorHelper::AddNotificationIcon()
{

	NOTIFYICONDATA nid = {};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = GetHWND();
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
	nid.uID = IDI_BASIC;
	nid.guidItem = iconGuid;

	nid.hIcon = LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_BASIC));

	//StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), L"息屏助手");

	nid.uVersion = NOTIFYICON_VERSION_4;

	Shell_NotifyIcon(NIM_ADD, &nid);

	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

// 不知道为什么没有生效
bool MonitorHelper::DeleteNotificationIcon()
{
	NOTIFYICONDATA nid = {};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.uFlags = NIF_GUID;
	nid.uID = IDI_BASIC;
	nid.guidItem = iconGuid;
	return Shell_NotifyIcon(NIM_DELETE, &nid);
}

LRESULT MonitorHelper::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	this->UnRegisterDisplayStateEvent();
	this->UnRegisitKeyBordEvent();
	this->DeleteNotificationIcon();
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}
