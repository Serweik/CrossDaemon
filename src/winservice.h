#ifndef WINSERVICE_H
#define WINSERVICE_H

#include "windows.h"
#include "tchar.h"
#include "iostream"
#include "string"

#include "daemongeneric.h"

class DaemonSpecific {
	public:
		static DaemonSpecific& instance(std::string DaemonName, DaemonGeneric* _daemonGeneric);

		int main(int argc, char* argv[]);
		static void WINAPI serviceMain(DWORD argc, LPTSTR* argv);
		static void WINAPI serviceCtrlHandler(DWORD ctrlCode);
		bool installDaemon();
		bool uninstallDaemon();
        void terminateDaemonByPid() {} //for unix compatibility
		static void setNewStatus(DaemonStatus newStatus, int exitCode);

	private:
		DaemonSpecific(std::string DaemonName);
		~DaemonSpecific();
		DaemonSpecific(const DaemonSpecific&) {}
		DaemonSpecific& operator = (const DaemonSpecific&) {return *this;}
		DaemonSpecific(DaemonSpecific&&) {}
		DaemonSpecific& operator = (DaemonSpecific&&) {return *this;}

		static void updateStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);

		static DaemonGeneric* daemonGeneric;

		static LPTSTR serviceName;
		static SERVICE_STATUS serviceStatus;
		static SERVICE_STATUS_HANDLE serviceStatusHandle;
};

#endif // WINSERVICE_H
