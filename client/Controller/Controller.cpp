//
// Created by andrea settimo on 2020-03-18.
//

#include "Controller.h"
#include <QMessageBox>
#include <QDesktopWidget>
#include <utility>

Controller::Controller(): messanger(new Messanger(this, this)), connection(new Connection(this)){
    user = nullptr;
    editor = nullptr;
    crdt = new CRDT(nullptr, messanger, this);
    crdtThread = new CDRTThread(this, crdt);
    crdt->moveToThread(crdtThread);
    crdtThread->start();

    /* creation connection and messanger object */
    connect(this->connection, SIGNAL(connectToAddress(QString, QString)),this, SLOT(connectClient(QString, QString)));

    /* Networking */
    networkingConnection();

    /* CRDT */
    crdtConnection();

    GUI->setGeometry((QApplication::desktop()->width() - connection->width())/2, (QApplication::desktop()->height() - connection->height())/2, connection->width(), connection->height());
    handleGUI(connection);
}

Controller::Controller(CRDT *crdt, Editor *editor, Messanger *messanger) : crdt(crdt), editor(editor), messanger(messanger) {
    editor->setController(this);
    user = nullptr;

    connect(editor, &Editor::logout, messanger, &Messanger::logOut);
    connect(messanger, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
    connect(messanger, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));
    connect(messanger, SIGNAL(fileRecive(std::vector<std::vector<Character>>, std::vector<std::pair<Character,int>>)), this, SLOT(openFile(std::vector<std::vector<Character>>,std::vector<std::pair<Character,int>>)));
    connect(this->messanger, SIGNAL(reciveUser(User*)), this, SLOT(reciveUser(User*)));
}

/* ------------------------------------------------------------- SINGAL AND SLOT CONNECTION */

/**
 * Handle the connection between signal and slot for the messanger object
 */
void Controller::networkingConnection() {
    connect(this->messanger, &Messanger::errorConnection, this, &Controller::errorConnection);
    connect(this->messanger, SIGNAL(fileRecive(std::vector<std::vector<Character>>, std::vector<std::pair<Character,int>>, QString)),
            this, SLOT(openFile(std::vector<std::vector<Character>>,std::vector<std::pair<Character,int>>, QString)));
    connect(this->messanger, SIGNAL(reciveUser(User*)), this, SLOT(reciveUser(User*)));
    connect(this->messanger, SIGNAL(editAccountFailed()), this, SLOT(errorEditAccount()));
    connect(this->messanger, SIGNAL(okEditAccount()), this, SLOT(okEditAccount()));
    connect(this->messanger, SIGNAL(shareCodeFailed()), this, SLOT(shareCodeFailed()));
    connect(this->messanger,SIGNAL(reciveUsernameList(QString, QStringList)), this, SLOT(reciveUsernameList(QString, QStringList)));
    connect(this->messanger, SIGNAL(fileNames(std::map<QString, bool>)), this, SLOT(showFileFinder(std::map<QString, bool>)));
    connect(this->messanger, SIGNAL(addFileNames(std::map<QString, bool>)), this, SLOT(addFileNames(std::map<QString, bool>)));
}

/**
 *
 * Handle the connection between signal and slot for the CRDT object
 */
void Controller::crdtConnection() {
    messanger->setCrdt(crdt);
    connect(crdt, SIGNAL(writeInsert(Character)), this->messanger, SLOT(writeInsert(Character)));
    connect(crdt, SIGNAL(writeDelete(Character)), this->messanger, SLOT(writeDelete(Character)));
    connect(crdt, SIGNAL(writeStyleChanged(Character)), this->messanger, SLOT(writeStyleChanged(Character)));
    connect(messanger, &Messanger::newMessage,crdt, &CRDT::newMessage);
}

/**
 *  Handle the connection between signal and slot for the login object
 */
void Controller::loginConnection() {
    login->setClient(messanger);
    connect(this->messanger, &Messanger::loginFailed, this->login, &Login::loginFailed);
    connect(this->messanger, &Messanger::logout, this, &Controller::showLogin);
    connect(this->login, SIGNAL(showRegistration()), this, SLOT(showRegistration()));
}

/**
 *  Handle the disconnection between signal and slot for the login object
 */
