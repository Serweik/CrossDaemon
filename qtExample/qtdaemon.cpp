#include "qtdaemon.h"

QtDaemon::~QtDaemon() {
	if(qtApp != nullptr) {
		delete webServerController;
		delete qtApp;
	}
}

int QtDaemon::appMain(int argc, char* argv[]) {
	qtApp = new QCoreApplication(argc, argv);

	webServerController = new WebServerController(nullptr, this);
	webServerController->initialize();
	int result = qtApp->exec();

	delete webServerController;
	delete qtApp;
	qtApp = nullptr;
	return result;
}

bool QtDaemon::pauseDaemon() {
	Daemon::pauseDaemon();
	webServerController->stopServer();
	return false;
}

bool QtDaemon::resumeDaemon() {
	Daemon::resumeDaemon();
	webServerController->startServer();
	return false;
}

bool QtDaemon::stopDaemon() {
	Daemon::stopDaemon();
	webServerController->stopServer();
	return false;
}

void QtDaemon::serverStatusChanged(WebServer::State serverState) {
	if(serverState == WebServer::SERVER_STOPPED) {
		if(currentStatus() == DaemonStatus::PAUSED) {
			daemon->setNewStatus(DaemonStatus::PAUSED, 0);
		}else {
			daemon->setNewStatus(DaemonStatus::STOPPED, 0);
			qtApp->exit(0);
		}
	}else if(serverState == WebServer::SERVER_LISTENING) {
		if(currentStatus() == DaemonStatus::RUNNING) {
			daemon->setNewStatus(DaemonStatus::RUNNING, 0);
		}else {
			//error
			daemon->setNewStatus(DaemonStatus::STOPPED, 1);
			qtApp->exit(1);
		}
	}else {
		daemon->setNewStatus(DaemonStatus::STOPPED, 1);
		qtApp->exit(1);
	}
}
