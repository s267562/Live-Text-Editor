//
// Created by Eugenio Marinelli on 2019-08-21.
//

#include "MainWindow.h"

MainWindow::MainWindow(QString siteId)
        : editor(new Editor(siteId,this)),
          login(new Login(this)),
          client(new Client(this)),
          crdt(new CRDT(siteId)),
          controller(new Controller(this->crdt,this->editor,this->client)),
          connection(new Connection(this)),
          registration(new Registration(this)),
          finder(new ShowFiles(this))
{
    this->login->setClient(this->client);

    /* define connection */
    connect(this->connection, SIGNAL(connectToAddress(QString)),this, SLOT(connectClient(QString)));
    connect(this->client, SIGNAL(fileNames(QStringList)),this, SLOT(showFileFinder(QStringList)));
}

void MainWindow::show() {
    this->connection->show();
}

void MainWindow::showFileFinder(QStringList fileList){
    this->login->close();
    //TODO: Popolare la WidgetList con i nomi dei file che arrivano dal server
    this->finder->addFiles(fileList);
    this->finder->show();
}

void MainWindow::showEditor(){
    this->login->close();
    this->editor->show();
}

void MainWindow::showLogin(){
    this->registration->close();
    this->login->show();
}

void MainWindow::connectClient(QString address) {
    bool res=this->client->connectTo(address);
    if(res) {
        this->connection->close();
        this->login->show();
    }
}

void MainWindow::showRegistration(){
    this->login->close();
    this->registration->show();
}

void MainWindow::requestForFile(QString filename){

    bool result = this->client->requestForFile(filename);

    if ( result ){
        this->editor->show();
    }
}