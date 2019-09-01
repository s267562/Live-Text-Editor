#include "Controller.h"
#include "Networking/Client.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QCommandLineParser>
#include "ui/login.h"
#include "ui/ui_login.h"
#include "MainWindow.h"
#include "utils/Constants.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

#if MAINWINDOW
    /* MAINWINDOW */
    MainWindow mw("temp");
    mw.show();
#else
    #if REGISTRATION_TEST
        /* TEST: REGISTRATION FEATURE */
        Client client;
        client.connectTo("127.0.0.1");
        client.registration("ciao","ciao","/Users/andrea/Documents/sfondi/preview.jpeg");
    #else
        /* TEST: TEXT EDITOR */
        Client client;

        CRDT crdt{}; // model
        client.setCRDT( &crdt);

        client.connectTo("127.0.0.1");
        QString username = "u2";
        client.logIn(username, "prova");

        QString siteId = username; // TODO il server (?) genera un siteId univoco
        crdt.setSiteId(siteId);

        Q_INIT_RESOURCE(textEditor);

        // The QApplication class manages the GUI application's control flow and main settings.
        QCoreApplication::setOrganizationName("QtProject");
        QCoreApplication::setApplicationName("text editor");
        QCoreApplication::setApplicationVersion(QT_VERSION_STR);

        QCommandLineParser parser;
        parser.setApplicationDescription(QCoreApplication::applicationName());
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addPositionalArgument("file", "The file to open.");
        parser.process(app);

        // view
        Editor editor{siteId};

        // controller
        Controller controller{&crdt, &editor, &client};


        editor.show();
    #endif
#endif
    return app.exec();
}