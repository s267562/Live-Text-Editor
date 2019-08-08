#include "Editor.h"

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
    CRDT crdt(siteId);

    // view-controller
    Editor te(siteId, crdt);

    const QRect availableGeometry = QApplication::desktop()->availableGeometry(&te);
    te.resize(availableGeometry.width() / 2, (availableGeometry.height() * 2) / 3);
    te.move((availableGeometry.width() - te.width()) / 2,
            (availableGeometry.height() - te.height()) / 2);

    te.show();

    return app.exec();
}