#ifndef DAEMON_H
#define DAEMON_H

#include "daemongeneric.h"

#ifdef DG_OS_WIN
	#include "winservice.h"
#else
    #include "unixdaemon.h"
#endif

class Daemon: protected DaemonGeneric {
	public:
		int main(std::string DaemonName, int argc, char* argv[]);

	protected:
		// DaemonGeneric interface
		void printHelp() override;
		bool initialize(int argc, char* argv[]) override;
		int appMain(int argc, char* argv[]) override;
		bool startDaemon() override;
		bool pauseDaemon() override;
		bool resumeDaemon() override;
		bool stopDaemon() override;
		DaemonStatus currentStatus() override;

	protected:
		DaemonSpecific* daemon;
		DaemonStatus daemonStatus = DaemonStatus::STOPPED;
};

#endif // DAEMON_H
