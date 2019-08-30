#include "Controller.h"
#include "Networking/Client.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QCommandLineParser>
#include "ui/login.h"
#include "ui/ui_login.h"
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    /* MAINWINDOW */
    MainWindow mw("temp");
    mw.show();

    /* TEST: REGISTRATION FEATURE
    Client client;
    client.connectTo("127.0.0.1");
    client.registration("ciao","ciao","/Users/andrea/Documents/sfondi/preview.jpeg");*/

    /* TEST: TEXT EDITOR */
    /*Client client;
    client.connectTo("127.0.0.1");
    client.logIn("prova","prova");

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

    QString siteId = "temp"; // TODO il server (?) genera un siteId univoco

    // model
    CRDT crdt{siteId};

    // view
    Editor editor{siteId};

    // controller
    Controller controller{&crdt, &editor, &client};

    const QRect availableGeometry = QApplication::desktop()->availableGeometry(&editor);
    editor.resize(availableGeometry.width() / 2, (availableGeometry.height() * 2) / 3);
    editor.move((availableGeometry.width() - editor.width()) / 2,
            (availableGeometry.height() - editor.height()) / 2);

    editor.show();*/

    return app.exec();
}