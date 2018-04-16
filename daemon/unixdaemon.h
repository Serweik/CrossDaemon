#ifndef UNIXDAEMON_H
#define UNIXDAEMON_H

#include "iostream"
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/un.h>
#include "sys/stat.h"
#include "unistd.h"
#include <signal.h>
#include <fcntl.h>

#include "daemongeneric.h"

class DaemonSpecific {
    public:
        static DaemonSpecific& instance(std::string DaemonName, DaemonGeneric* _daemonGeneric);

        int main(int argc, char* argv[]);
        bool installDaemon();
        bool uninstallDaemon();
        void terminateDaemonByPid();
        static void setNewStatus(DaemonStatus newStatus, int exitCode);
        static void serverHandler(int serverSocketDescr);
        static void termHandler(int sig);
        static void pauseHandler(int sig);
        static void resumeHandler(int sig);

    private:
        DaemonSpecific(std::string DaemonName);
        ~DaemonSpecific();
        DaemonSpecific(const DaemonSpecific&) {}
        DaemonSpecific& operator = (const DaemonSpecific&) {return *this;}
        DaemonSpecific(DaemonSpecific&&) {}
        DaemonSpecific& operator = (DaemonSpecific&&) {return *this;}

        void setFdLimit(int MaxFd);
        void signalConfig();

        static DaemonGeneric* daemonGeneric;

        char* daemonName;
        static char* socketPath;
        static char* pidFilePath;
        static char* testMessage;
        static char* testResponse;
        static bool needFinishSocketThread;
};

#endif // UNIXDAEMON_H
