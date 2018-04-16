#include "webservercontroller.h"
#include "qtdaemon.h"

WebServerController::WebServerController(QObject* parent, QtDaemon* _qtDaemon):
	QObject(parent), qtDaemon(_qtDaemon) {

}

WebServerController::~WebServerController() {
	if(serverThread != nullptr) {
		serverThread->quit();
		serverThread->wait(5000);
		delete serverThread;
		delete webServer;
	}
}

bool WebServerController::initialize() {
	serverThread = new QThread();
	webServer = new WebServer(8080);

	connect(this, SIGNAL(startWebServer()), webServer, SLOT(startServer()), Qt::QueuedConnection);
	connect(this, SIGNAL(stopWebServer()), webServer, SLOT(stopServer()), Qt::QueuedConnection);
	connect(webServer, SIGNAL(stateChanged(WebServer::State)), this, SLOT(serverStateChanged(WebServer::State)), Qt::QueuedConnection);
	webServer->startServer();
	webServer->moveToThread(serverThread);
	serverThread->start();
	return true;
}

void WebServerController::startServer() {
	emit startWebServer();
}

void WebServerController::stopServer() {
	emit stopWebServer();
}

void WebServerController::serverStateChanged(WebServer::State state) {
	qtDaemon->serverStatusChanged(state);
}
