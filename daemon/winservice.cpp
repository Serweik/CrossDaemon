#ifdef Q_OS_WIN
#include "winservice.h"

DaemonGeneric* DaemonSpecific::daemonGeneric = nullptr;

LPTSTR DaemonSpecific::serviceName = nullptr;
SERVICE_STATUS DaemonSpecific::serviceStatus;
SERVICE_STATUS_HANDLE DaemonSpecific::serviceStatusHandle;

DaemonSpecific& DaemonSpecific::instance(std::string DaemonName, DaemonGeneric* _daemonGeneric) {
	static DaemonSpecific singleton(DaemonName);
	daemonGeneric = _daemonGeneric;
	return singleton;
}

DaemonSpecific::DaemonSpecific(std::string DaemonName) {
	if(DaemonName.length() > 0){
		size_t len = DaemonName.length();
		serviceName = new TCHAR[len + 1];
        std::mbstowcs(serviceName, DaemonName.data(), len);
		serviceName[len] = '\0';
	}else {
		serviceName = new TCHAR[12];
		std::wcscpy(serviceName, L"CrossDaemon");
		serviceName[11] = '\0';
	}
}

DaemonSpecific::~DaemonSpecific() {
	delete[] serviceName;
}

int DaemonSpecific::main(int, char**) {
	SERVICE_TABLE_ENTRY serviceTable[] = {
		{serviceName, serviceMain},
		{NULL, NULL}
	};

	if(StartServiceCtrlDispatcher(serviceTable)) {
		return 0;
	}else {
		return GetLastError();
	}
}

void DaemonSpecific::serviceMain(DWORD argc, LPTSTR* argv) {
	serviceStatusHandle = RegisterServiceCtrlHandler(serviceName, serviceCtrlHandler);
	if(serviceStatusHandle == NULL) {
		return;
	}

	char* charArgv[argc];
	for(unsigned int i = 0; i < argc; ++i) {
		size_t len = std::wcslen(argv[i]);
		charArgv[i] = new char[len + 1];
		std::wcstombs(charArgv[i], argv[i], len);
		charArgv[i][len] = '\0';
	}

	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwServiceSpecificExitCode = 101;
	updateStatus(SERVICE_START_PENDING, 0, 0);

	int appReturnCode = 0;

	if(daemonGeneric->initialize(argc, charArgv)) {
		if(daemonGeneric->startDaemon()) {
			updateStatus(SERVICE_RUNNING, 0, 0);
			appReturnCode = daemonGeneric->appMain(argc, charArgv);
		}
	}

	for(unsigned int i = 0; i < argc; ++i) {
		delete[] charArgv[i];
	}

	updateStatus(SERVICE_STOPPED, appReturnCode, 30 * 1000);
}

void DaemonSpecific::serviceCtrlHandler(DWORD ctrlCode) {
	switch(ctrlCode) {
		case SERVICE_CONTROL_STOP: case SERVICE_CONTROL_SHUTDOWN:
			updateStatus(SERVICE_STOP_PENDING, NOERROR, 30 * 1000);
			if(daemonGeneric->stopDaemon()) {
				updateStatus(SERVICE_STOPPED, NOERROR, 30 * 1000);
			}
			break;

		case SERVICE_CONTROL_PAUSE:
			updateStatus(SERVICE_PAUSE_PENDING, NOERROR, 30 * 1000);
			if(daemonGeneric->pauseDaemon()) {
				updateStatus(SERVICE_PAUSED, NOERROR, 30 * 1000);
			}
			break;

		case SERVICE_CONTROL_CONTINUE:
			updateStatus(SERVICE_CONTINUE_PENDING, NOERROR, 30 * 1000);
			if(daemonGeneric->resumeDaemon()) {
				updateStatus(SERVICE_RUNNING, NOERROR, 30 * 1000);
			}
			break;

		case SERVICE_CONTROL_INTERROGATE:
			updateStatus(serviceStatus.dwCurrentState, NOERROR, 30 * 1000);
			break;

		default:
			break;
	}
}

void DaemonSpecific::updateStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) {
	static DWORD dwCheckPoint = 1;
	if((dwCurrentState == SERVICE_START_PENDING) || (dwCurrentState == SERVICE_STOP_PENDING)) {
		serviceStatus.dwControlsAccepted = 0;
	} else {
		serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
	}
	serviceStatus.dwCurrentState = dwCurrentState;
	serviceStatus.dwWin32ExitCode = dwWin32ExitCode;
	serviceStatus.dwWaitHint = dwWaitHint;
	if((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED)) {
		serviceStatus.dwCheckPoint = 0;
	}else {
		serviceStatus.dwCheckPoint = dwCheckPoint++;
	}
	SetServiceStatus(serviceStatusHandle, &serviceStatus);
}

bool DaemonSpecific::installDaemon() {
	bool Result = false;
	SC_HANDLE schSCManager = OpenSCManager(
		NULL, // local machine
		NULL, // ServicesActive database
		SC_MANAGER_ALL_ACCESS); // full access rights

	if(schSCManager != NULL) {
		const int bufsize = MAX_PATH * 4;
		WCHAR servicePath[bufsize];
		if(GetModuleFileName(NULL, servicePath, bufsize)) {
			SC_HANDLE schService = CreateService(
				schSCManager, // SCManager database
				serviceName, // name of service
				serviceName, // service name to display
				SERVICE_ALL_ACCESS, // desired access
				SERVICE_WIN32_OWN_PROCESS, // service type
				SERVICE_DEMAND_START, // start type
				SERVICE_ERROR_NORMAL, // error control type
				servicePath, // path to service's binary
				NULL, // no load ordering group
				NULL, // no tag identifier
				NULL, // no dependencies
				NULL, // LocalSystem account
				NULL); // no password

			if(schService != NULL) {
				Result = true;
			}
		} else {
			std::cout << "GetModuleFileName failed with this error: " << std::to_string(GetLastError()) << std::endl;
		}
		CloseServiceHandle(schSCManager);
	} else {
		int LastError = GetLastError();
		std::cout << "error code = " << std::to_string(LastError) << std::endl;
	}
	return Result;
}

bool DaemonSpecific::uninstallDaemon() {
	bool Result = false;
	SC_HANDLE schSCManager = OpenSCManager(
		NULL, // local machine
		NULL, // ServicesActive database
		SC_MANAGER_ALL_ACCESS); // full access rights

								//Check handle
	if(schSCManager != NULL) {
		SC_HANDLE schService = OpenService(
			schSCManager, // SCManager database
			serviceName, // name of service
			DELETE); // only need DELETE access

		if(schService != NULL) {
			ControlService(schService, SERVICE_CONTROL_STOP, &serviceStatus);
			DeleteService(schService);
			CloseServiceHandle(schSCManager);
			Result = true;
		}
	} else {
		int LastError = GetLastError();
		std::cout << "error code = " << std::to_string(LastError) << std::endl;
	}
	return Result;
}

void DaemonSpecific::setNewStatus(DaemonStatus newStatus, int exitCode) {
	if(newStatus == DaemonStatus::PAUSED) {
		updateStatus(SERVICE_PAUSED, exitCode, 30 * 1000);
	}else if(newStatus == DaemonStatus::RUNNING) {
		updateStatus(SERVICE_RUNNING, exitCode, 30 * 1000);
	}else if(newStatus == DaemonStatus::STOPPED) {
		updateStatus(SERVICE_STOPPED, exitCode, 30 * 1000);
	}
}
#endif
