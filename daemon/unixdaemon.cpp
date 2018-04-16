#ifndef Q_OS_WIN
#include "unixdaemon.h"

DaemonGeneric* DaemonSpecific::daemonGeneric = nullptr;
char* DaemonSpecific::socketPath = nullptr;
char* DaemonSpecific::pidFilePath = nullptr;
char* DaemonSpecific::testMessage = nullptr;
char* DaemonSpecific::testResponse = nullptr;
bool DaemonSpecific::needFinishSocketThread = false;

DaemonSpecific& DaemonSpecific::instance(std::string DaemonName, DaemonGeneric* _daemonGeneric) {
    static DaemonSpecific singleton(DaemonName);
    daemonGeneric = _daemonGeneric;
    return singleton;
}

DaemonSpecific::DaemonSpecific(std::string DaemonName) {
    size_t len = DaemonName.length();
    if(len > 0){
        daemonName = new char[len + 1];
        strncpy(daemonName, DaemonName.data(), len);
        daemonName[len] = '\0';
    }else {
        len = 11;
        daemonName = new char[12];
        strcpy(daemonName, "CrossDaemon");
        daemonName[len] = '\0';
    }
    socketPath = new char[len + 5 + 1];
    strcpy(socketPath, "/tmp/");
    strcat(socketPath, daemonName);
    pidFilePath = new char[len + 9 + 1];
    strcpy(pidFilePath, socketPath);
    strcat(pidFilePath, ".pid");
    testMessage = new char[15];
    strcpy(testMessage, "You all right?");
    testResponse = new char[len + 7 + 1];
    strcpy(testResponse, daemonName);
    strcat(testResponse, " is ok!");
}

DaemonSpecific::~DaemonSpecific() {
    delete [] daemonName;
    delete [] socketPath;
    delete [] pidFilePath;
    delete [] testMessage;
    delete [] testResponse;
}

void DaemonSpecific::setFdLimit(int MaxFd) {
   rlimit lim;
   lim.rlim_cur = MaxFd;
   lim.rlim_max = MaxFd;
   setrlimit(RLIMIT_NOFILE, &lim);
}

void DaemonSpecific::signalConfig() {
    struct sigaction sa;
    sigset_t newset;
    sigemptyset(&newset);
    sigaddset(&newset, SIGHUP);
    sigaddset(&newset, SIGTERM);
    sigaddset(&newset, SIGINT);
    sigaddset(&newset, SIGTSTP);
    sigaddset(&newset, SIGCONT);
    sigprocmask(SIG_BLOCK, &newset, 0);
    sa.sa_handler = termHandler;
    sigaction(SIGTERM, &sa, 0);
    sigaction(SIGINT, &sa, 0);
    sa.sa_handler = pauseHandler;
    sigaction(SIGTSTP, &sa, 0);
    sa.sa_handler = resumeHandler;
    sigaction(SIGCONT, &sa, 0);
}

