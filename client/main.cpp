#include "Controller.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QCommandLineParser>


int main(int argc, char *argv[]) {

    Q_INIT_RESOURCE(textEditor);

    // The QApplication class manages the GUI application's control flow and main settings.
    QApplication app(argc, argv);
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

    return app.exec();
}