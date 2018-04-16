#include "webserver.h"

WebServer::WebServer(quint16 _port, QObject* parent):
	QTcpServer(parent), port(_port) {

	qRegisterMetaType<WebServer::State>();
}

WebServer::~WebServer() {
	if(isListening()) {
		close();
	}
}

void WebServer::startServer() {
	if(!isListening()) {
		if(listen(QHostAddress::Any, port)) {
			emit stateChanged(SERVER_LISTENING);
		}else {
			emit stateChanged(SERVER_ERROR);
		}
	}else {
		emit stateChanged(SERVER_LISTENING);
	}
}

void WebServer::stopServer() {
	if(isListening()) {
		close();
	}
	emit stateChanged(SERVER_STOPPED);
}

void WebServer::incomingConnection(qintptr handle) {
	QTcpSocket* socket = new QTcpSocket(this);
	socket->setSocketDescriptor(handle);
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketClosed()));
}

void WebServer::socketRead() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if(socket) {
        QString data = QString::fromLatin1(socket->readAll());
        if(data.startsWith("GET")) {
            socket->write("HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html\r\n"
                          "Content-Length: 74"
                          "Connection: close\r\n\r\n"
                          "<!DOCTYPE html>"
                          "<html>"
                              "<head></head>"
                              "<body>"
                                  "Web server is running!"
                              "</body>"
                          "</html>");
            socket->waitForBytesWritten();
            socket->close();
        }
    }
}

void WebServer::socketClosed() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if(socket) {
       socket->deleteLater();
    }
}
