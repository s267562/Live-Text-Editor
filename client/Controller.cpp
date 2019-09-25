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
    connect(this->messanger, SIGNAL(reciveUser(User*)), this, SLOT(reciveUser(User*)));
    connect(this->messanger, SIGNAL(editAccountFailed()), this, SLOT(errorEditAccount()));
    connect(this->messanger, SIGNAL(okEditAccount()), this, SLOT(okEditAccount()));
    connect(this->messanger, SIGNAL(shareCodeFailed()), this, SLOT(shareCodeFailed()));
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
    connect(this->messanger, SIGNAL(reciveUser(User*)), this, SLOT(reciveUser(User*)));
}

/* USER */

void Controller::reciveUser(User *user){
    this->user = user;
}

/* NETWORKING */

void Controller::errorConnection(){
    QMessageBox::information(now, "Connection", "Try again, connection not established!");
    // TODO: retry...
}

/* CONNECTION */

void Controller::connectClient(QString address, QString port) {
    bool res = this->messanger->connectTo(address, port);    // TODO: non va...

    if (res) {
        this->connection->close();

        /* creation login object */
        login = new Login(this, this);
        now = login;
        login->setClient(messanger);
        connect(this->messanger, &Messanger::loginFailed, this->login, &Login::loginFailed);
        connect(this->messanger, &Messanger::logout, this, &Controller::showLogin);
        connect(this->login, SIGNAL(showRegistration()), this, SLOT(showRegistration()));
        //connect(this->login, SIGNAL(loginSuccessful()), this, SLOT(showFileFinder()));

        /* creation registration object */
        registration = new Registration(this, this);
        registration->setClient(messanger);
        connect(this->messanger, &Messanger::registrationFailed, this->registration, &Registration::registrationFailed);
        connect(this->registration, SIGNAL(showLogin()), this, SLOT(showLogin()));

        /* creation showfiles object */
        finder = new ShowFiles(this, this);
        connect(this->finder, &ShowFiles::logout, this->messanger, &Messanger::logOut);
        connect(this->messanger, &Messanger::requestForFileFailed, this->finder, &ShowFiles::showError);
        connect(this->messanger, SIGNAL(fileNames(std::map<QString, bool>)), this, SLOT(showFileFinder(std::map<QString, bool>)));
        connect(this->messanger, SIGNAL(addFileNames(std::map<QString, bool>)), this, SLOT(addFileNames(std::map<QString, bool>)));
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
    now = registration;
    this->registration->show();
}

/* SHOW FILE */

void Controller::showFileFinder(std::map<QString, bool> fileList){
    loading->close();
    now->close();
    this->finder->addFiles(fileList);

    if (user != nullptr){
        user->setFileList(fileList);
        user->setIsLogged(true);
        now = finder;
        this->finder->show();
    }


}

void Controller::showFileFinderOtherView(){
    now->close();
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
            connect(messanger, &Messanger::insertFailed, editor, &Editor::showError);
            connect(messanger, &Messanger::deleteFailed, editor, &Editor::showError);
            connect(messanger, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));
            connect(this->finder, &ShowFiles::logout, this->messanger, &Messanger::logOut);
            connect(this->editor, &Editor::showFinder, this, &Controller::showFileFinderOtherView);
            connect(editor, &Editor::logout, messanger, &Messanger::logOut);
        }else{
            editor->reset();
        }
        editor->setFilename(&filename);
        now->close();
        now = editor;
        editor->show();
        startLoadingPopup();
    }
}

void Controller::showEditor(){
    editor->reset();
    editor->show();
}

void Controller::localInsert(QString val, QTextCharFormat textCharFormat, Pos pos) {
    // insert into the model
    Character character = this->crdt->handleLocalInsert(val.at(0).toLatin1(), textCharFormat, pos);

    // send insert at the server.
    this->messanger->writeInsert(character);
}

void Controller::styleChange(QTextCharFormat textCharFormat, Pos pos) {
    // check if style change
    if(crdt->styleChanged(textCharFormat, pos)) {
        Character character = crdt->getCharacter(pos);

        // send insert at the server.
        this->messanger->writeStyleChanged(character);
    } else {
        // do nothing...
    }
}


void Controller::localDelete(Pos startPos, Pos endPos) {
    std::vector<Character> removedChars = this->crdt->handleLocalDelete(startPos, endPos);

    for(Character c : removedChars) {
        this->messanger->writeDelete(c);
    }
}

void Controller::newMessage(Message message) {
    // Message message = this->messanger->getMessage();

    if(message.getType() == INSERT) {
        Character character = message.getCharacter();

        Pos pos = this->crdt->handleRemoteInsert(character);

        if(character.getSiteId() == this->crdt->getSiteId()) {
            // local insert - only in the model; the char is already in the view.
        } else {
            // remote insert - the char is to insert in the model and in the view. Insert into the editor.
            this->editor->insertChar(character.getValue(), character.getTextCharFormat(), pos);
        }
    } else if(message.getType() == STYLE_CHANGED) {
        Pos pos = this->crdt->handleRemoteStyleChanged(message.getCharacter());

        if(pos) {
            // delete from the editor.
            this->editor->changeStyle(pos, message.getCharacter().getTextCharFormat());
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
    user = messanger->user;
    return user;
}

void Controller::sendEditAccount(QString username, QString newPassword, QString oldPassword, QByteArray avatar){
    messanger->sendEditAccount(username, newPassword, oldPassword, avatar);
    /*loading = new Loading(now);
    loading->show();*/
}

void Controller::errorEditAccount() {
    QMessageBox::warning(now, "Edit account", "Try again, Edit account!");
    //loading->close();
}

void Controller::okEditAccount(){
    //loading->close();
}

void Controller::startLoadingPopup(){
    loading = new Loading(now);
    loading->show();
}

void Controller::stopLoadingPopup(){
    if (loading != nullptr){
        loading->close();
    }
}

void Controller::sendShareCode(QString sharecode){
    messanger->sendShareCode(sharecode);
    startLoadingPopup();
}

void Controller::shareCodeFailed(){
    QMessageBox::warning(now, "Share Code", "Share code is wrong! Try again!");
    loading->close();
}

void Controller::addFileNames(std::map<QString, bool> filenames){
    finder->addFile(filenames);
    loading->close();
}