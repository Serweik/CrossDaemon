#ifndef DAEMONGENERIC_H
#define DAEMONGENERIC_H

enum class DaemonStatus {
	STOPPED,
	RUNNING,
	PAUSED
};

class DaemonGeneric {
	public:
		virtual ~DaemonGeneric() {}
		virtual void printHelp() = 0;
		virtual bool initialize(int argc, char * argv[]) = 0;
		virtual int appMain(int argc, char* argv[]) = 0;
		virtual bool startDaemon() = 0;
		virtual bool pauseDaemon() = 0;
		virtual bool resumeDaemon() = 0;
		virtual bool stopDaemon() = 0;
		virtual DaemonStatus currentStatus() = 0;
};

#endif // DAEMONGENERIC_H
