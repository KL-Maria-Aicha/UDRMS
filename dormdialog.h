#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>

struct DormData {
    QString code;
    QString displayName;
    QString address;
    int     capacity;
    QString restaurantName;
};

class DormDialog : public QDialog {
    Q_OBJECT
    QLineEdit* editCode;
    QLineEdit* editName;
    QLineEdit* editAddr;
    QSpinBox*  spinCap;
    QLineEdit* editRest;
public:
    explicit DormDialog(QWidget* parent = nullptr,
                        const DormData* existing = nullptr);
    DormData getData() const;
};