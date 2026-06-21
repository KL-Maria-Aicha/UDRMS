#pragma once
#include <QDialog>
#include <QStackedWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class LoginWindow : public QDialog {
    Q_OBJECT

    QStackedWidget* stack;

    QLineEdit*   siUser;
    QLineEdit*   siPass;
    QLabel*      siError;

    void buildSignInPage(QWidget* page);
    void buildArtPanel(QWidget* panel);

public:
    explicit LoginWindow(QWidget* parent = nullptr);

private slots:
    void attemptLogin();
};