void Controller::loginDisconnection() {
    disconnect(this->messanger, &Messanger::loginFailed, this->login, &Login::loginFailed);
    disconnect(this->messanger, &Messanger::logout, this, &Controller::showLogin);
    disconnect(this->login, SIGNAL(showRegistration()), this, SLOT(showRegistration()));
}

/**
 * Handle the connection between signal and slot for the registration object
 */
void Controller::registrationConnection() {
    registration->setClient(messanger);
    connect(this->messanger, &Messanger::registrationFailed, this->registration, &Registration::registrationFailed);
    connect(this->registration, SIGNAL(showLogin()), this, SLOT(showLogin()));
}

/**
 *  Handle the disconnection between signal and slot for the registration object
 */
void Controller::registrationDisconnection() {
    disconnect(this->messanger, &Messanger::registrationFailed, this->registration, &Registration::registrationFailed);
    disconnect(this->registration, SIGNAL(showLogin()), this, SLOT(showLogin()));
}

/**
 *  Handle the connection between signal and slot for the showFiles object
 */
void Controller::finderConnection() {
    connect(this->finder, &ShowFiles::logout, this->messanger, &Messanger::logOut);
    connect(this->messanger, &Messanger::requestForFileFailed, this->finder, &ShowFiles::showError);
    connect(this->finder, SIGNAL(newFile(QString)), this, SLOT(requestForFile(QString)));
}

/**
 *  Handle the disconnection between signal and slot for the showFiles object
 */
void Controller::finderDisconnection() {
    disconnect(this->finder, &ShowFiles::logout, this->messanger, &Messanger::logOut);
    disconnect(this->messanger, &Messanger::requestForFileFailed, this->finder, &ShowFiles::showError);
    disconnect(this->finder, SIGNAL(newFile(QString)), this, SLOT(requestForFile(QString)));
}

/**
 * Handle the disconnection between signal and slot for the editor object
 */
void Controller::editorConnection() {
    /* connecting */
    connect(this->editor, &Editor::showFinder, this, &Controller::showFileFinderOtherView);
    connect(messanger, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
    connect(messanger, &Messanger::insertFailed, editor, &Editor::showError);
    connect(messanger, &Messanger::deleteFailed, editor, &Editor::showError);
    connect(messanger, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));
    connect(editor, &Editor::logout, messanger, &Messanger::logOut);
    if (finder == now) {
        connect(this->finder, &ShowFiles::logout, this->messanger, &Messanger::logOut);
        connect(this->editor, &Editor::showFinder, this, &Controller::showFileFinderOtherView);
    }
    /* MULTI THREAD */
    crdt->setEditor(editor);
    //connect(crdt, SIGNAL(insertChar(char, QTextCharFormat, Pos, QString)), editor, SLOT(insertChar(char, QTextCharFormat, Pos, QString)));
    connect(crdt, SIGNAL(changeStyle(Pos, const QTextCharFormat&, QString)), editor, SLOT(changeStyle(Pos , const QTextCharFormat&, QString)));
    connect(crdt, SIGNAL(deleteChar(Pos, QString)), editor, SLOT(deleteChar(Pos, QString)));
    connect(editor, SIGNAL(localDelete(Pos , Pos )), crdt, SLOT(localDelete(Pos , Pos )));
    connect(editor, SIGNAL(totalLocalInsert(int , QTextCursor* , QString, int )), crdt, SLOT(totalLocalInsert(int , QTextCursor* , QString, int )), Qt::QueuedConnection);
    //connect(editor, SIGNAL(totalLocalStyleChange(int , QTextCursor*, int, int, int)), crdt, SLOT(totalLocalStyleChange(int, QTextCursor, int, int, int)), Qt::QueuedConnection);
}

/*  ------------------------------------------------------------- USER */

/**
 * This method recives the user object and handling it in specific way
 * @param user
 */
void Controller::reciveUser(User *user){
    if (this->user == nullptr || !this->user->isIsLogged()){
        this->user = user;
        this->crdt->setSiteId(user->getUsername());
        this->user->setIsLogged(true);
    }else{
        this->user->setUsername(user->getUsername());
        this->user->setAvatar(user->getAvatar());
        this->crdt->setSiteId(user->getUsername());
        stopLoadingPopup();
        if (now == finder)
            this->finder->closeEditAccount();
        //if (this->editor != nullptr && now == editor)
            //this->editor->closeEditAccount();
    }
    emit userRecived();
}

