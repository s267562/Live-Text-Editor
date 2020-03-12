//
// Created by simone on 09/08/19.
//

#include "Controller.h"
#include <QMessageBox>

Controller::Controller(): messanger(new Messanger(this)), connection(new Connection(this)){
    user = nullptr;
    editor = nullptr;
    crdt = new CRDT(nullptr, messanger);
    crdtThread = new CDRTThread(this, crdt);
    crdt->moveToThread(crdtThread);
    crdtThread->start();

    /* creation connection and messanger object */
    connect(this->messanger, &Messanger::errorConnection, this, &Controller::errorConnection);
    connect(messanger, SIGNAL(fileRecive(std::vector<std::vector<Character>>, std::vector<std::pair<Character,int>>, QString)), this, SLOT(openFile(std::vector<std::vector<Character>>,std::vector<std::pair<Character,int>>, QString)));
    connect(this->connection, SIGNAL(connectToAddress(QString, QString)),this, SLOT(connectClient(QString, QString)));

    /*connect(messanger, &Messanger::newMessage,
            this, &Controller::newMessage);*/
    connect(this->messanger, SIGNAL(reciveUser(User*)), this, SLOT(reciveUser(User*)));
    connect(this->messanger, SIGNAL(editAccountFailed()), this, SLOT(errorEditAccount()));
    connect(this->messanger, SIGNAL(okEditAccount()), this, SLOT(okEditAccount()));
    connect(this->messanger, SIGNAL(shareCodeFailed()), this, SLOT(shareCodeFailed()));
    connect(this->messanger,SIGNAL(reciveUsernameList(QString, QStringList)), this, SLOT(reciveUsernameList(QString, QStringList)));

    /* multi threading */
    messanger->setCrdt(crdt);
    connect(crdt, SIGNAL(writeInsert(Character)), this->messanger, SLOT(writeInsert(Character)));
    connect(crdt, SIGNAL(writeDelete(Character)), this->messanger, SLOT(writeDelete(Character)));
    connect(crdt, SIGNAL(writeStyleChanged(Character)), this->messanger, SLOT(writeStyleChanged(Character)));
    connect(messanger, &Messanger::newMessage,crdt, &CRDT::newMessage);

    now = connection;
    connection->show();
}

Controller::Controller(CRDT *crdt, Editor *editor, Messanger *messanger) : crdt(crdt), editor(editor), messanger(messanger) {
    editor->setController(this);
    user = nullptr;

    // Controller
    /*connect(messanger, &Messanger::newMessage,
              this, &Controller::newMessage);*/

    connect(editor, &Editor::logout, messanger, &Messanger::logOut);
    connect(messanger, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
    connect(messanger, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));
    connect(messanger, SIGNAL(fileRecive(std::vector<std::vector<Character>>, std::vector<std::pair<Character,int>>)), this, SLOT(openFile(std::vector<std::vector<Character>>,std::vector<std::pair<Character,int>>)));
    connect(this->messanger, SIGNAL(reciveUser(User*)), this, SLOT(reciveUser(User*)));
}

/* USER */

void Controller::reciveUser(User *user){
    if (this->user == nullptr || this->user->isIsLogged() != true){
        this->user = user;
        this->crdt->setSiteId(user->getUsername());
        this->user->setIsLogged(true);
    }else{
        this->user->setUsername(user->getUsername());
        this->user->setAvatar(user->getAvatar());
        stopLoadingPopup();
        this->finder->closeEditAccount();
        if (this->editor != nullptr)
            this->editor->closeEditAccount();
    }
    emit userRecived();
}

User* Controller::getUser(){
    return this->user;
}

/* NETWORKING */

void Controller::errorConnection(){
    QMessageBox::information(now, "Connection", "Try again, connection not established!");
    now->close();
    now = connection;
    connection->show();
}

/* CONNECTION */

