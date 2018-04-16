#include "qtdaemon.h"

int main(int argc, char *argv[]) {
	QtDaemon deamon;
	return deamon.main("QtDaemon", argc, argv);
}
