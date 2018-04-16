#ifndef QTDAEMON_H
#define QTDAEMON_H

#include "daemon/daemon.h"

#include <QCoreApplication>
#include "webservercontroller.h"
#include "QDateTime"

class QtDaemon: public Daemon {
	public:
		~QtDaemon();

		void serverStatusChanged(WebServer::State serverState);

	protected:
		// DaemonGeneric interface
		int appMain(int argc, char* argv[]) override;
		bool pauseDaemon() override;
		bool resumeDaemon() override;
		bool stopDaemon() override;

	private:
		QCoreApplication* qtApp = nullptr;
		WebServerController* webServerController = nullptr;
};

#endif // QTDAEMON_H
