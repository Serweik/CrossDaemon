#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "QTcpServer"
#include "QTcpSocket"
#include "QFile"
#include "QTextStream"
#include <QMetaType>

class WebServer: public QTcpServer {
	Q_OBJECT
	public:
		WebServer(quint16 _port, QObject* parent = 0);
		~WebServer();

		enum State {
			SERVER_STOPPED,
			SERVER_LISTENING,
			SERVER_ERROR
		};

	signals:
		void stateChanged(WebServer::State state);

	public slots:
		void startServer();
		void stopServer();

	// QTcpServer interface
	protected:
		void incomingConnection(qintptr handle) override;

    private slots:
        void socketRead();
        void socketClosed();

	private:
		quint16 port = 0;

};
Q_DECLARE_METATYPE(WebServer::State)

#endif // WEBSERVER_H
