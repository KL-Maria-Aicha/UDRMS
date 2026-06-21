#include <QApplication>
#include <QMessageBox>
#include "databasemanager.h"
#include "loginwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("UDRMS");
    app.setOrganizationName("ENSIA");
    app.setApplicationVersion("2.0");

    if (!DatabaseManager::getInstance()->initDatabase()) {
        QMessageBox::critical(nullptr, "Fatal Error",
                              "Database initialisation failed. Cannot start.");
        return 1;
    }

    LoginWindow login;
    login.show();
    return app.exec();
}