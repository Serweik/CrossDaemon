#include "daemon.h"

int Daemon::main(std::string DaemonName, int argc, char* argv[]) {
	daemon = (DaemonSpecific*)&DaemonSpecific::instance(DaemonName, this);
	if(argc > 1) {
		if((strcmp(argv[1], "-i") == 0) || (strcmp(argv[1], "-install") == 0)) {
			std::cout << "Installing the service..." << std::endl;
			if(daemon->installDaemon()) {
				std::cout << "Installation has been successfully complete." << std::endl;
			} else {
				std::cout << "Installation error." << std::endl;
			}
        }else if((strcmp(argv[1], "-u") == 0) || (strcmp(argv[1], "-uninstall") == 0)) {
			std::cout << "Uninstalling the service..." << std::endl;
			if(daemon->uninstallDaemon()) {
				std::cout << "Uninstallation has been successfully complete." << std::endl;
			} else {
				std::cout << "Uninstallation error." << std::endl;
			}
        }else if((strcmp(argv[1], "-s") == 0) || (strcmp(argv[1], "-stop") == 0)) {
            daemon->terminateDaemonByPid();
        }else {
			printHelp();
		}
		return 0;
	} else {
		return daemon->main(argc, argv);
	}
}

void Daemon::printHelp() {
	std::cout << "\"-i/-install\" for installation of the service,\n"
				 "\"-u/-uninstall\" for uninstallation of the service,\n" << std::endl;
}

bool Daemon::initialize(int, char**) {
	return true;
}

int Daemon::appMain(int, char**) {
	while(daemonStatus != DaemonStatus::STOPPED) {
		if(daemonStatus == DaemonStatus::RUNNING) {
			//do something
		}
	}
	return 0;
}

bool Daemon::startDaemon() {
	daemonStatus = DaemonStatus::RUNNING;
	return true;
}

bool Daemon::pauseDaemon() {
	daemonStatus = DaemonStatus::PAUSED;
	return true;
}

bool Daemon::resumeDaemon() {
	daemonStatus = DaemonStatus::RUNNING;
	return true;
}

bool Daemon::stopDaemon() {
	daemonStatus = DaemonStatus::STOPPED;
	return true;
}

DaemonStatus Daemon::currentStatus() {
	return daemonStatus;
}
