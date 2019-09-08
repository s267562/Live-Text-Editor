//
// Created by simone on 09/08/19.
//

#include "Controller.h"
#include <QMessageBox>


Controller::Controller(): messanger(new Messanger(this)), connection(new Connection(this)){
    user = nullptr;
    editor = nullptr;
    crdt = new CRDT();
    messanger->setCRDT(crdt);

    /* creation connection and messanger object */
    connect(this->messanger, &Messanger::errorConnection, this, &Controller::errorConnection);
    connect(messanger, SIGNAL(fileRecive(std::vector<std::vector<Character>>)), this, SLOT(openFile(std::vector<std::vector<Character>>)));
    connect(this->connection, SIGNAL(connectToAddress(QString, QString)),this, SLOT(connectClient(QString, QString)));
    connect(messanger, &Messanger::newMessage,
            this, &Controller::newMessage);
    now = connection;
    connection->show();
}

Controller::Controller(CRDT *crdt, Editor *editor, Messanger *messanger) : crdt(crdt), editor(editor), messanger(messanger) {
    editor->setController(this);
    user = nullptr;

    // Controller
    connect(messanger, &Messanger::newMessage,
              this, &Controller::newMessage);

    connect(editor, &Editor::logout, messanger, &Messanger::logOut);
    connect(messanger, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
    connect(messanger, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));
    connect(messanger, SIGNAL(fileRecive(std::vector<std::vector<Character>>)), this, SLOT(openFile(std::vector<std::vector<Character>>)));
}

/* NETWORKING */

void Controller::errorConnection(){
    QMessageBox::information(this, "Connection", "Try again, connection not established!");
    // TODO: retry...
}

/* CONNECTION */

void Controller::connectClient(QString address, QString port) {
    bool res = this->messanger->connectTo(address, port);    // TODO: non va...

    if (res) {
        this->connection->close();

        /* creation login object */
        login = new Login(this);
        now = login;
        login->setClient(messanger);
        connect(this->messanger, &Messanger::loginFailed, this->login, &Login::loginFailed);
        connect(this->messanger, &Messanger::logout, this, &Controller::showLogin);
        connect(this->login, SIGNAL(showRegistration()), this, SLOT(showRegistration()));
        //connect(this->login, SIGNAL(loginSuccessful()), this, SLOT(showFileFinder()));

        /* creation registration object */
        registration = new Registration(this);
        connect(this->registration, SIGNAL(showLogin()), this, SLOT(showLogin()));

        /* creation showfiles object */
        finder = new ShowFiles(this);
        connect(this->finder, &ShowFiles::logout, this->messanger, &Messanger::logOut);
        connect(this->messanger, SIGNAL(fileNames(QStringList)), this, SLOT(showFileFinder(QStringList)));
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
    /*connect(this->finder, &ShowFiles::logout, this->messanger, &Messanger::logOut);
    connect(this->editor, &Editor::showFinder, this, &Controller::showFileFinderOtherView);*/
    /*connect(messanger, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
    connect(messanger, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));*/
    now = finder;
    this->finder->show();
}

/* EDITOR */

void Controller::requestForFile(QString filename){
    bool result = this->messanger->requestForFile(filename);

    if (result){
        if (editor == nullptr){
            siteId = user->getUsername();
            editor = new Editor(siteId, this, this);

            /* connecting */
            connect(this->editor, &Editor::showFinder, this, &Controller::showFileFinderOtherView);
            connect(messanger, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
            connect(messanger, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));
            connect(this->finder, &ShowFiles::logout, this->messanger, &Messanger::logOut);
            connect(this->editor, &Editor::showFinder, this, &Controller::showFileFinderOtherView);
            connect(editor, &Editor::logout, messanger, &Messanger::logOut);
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

void Controller::localInsert(QString chars, CharFormat charFormat, Pos startPos) {
    // send insert at the server. To insert it in the model we need the position computed by the server.
    InsertCharacter character((char)chars[0].toLatin1(), crdt->getSiteId(), charFormat, startPos);
    this->messanger->insert(character);
}

void Controller::localDelete(Pos startPos, Pos endPos) {
    std::vector<Character> removedChars = this->crdt->handleDelete(startPos, endPos);

    for(Character c : removedChars) {
        this->messanger->deleteChar(c);
    }
}

void Controller::newMessage(Message message) {
    // Message message = this->messanger->getMessage();

    if(message.getType() == INSERT) {
        Character character = message.getCharacter();

        Pos pos = this->crdt->insert(character);

        if(character.getSiteId() == this->crdt->getSiteId()) {
            // local insert - only in the model; the char is already in the view.
        } else {
            // remote insert - the char is to insert in the model and in the view. Insert into the editor.
            this->editor->insertChar(character.getValue(), character.getCharFormat(), pos);
        }
    } else if(message.getType() == DELETE) {
        Pos pos = this->crdt->handleRemoteDelete(message.getCharacter());

        if(pos) {
            // delete from the editor.
            this->editor->deleteChar(pos);
        }
    }
}

void Controller::openFile(std::vector<std::vector<Character>> initialStructure) {
    crdt->setStructure(initialStructure);
    this->editor->replaceText(this->crdt->toText());
}

User* Controller::getUser(){
    if (user == nullptr){
        user = new User(login->getUsername());
        user->setIsLogged(true);
    }
    return user;
}