User* Controller::getUser(){
    return this->user;
}

/*  -------------------------------------------------------------  NETWORKING */

/**
 * This method handles the disconnection with the server
 */
void Controller::errorConnection(){
    QMessageBox::information(now, "Connection", "Try again, connection not established!");
    connection = new Connection(this);
    connect(this->connection, SIGNAL(connectToAddress(QString, QString)),this, SLOT(connectClient(QString, QString)));
    now = connection;
    handleGUI(connection);
    stopLoadingPopup();
}

/*  -------------------------------------------------------------  CONNECTION */

/**
 * This method handles connection with server
 * @param address
 * @param port
 */
void Controller::connectClient(QString address, QString port) {
    bool res = this->messanger->connectTo(std::move(address), port);

    if (res) {
        /* creation login object */
        showLogin();
    }
}

/* LOGIN */

/**
 * This method handles the showing part of login
 */
void Controller::showLogin(){
    /* creation login object */
    if (login != nullptr){
        loginDisconnection();
    }
    login = new Login(this, this);
    now = login;

    loginConnection();

    /* creation registration object */
    if (registration != nullptr){
        registrationDisconnection();
    }
    registration = new Registration(this, this);
    registrationConnection();

    /* creation showfiles object */
    if (finder != nullptr) {
        finderDisconnection();
    }
    finder = new ShowFiles(this, this);
    finderConnection();
    handleGUI(login);
}

/* ------------------------------------------------------------- REGISTRATION */

/**
 * This method handles the showing part of registation
 */
void Controller::showRegistration(){
    this->registration->reset();
    now = registration;
    handleGUI(registration);
}

/* SHOW FILE */

/**
 * This method recives the list of files
 * @param fileList
 */
void Controller::showFileFinder(const std::map<QString, bool>& fileList){
    stopLoadingPopup();

    if (now != editor){
        now = finder;
        this->finder->addFiles(fileList);
        handleGUI(finder);
    }

    if (user != nullptr){
        user->setFileList(fileList);
    }
}

/**
 * This method shows the showFiles for request another view
 */
void Controller::showFileFinderOtherView(){
    bool result = this->messanger->requestForFile("**FILE_FITTIZIO**");

    if (editor != nullptr) {
        disconnect(messanger, SIGNAL(setUsers(QStringList)), editor, SLOT(setUsers(QStringList)));
        disconnect(messanger, SIGNAL(removeUser(QString)), editor, SLOT(removeUser(QString)));
        disconnect(this, SIGNAL(userRecived()), this->editor, SLOT(changeUser()));
    }
    now = finder;
    if (finder != nullptr)
        finderDisconnection();
    finder = new ShowFiles(this, this);
    finderConnection();

    this->finder->addFiles(user->getFileList());
    handleGUI(finder);
}

/* ------------------------------------------------------------- EDITOR */

/**
 * This method handles the request for a specific file
 * @param filename
 */
void Controller::requestForFile(const QString& filename){
    std::unique_lock<std::shared_mutex> requestLock(mutexRequestForFile);
    requestFFile = true;
    qDebug() << "requestForFile" << filename;
    bool result = this->messanger->requestForFile(filename);

    if (result) {
        siteId = user->getUsername();
        editor = new Editor(siteId, this, this);
        editor->setFilename(filename);
        editorConnection();

        disconnect(this, SIGNAL(userRecived()), this->finder, SLOT(changeImage())); // da vedere
        now = editor;
        handleGUI(editor);
        startLoadingPopup();
    }
}

void Controller::showEditor(){
    editor->reset();
    editor->show();
}

/**
 * This method recives a file and handle it a specific way
 * @param initialStructure
 * @param styleBlocks
 * @param filename
 */
void Controller::openFile(const std::vector<std::vector<Character>>& initialStructure, std::vector<std::pair<Character,int>> styleBlocks, QString filename) {
    std::unique_lock<std::shared_mutex> requestLock(mutexRequestForFile);
    requestFFile = false;
    crdt->setStructure(initialStructure);           // fare un segnale
    crdt->setStyle(styleBlocks);                    // fare un segnale

    editor->replaceText(this->crdt->getStructure());
    std::vector<int> alignment_block;
    alignment_block.reserve(styleBlocks.size());
    
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

    finder->closeCreateFile();
}

