#include "Controller.h"
#include "Networking/Client.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QCommandLineParser>
#include "ui/login.h"
#include "ui/ui_login.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Client c(&app);

    //c.logIn("username","password");
    //c.requestForFile("CiaoTy!");

    std::vector<int> numbers{1,2,3,4,5};
    /*c.deleteChar("c", "123", numbers);
    c.insert("c", "123", 1);*/
    //c.logOut();
    //c.deleteChar("c", 1);
    //c.deleteChar("c", numbers);
    //c.registration("ciao","ciao","/Users/andrea/Documents/sfondi/preview.jpeg");

    Login login;
    login.setClient(&c);
    login.show();
    c.connectTo("127.0.0.1");
    c.requestForFile("CiaoTy!");
    c.insert("c", "123", 1);

    /*Q_INIT_RESOURCE(textEditor);

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

    std::string siteId = "temp"; // TODO il server (?) genera un siteId univoco

    // model
    CRDT crdt{siteId};

    // view
    Editor editor{siteId};

    // controller
    Controller controller{&crdt, &editor};

    const QRect availableGeometry = QApplication::desktop()->availableGeometry(&editor);
    editor.resize(availableGeometry.width() / 2, (availableGeometry.height() * 2) / 3);
    editor.move((availableGeometry.width() - editor.width()) / 2,
            (availableGeometry.height() - editor.height()) / 2);

    editor.show();
*/
    return app.exec();
}