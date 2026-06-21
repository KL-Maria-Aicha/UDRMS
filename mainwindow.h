#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QFrame>
#include <QDateEdit>
#include "user.h"

class DonutChart;
class BarChart;

class MainWindow : public QMainWindow {
    Q_OBJECT

    User*           currentUser;

    // ── Sidebar
    QWidget*        sidebar       = nullptr;
    QStackedWidget* stack         = nullptr;
    QVector<QPushButton*> navButtons;
    QLabel*         topBarTitle   = nullptr;
    QLabel*         topBarSub     = nullptr;

    // ── Staff/Admin: Students page
    QTableWidget* studentTable   = nullptr;
    QLineEdit*    studentSearch  = nullptr;

    // ── Staff/Admin: Rooms page
    QComboBox*    dormSelector   = nullptr;
    QTableWidget* roomTable      = nullptr;

    // ── Staff/Admin: Restaurant page
    QComboBox*    restDormSelector = nullptr;
    QTableWidget* menuTable        = nullptr;
    QTableWidget* reservTable      = nullptr;
    QDateEdit*    reservDateEdit   = nullptr;

    // ── Admin: Reports page
    QTableWidget* reportTable    = nullptr;
    DonutChart*   donutChart     = nullptr;
    BarChart*     barChart       = nullptr;
    QTableWidget* reservReport   = nullptr;
    QDateEdit*    rptFrom        = nullptr;
    QDateEdit*    rptTo          = nullptr;

    // ── Admin: Users page
    QTableWidget* userTable      = nullptr;

    // ── Student portal widgets
    QLabel*       spName         = nullptr;
    QLabel*       spId           = nullptr;
    QLabel*       spRoom         = nullptr;
    QLabel*       spDorm         = nullptr;
    QTableWidget* spMenuTable    = nullptr;
    QTableWidget* spReservTable  = nullptr;
    QDateEdit*    spDateEdit     = nullptr;
    QComboBox*    spDormCombo    = nullptr;

    // ── Setup helpers
    void buildSidebar();
    void buildTopBar(QVBoxLayout* rightCol);
    void applyGlobalStyle();
    QWidget* pageStudents();
    QWidget* pageRooms();
    QWidget* pageRestaurant();
    QWidget* pageReports();
    QWidget* pageUsers();
    QWidget* pageStudentPortal();

    void fillDormCombo(QComboBox* combo);
    QWidget* makeStatCard(const QString& value, const QString& label,
                          const QString& accent, const QString& bgTint,
                          const QString& iconPath);
    QWidget* makeBanner(const QString& svgRes, const QString& title,
                        const QString& subtitle);
    QIcon    svgIcon(const QString& resourcePath, const QColor& color);
    void     switchPage(int index, const QString& title, const QString& sub);

public:
    explicit MainWindow(User* user, QWidget* parent = nullptr);
    ~MainWindow() override;

public slots:
    void refreshStudentTable();
    void refreshRoomTable();
    void refreshMenuTable();
    void refreshReservTable();
    void refreshOccupancyReport();
    void refreshReservReport();
    void refreshUserTable();

    void onAddStudent();
    void onEditStudent();
    void onDeleteStudent();
    void onAssignRoom();
    void onVacateStudent();

    void onAddDormitory();
    void onEditDormitory();
    void onAddRoom();
    void onDeleteRoom();

    void onAddMenuItem();
    void onDeleteMenuItem();

    void onAddUser();
    void onToggleUserActive();
    void onLogout();

    void spRefreshMenu();
    void spRefreshReservations();
    void spReserve();
    void spCancel();
};