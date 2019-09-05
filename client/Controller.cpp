//
// Created by simone on 09/08/19.
//

#include "Controller.h"
#include <QMessageBox>

Controller::Controller(): client(new Client(this)), connection(new Connection(this)){
    user = nullptr;
    editor = nullptr;
    crdt = new CRDT();
    client->setCRDT(crdt);

    /* creation connection and client object */
    connect(this->client, &Client::errorConnection, this, &Controller::errorConnection);
    connect(client, SIGNAL(fileRecive(std::vector<std::vector<Character>>)), this, SLOT(openFile(std::vector<std::vector<Character>>)));
    connect(this->connection, SIGNAL(connectToAddress(QString)),this, SLOT(connectClient(QString)));
    connect(client, &Client::newMessage,
            this, &Controller::newMessage);
    now = connection;
    connection->show();
}

Controller::Controller(CRDT *crdt, Editor *editor, Client *client) : crdt(crdt), editor(editor), client(client) {
    editor->setController(this);

    // Controller
    connect(client, &Client::newMessage,
              this, &Controller::newMessage);

    connect(editor, &Editor::logout, client, &Client::logOut);
    connect(client, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
    connect(client, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));
    connect(client, SIGNAL(fileRecive(std::vector<std::vector<Character>>)), this, SLOT(openFile(std::vector<std::vector<Character>>)));
}

/* NETWORKING */

void Controller::errorConnection(){
    QMessageBox::information(this, "Connection", "Try again, connection not established!");
    // TODO: retry...
}

/* CONNECTION */

void Controller::connectClient(QString address) {
    bool res = this->client->connectTo(address);    // TODO: non va...

    if (res) {
        this->connection->close();

        /* creation login object */
        login = new Login(this);
        now = login;
        login->setClient(client);
        connect(this->client, &Client::loginFailed, this->login, &Login::loginFailed);
        connect(this->client, &Client::logout, this, &Controller::showLogin);
        connect(this->login, SIGNAL(showRegistration()), this, SLOT(showRegistration()));
        //connect(this->login, SIGNAL(loginSuccessful()), this, SLOT(showFileFinder()));

        /* creation registration object */
        registration = new Registration(this);
        connect(this->registration, SIGNAL(showLogin()), this, SLOT(showLogin()));

        /* creation showfiles object */
        finder = new ShowFiles(this);
        connect(this->finder, &ShowFiles::logout, this->client, &Client::logOut);
        connect(this->client, SIGNAL(fileNames(QStringList)), this, SLOT(showFileFinder(QStringList)));
        connect(this->finder, SIGNAL(newFile(QString)), this, SLOT(requestForFile(QString)));

        this->login->show();
    }
}

/* LOGIN */

void Controller::showLogin(){
    now->close();
    this->login->reset();
    now = login;
    this->login->show();
}

/* REGISTRATION */

void Controller::showRegistration(){
    now->close();
    this->registration->reset();
    this->registration->show();
}

/* SHOW FILE */

void Controller::showFileFinder(QStringList fileList){
    now->close();
    this->finder->addFiles(fileList);
    if (user == nullptr){
        user = new User(login->getUsername());
        user->setFileLis(fileList);
        user->setIsLogged(true);
    }
    now = finder;
    this->finder->show();
}

void Controller::showFileFinderOtherView(){
    now->close();
    /*delete finder;
    finder = new ShowFiles(this);
    this->finder->addFiles(user->getFileList());
    this->editor->close();*/
    /*connect(this->finder, &ShowFiles::logout, this->client, &Client::logOut);
    connect(this->editor, &Editor::showFinder, this, &Controller::showFileFinderOtherView);*/
    /*connect(client, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
    connect(client, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));*/
    now = finder;
    this->finder->show();
}

/* EDITOR */

void Controller::requestForFile(QString filename){
    bool result = this->client->requestForFile(filename);

    if (result){
        if (editor == nullptr){
            siteId = user->getUsername();
            editor = new Editor(siteId, this, this);

            /* connecting */
            connect(this->editor, &Editor::showFinder, this, &Controller::showFileFinderOtherView);
            connect(client, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
            connect(client, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));
            connect(this->finder, &ShowFiles::logout, this->client, &Client::logOut);
            connect(this->editor, &Editor::showFinder, this, &Controller::showFileFinderOtherView);
            connect(editor, &Editor::logout, client, &Client::logOut);
        }else{
            editor->reset();
        }
        now->close();
        now = editor;
        editor->show();
    }
}

void Controller::showEditor(){
    editor->reset();
    editor->show();
}

void Controller::localInsert(QString chars, Pos startPos) {
    // send insert at the server. To insert it in the model we need the position computed by the server.
    this->client->insert(chars, startPos);
}

void Controller::localDelete(Pos startPos, Pos endPos) {
    std::vector<Character> removedChars = this->crdt->handleDelete(startPos, endPos);

    for(Character c : removedChars) {
        this->client->deleteChar(QString{c.getValue()}, c.getPosition());
    }
}

void Controller::newMessage(Message message) {
    // Message message = this->client->getMessage();

    if(message.getType() == INSERT) {
        Character character = message.getCharacter();

        Pos pos = this->crdt->insert(character);

        if(character.getSiteId() == this->crdt->getSiteId()) {
            // local insert - only in the model; the char is already in the view.
        } else {
            // remote insert - the char is to insert in the model and in the view. Insert into the editor.
            this->editor->insertChar(character.getValue(), pos);
        }
    } else if(message.getType() == DELETE) {
        if(!(message.getCharacter().getSiteId() == this->crdt->getSiteId())) {
            Pos pos = this->crdt->handleRemoteDelete(message.getCharacter());

            if(pos) {
                // delete from the editor.
                this->editor->deleteChar(pos);
            }
        }
    }
}

void Controller::openFile(std::vector<std::vector<Character>> structure) {

}