/**
 * This method handles the account changes
 * @param username
 * @param newPassword
 * @param oldPassword
 * @param avatar
 */
void Controller::sendEditAccount(QString username, QString newPassword, QString oldPassword, const QByteArray& avatar){
    messanger->sendEditAccount(std::move(username), std::move(newPassword), std::move(oldPassword), avatar);
    startLoadingPopup();
}

/**
 * This method, in case of error, shows an error message
 */
void Controller::errorEditAccount() {
    QMessageBox::warning(GUI, "Edit account", "Try again, Edit account!");
    stopLoadingPopup();
}

/**
 * This method calls the stopLoadingPopup for to stop the loading pop up
 */
void Controller::okEditAccount(){
    stopLoadingPopup();
}

/**
 * This mathod starts the loading pop up
 */
void Controller::startLoadingPopup(){
    loading = new Loading(GUI);
    loadingPoPupIsenabled = true;
    loading->show();
}

/**
 * This method stops the loading pop up
 */
void Controller::stopLoadingPopup(){
    if (loadingPoPupIsenabled && loading != nullptr){
        loadingPoPupIsenabled = false;
        loading->setParent(GUI);
        loading->close();
    }
}

/**
 * This method sends the share code
 * @param sharecode
 */
void Controller::sendShareCode(const QString& sharecode){
    messanger->sendShareCode(sharecode);
    startLoadingPopup();
}

/**
 * this method shows an error message
 */
void Controller::shareCodeFailed(){
    QMessageBox::warning(GUI, "Share Code", "Share code is wrong! Try again!");
    stopLoadingPopup();
}

/**
 * This method is called in the case of file sharing was successful
 * @param filenames
 */
void Controller::addFileNames(std::map<QString, bool> filenames){
    finder->addFile(std::move(filenames));
    finder->closeAddFile();
    stopLoadingPopup();
}

/**
 * This method does a request for a username list for a specific file
 * @param filename
 * @param customWideget
 */
void Controller::requestForUsernameList(QString filename, CustomWidget *customWideget){
    this->customWidget = customWideget;
    // Effettua la rischiesta al server
    this->messanger->requestForUsernameList(std::move(filename));
    startLoadingPopup();
}

/**
 * This method recives the username list for a specific file
 * @param filename
 * @param userlist
 */
void Controller::reciveUsernameList(const QString& filename, QStringList userlist){
    stopLoadingPopup();
    this->customWidget->createFileInformation(std::move(userlist));
}

/**
 * This method handles the file information changes
 * @param oldFileaname
 * @param newFileaname
 * @param usernames
 */
void Controller::sendFileInformationChanges(QString oldFileaname, QString newFileaname, const QStringList& usernames){
    this->messanger->sendFileInfomationChanges(std::move(oldFileaname), std::move(newFileaname), usernames);
    startLoadingPopup();
}

/**
 * This method handles the elimination of a file
 * @param filename
 */
void Controller::sendDeleteFile(QString filename){
    this->messanger->sendDeleteFile(std::move(filename));
    startLoadingPopup();
}

Controller::~Controller(){
    this->crdtThread->quit();
    this->crdtThread->wait();
}

CRDT *Controller::getCrdt() const {
    return crdt;
}

/**
 * This method handles the change of view
 * @param window
 */
void Controller::handleGUI(QMainWindow *window) {
    now = window;
    GUI->setMinimumWidth(window->maximumWidth());
    GUI->setMinimumHeight(window->maximumHeight());
    QWidget *w = GUI->centralWidget();
    if (w != nullptr) {
        w->setParent(this);
        w->hide();
    }
    GUI->setCentralWidget(window);
    GUI->show();
    window->show();
}

bool Controller::isRequestFFile() const {
    return requestFFile;
}

void Controller::setRequestFFile(bool requestFFile) {
    Controller::requestFFile = requestFFile;
}

QMainWindow *Controller::getGui() const {
    return GUI;
}

Messanger *Controller::getMessanger() const {
    return messanger;
}

void Controller::reciveExternalException(){
    throw "Exception";
}

void Controller::inviledateTextEditor() {
    std::shared_lock<std::shared_mutex> requestLock(mutexRequestForFile);
    std::shared_lock<std::shared_mutex> isWorkingLock(getCrdt()->mutexIsWorking);
    editor->replaceText(this->crdt->getStructure());
}
