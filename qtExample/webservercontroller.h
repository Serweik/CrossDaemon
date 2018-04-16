#ifndef WEBSERVERCONTROLLER_H
#define WEBSERVERCONTROLLER_H

#include <QObject>
#include "webserver.h"
#include <QThread>
#include <QFile>
#include <QTextStream>

class QtDaemon;

class WebServerController: public QObject {
	Q_OBJECT
	public:
		explicit WebServerController(QObject* parent = nullptr, QtDaemon* _qtDaemon = nullptr);
		~WebServerController();

		bool initialize();
		void startServer();
		void stopServer();

	signals:
		void startWebServer();
		void stopWebServer();

	public slots:
		void serverStateChanged(WebServer::State state);

	private:
		QtDaemon* qtDaemon = nullptr;

		WebServer* webServer = nullptr;
		QThread* serverThread = nullptr;
};

#endif // WEBSERVERCONTROLLER_H