int DaemonSpecific::main(int argc, char* argv[]) {
    umask(0);
    if(access(socketPath, F_OK) == 0) {
        int socketDescr = 0;
        int cycleCount = 0;
        if((socketDescr = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0) {
            fcntl(socketDescr, F_SETFL, O_NONBLOCK);
            sockaddr_un address;
            address.sun_family = AF_UNIX;
            strcpy(address.sun_path, socketPath);
            if(connect(socketDescr, (sockaddr*)&address, sizeof(address)) != -1) {
                int forNoWarning = write(socketDescr, testMessage, sizeof(testMessage));
                char response[50] = "";
                if(forNoWarning == 0) {
                    response[0] = '\0';
                }

                while(cycleCount < 10) {
                    int numSymbols = read(socketDescr, response, 49);
                    if((numSymbols > 0) && (strncmp(response, testResponse, numSymbols) == 0)) {
                        cycleCount = -1;
                        break;
                    }else {
                        ++cycleCount;
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }
            }
            close(socketDescr);
        }
        if(cycleCount != -1) {
            //socketFile можно удалять, демон не запущен
            unlink(socketPath);
            unlink(pidFilePath);
        }else {
            std::cout << "Daemon already run!" << std::endl;
            //демон уже работает
            return 0;
        }
    }
    std::cout << "Starting daemon" << std::endl;

    int pid = fork();

    if(pid == -1) {
        std::cout << "Start daemon is Failed" << std::endl;
        return 1;
    }else if(pid != 0) {
        //родитель
        std::cout << "Daemon started" << std::endl;
        return 0;
    }else {
        //потомок
        int sid = setsid();

        if(sid < 0) {
            return 1;
        }else {
            if(chdir("/") < 0) {
                return 1;
            }else {
                close(STDIN_FILENO);
                close(STDOUT_FILENO);
                close(STDERR_FILENO);

                setFdLimit(1024*10); //file descryptor limit

                if(daemonGeneric->initialize(argc, argv)) {
                    if(daemonGeneric->startDaemon()) {
                        int result = -1;
                        std::ofstream pidFile(pidFilePath);
                        if(pidFile.is_open()) {
                            pidFile << (size_t)getpid();
                            pidFile.close();

                            int socketDescr = socket(AF_UNIX, SOCK_STREAM, 0);
                            if(socketDescr >= 0) {
                                sockaddr_un server_address;
                                server_address.sun_family = AF_UNIX;
                                strcpy(server_address.sun_path, socketPath);
                                fcntl(socketDescr, F_SETFL, O_NONBLOCK);
                                if(bind(socketDescr, (sockaddr*)&server_address, sizeof(server_address)) != -1) {
                                    if(listen(socketDescr, 2) != -1) {
                                        std::thread serverThread(serverHandler, socketDescr);
                                        signalConfig();
                                        result = daemonGeneric->appMain(argc, argv);
                                        needFinishSocketThread = true;
                                        serverThread.join();
                                    }
                                }
                                close(socketDescr);
                                unlink(socketPath);
                            }
                            unlink(pidFilePath);
                        }
                        return result;
                    }else {
                        return 1;
                    }
                }else {
                    return 1;
                }
            }
        }
    }
}

bool DaemonSpecific::installDaemon() {
    return true;
}

bool DaemonSpecific::uninstallDaemon() {
    return true;
}

void DaemonSpecific::terminateDaemonByPid() {
    std::ifstream pidFile(pidFilePath);
    if(pidFile.is_open()) {
        size_t pid = 0;
        pidFile >> pid;
        pidFile.close();
        if(pid != 0) {
            kill(pid, SIGTERM);
        }
    }
}

void DaemonSpecific::serverHandler(int serverSocketDescr) {
    sockaddr_un client_address;
    socklen_t clientLen = sizeof(client_address);
    int clientSocket;
    int cycleCount = 0;
    while(!needFinishSocketThread) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if((clientSocket = accept(serverSocketDescr, (sockaddr*)&client_address, &clientLen)) >= 0) {
            char response[50] = "";
            while(!needFinishSocketThread) {
                int numSymbols = read(clientSocket, response, 49);
                if((numSymbols > 0) && (strncmp(response, testMessage, numSymbols) == 0)) {
                    int forNoWarning = write(clientSocket, testResponse, sizeof(testResponse));
                    if(forNoWarning == 0) {
                        numSymbols = 0;
                    }
                }else if(numSymbols == -1 && errno == EAGAIN) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }else {
                    break;
                }
            }
            close(clientSocket);
        }else if(clientSocket == -1 && errno != EAGAIN) {
            std::exit(100500); //big error!
        }else if(cycleCount > 10) {
            cycleCount = 0;
            if(access(socketPath, F_OK) != 0) {
                std::exit(100500); //big error!
            }
        }else {
            ++cycleCount;
        }
    }
}

void DaemonSpecific::termHandler(int) {
    daemonGeneric->stopDaemon();
}

void DaemonSpecific::pauseHandler(int) {
    daemonGeneric->pauseDaemon();
}

void DaemonSpecific::resumeHandler(int) {
    daemonGeneric->resumeDaemon();
}

void DaemonSpecific::setNewStatus(DaemonStatus, int) {
    //need for compatibility with Windows
}
#endif