void Controller::connectClient(QString address, QString port) {
    bool res = this->messanger->connectTo(address, port);

    if (res) {
        now->close();

        /* creation login object */
        login = new Login(this, this);
        now = login;
        login->setClient(messanger);
        connect(this->messanger, &Messanger::loginFailed, this->login, &Login::loginFailed);
        connect(this->messanger, &Messanger::logout, this, &Controller::showLogin);
        connect(this->login, SIGNAL(showRegistration()), this, SLOT(showRegistration()));

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


    if (now == editor){
        /*disconnect(messanger, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
        disconnect(messanger, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));
        disconnect(this, SIGNAL(userRecived()), this->editor, SLOT(changeUser()));*/
    }else{
        now->close();
        now = finder;
        this->finder->addFiles(fileList);
        this->finder->show();
    }

    if (user != nullptr){
        user->setFileList(fileList);
    }
}

void Controller::showFileFinderOtherView(){
    now->close();
    now = finder;
    disconnect(messanger, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
    disconnect(messanger, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));
    disconnect(this, SIGNAL(userRecived()), this->editor, SLOT(changeUser()));
    connect(this, SIGNAL(userRecived()), this->finder, SLOT(changeImage()));
    this->finder->addFiles(user->getFileList());
    this->finder->show();
}

/* EDITOR */

void Controller::requestForFile(QString filename){
    bool result = this->messanger->requestForFile(filename);
    qDebug() << "Controller1: "<< QThread::currentThreadId();
    if (result){
        if (editor == nullptr){
            siteId = user->getUsername();
            editor = new Editor(siteId, nullptr, this);

            /* connecting */
            connect(this->editor, &Editor::showFinder, this, &Controller::showFileFinderOtherView);
            connect(messanger, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
            connect(messanger, &Messanger::insertFailed, editor, &Editor::showError);
            connect(messanger, &Messanger::deleteFailed, editor, &Editor::showError);
            connect(messanger, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));
            connect(this->finder, &ShowFiles::logout, this->messanger, &Messanger::logOut);
            connect(this->editor, &Editor::showFinder, this, &Controller::showFileFinderOtherView);
            connect(editor, &Editor::logout, messanger, &Messanger::logOut);
            /* MULTI THREAD */
            crdt->setEditor(editor);
            connect(crdt, SIGNAL(insertChar(char, QTextCharFormat, Pos, QString)), editor, SLOT(insertChar(char, QTextCharFormat, Pos, QString)));
            connect(crdt, SIGNAL(changeStyle(Pos, const QTextCharFormat&, QString)), editor, SLOT(changeStyle(Pos , const QTextCharFormat&, QString)));
            connect(crdt, SIGNAL(deleteChar(Pos, QString)), editor, SLOT(deleteChar(Pos, QString)));
            connect(editor, SIGNAL(localDelete(Pos , Pos )), crdt, SLOT(localDelete(Pos , Pos )));
            connect(editor, SIGNAL(totalLocalInsert(int , QTextCursor* , QString, int )), crdt, SLOT(totalLocalInsert(int , QTextCursor* , QString, int )), Qt::QueuedConnection);
            connect(editor, SIGNAL(totalLocalStyleChange(int , QTextCursor, int)), crdt, SLOT(totalLocalStyleChange(int, QTextCursor, int)), Qt::QueuedConnection);

        }else{
            connect(messanger, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
            connect(messanger, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));
            connect(this, SIGNAL(userRecived()), this->editor, SLOT(changeUser()));
            editor->reset();
        }
        disconnect(this, SIGNAL(userRecived()), this->finder, SLOT(changeImage()));
        editor->setFilename(filename);
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

/*void Controller::localInsert(QString val, QTextCharFormat textCharFormat, Pos pos) {
    // insert into the model
    Character character = this->crdt->handleLocalInsert(val.at(0).toLatin1(), textCharFormat, pos);
    
    QTextCharFormat tcf=character.getTextCharFormat();

    // send insert at the server.
    this->messanger->writeInsert(character);
}

void Controller::styleChange(QTextCharFormat textCharFormat, Pos pos) {
    // check if style change
    if(crdt->styleChanged(textCharFormat, pos)) {
        Character character = crdt->getCharacter(pos);

        // send insert at the server.
        this->messanger->writeStyleChanged(character);
    }
    else {
        // do nothing...
    }

}




void Controller::localDelete(Pos startPos, Pos endPos) {
    std::vector<Character> removedChars = this->crdt->handleLocalDelete(startPos, endPos);

    for(Character c : removedChars) {
        this->messanger->writeDelete(c);
    }
}*/

/*void Controller::newMessage(Message message) {

    qDebug() << "\nController.cpp - newMessage()";


    if(message.getType() == INSERT) {
        Character character = message.getCharacter();
        Pos pos = this->crdt->handleRemoteInsert(character);

        if(character.getSiteId() == this->crdt->getSiteId()) {
            // local insert - only in the model; the char is already in the view.
        } else {
            // remote insert - the char is to insert in the model and in the view. Insert into the editor.
            qDebug() << message.getSender();
            this->editor->insertChar(character.getValue(), character.getTextCharFormat(), pos, message.getSender());

        }
    } else if(message.getType() == STYLE_CHANGED) {
        Character character=message.getCharacter();
        Pos pos = this->crdt->handleRemoteStyleChanged(character);

        if(pos) {
            // delete from the editor.
            QTextCharFormat tcf =  message.getCharacter().getTextCharFormat();

            this->editor->changeStyle(pos, tcf, message.getSender());

        }
    } else if(message.getType() == ALIGNMENT_CHANGED) {
        int row = this->crdt->getRow(message.getCharacter());
        if(row>=0){
            this->editor->remoteAlignmentChanged(message.getAlignmentType(), row);
        }
        
    }
    else if(message.getType() == DELETE) {

        QDebug qD(QtDebugMsg);

        QString sender = message.getSender();
        Character character = message.getCharacter();

        //TODO: Fix print

        qD << "\n\t\tMESSAGE:\tDELETE";
        qD << "\n\t\tSENDER:\t" << sender;
        qD << "\n\t\tCHAR TO REMOVE:\t\tValue:\t" << character.getValue()+"\tCounter:\t" << character.getCounter() << "\tsiteId:\t" + character.getSiteId() + "\tPosition:\t";

        for (Identifier id : character.getPosition()) {
            qD << id.getDigit();
        }


        Pos pos = this->crdt->handleRemoteDelete(character);
        if(pos) {
            // delete from the editor.
            QChar removedChar=this->editor->deleteChar(pos,sender);

            qD << "\n\t\tChar removed correctly";
            //TODO: If an error occurs?

        }else{ // Skip in case char doesn't exist
            qD << "\n\t\tThe char has already been removed";
        }

    }
}*/

void Controller::alignChange(int alignment_type, int blockNumber) { // -> da gestire forse nel crdt

    // send insert at the server.
    //TODO Check this
    Character blockId=this->crdt->getBlockIdentifier(blockNumber); // Retrieve the char used as unique identifier of row (block)
    //if(blockId.getSiteId()!=-1){
    this->messanger->writeAlignmentChanged(alignment_type,blockId);
    //}
}

void Controller::openFile(std::vector<std::vector<Character>> initialStructure, std::vector<std::pair<Character,int>> styleBlocks, QString filename) {
    // introdurre sincronizzazione
    std::unique_lock<std::shared_mutex> uniqueLock(crdt->mutexCRDT);
    crdt->setStructure(initialStructure);
    crdt->setStyle(styleBlocks);
    editor->replaceText(this->crdt->getStructure());
    std::vector<int> alignment_block;
    for(std::pair<Character,int> & block : styleBlocks){
        alignment_block.emplace_back(block.second);
    }
    this->editor->formatText(alignment_block);
    /* aggiunta del file name nella lista presente nell' oggetto user se non è presente */
    auto result = this->user->getFileList().find(filename);

    if (result != this->user->getFileList().end()){
        this->user->addFile(filename);
        this->finder->addFiles(this->user->getFileList());
    }
}

void Controller::sendEditAccount(QString username, QString newPassword, QString oldPassword, QByteArray avatar){
    messanger->sendEditAccount(username, newPassword, oldPassword, avatar);
    startLoadingPopup();
}

void Controller::errorEditAccount() {
    QMessageBox::warning(now, "Edit account", "Try again, Edit account!");
    stopLoadingPopup();
}

void Controller::okEditAccount(){
    stopLoadingPopup();
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
    stopLoadingPopup();
}

void Controller::addFileNames(std::map<QString, bool> filenames){
    finder->addFile(filenames);
    finder->closeAddFile();
    stopLoadingPopup();
}

void Controller::requestForUsernameList(QString filename, CustomWidget *customWideget){
    // Effettua la rischiesta al server
    this->customWidget = customWideget;
    // Effettua la rischiesta al server
    this->messanger->requestForUsernameList(filename);
    startLoadingPopup();
}

void Controller::reciveUsernameList(QString filename, QStringList userlist){
    stopLoadingPopup();
    this->customWidget->createFileInformation(userlist);
}

void Controller::sendFileInformationChanges(QString oldFileaname, QString newFileaname, QStringList usernames){
    this->messanger->sendFileInfomationChanges(oldFileaname, newFileaname, usernames);
    startLoadingPopup();
}

void Controller::sendDeleteFile(QString filename){
    this->messanger->sendDeleteFile(filename);
    startLoadingPopup();
}

/*void Controller::totalLocalInsert(int charsAdded, QTextCursor cursor, QString chars, int position) {
    qDebug() << "Controller: " << QThread::currentThreadId();
    for(int i=0; i<charsAdded; i++) {
        // for each char added
        cursor.setPosition(position + i);
        int line = cursor.blockNumber();
        int ch = cursor.positionInBlock();
        Pos startPos{ch, line}; // Pos(int ch, int line, const std::string);
        // select char
        cursor.setPosition(position + i + 1, QTextCursor::KeepAnchor);
        QTextCharFormat charFormat = cursor.charFormat();

        localInsert(chars.at(i), charFormat, startPos);
    }
}*/

Controller::~Controller(){
    this->crdtThread->quit();
    this->crdtThread->wait();
}

CRDT *Controller::getCrdt() const {
    return crdt;
}
