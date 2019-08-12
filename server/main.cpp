/**
 * Created by Davide Sordi
 * Using CLion
 * Date: 12/08/2019
 * Time: 15.32
 * 
 * Class: main
 * Project: textEditor
 */

#include <iostream>
#include <QCoreApplication>
#include "Networking/Server.h"

int main(int argc, char *argv[]) {
	QCoreApplication a(argc, argv);

	// Make a server and starts it
	Server server;
	server.startServer(1234);

	return a.exec();
}