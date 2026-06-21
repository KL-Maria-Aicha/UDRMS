#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include "student.h"

class StudentDialog : public QDialog {
    Q_OBJECT

    QLineEdit* editId;
    QLineEdit* editName;
    QSpinBox*  spinYear;
    QComboBox* comboGender;
    QLineEdit* editUsername;
    QLineEdit* editPassword;

    bool editMode;

    static QString dialogStyleSheet();

public:
    explicit StudentDialog(QWidget* parent = nullptr,
                           const Student* existing = nullptr);
    Student getStudent()  const;
    QString getUsername() const;
    QString getPassword() const;
};