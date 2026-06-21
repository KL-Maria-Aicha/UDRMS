#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <tuple>

class MenuItemDialog : public QDialog {
    Q_OBJECT
    QLineEdit*   editName;
    QComboBox*   comboType;
    QString      chosenPhotoPath;  // full filesystem path
    QLabel*      photoPreview;
    QPushButton* btnPhoto;

    static const QString kDialogSS;
public:
    explicit MenuItemDialog(QWidget* parent = nullptr);
    // returns {name, mealType, photoPath}
    std::tuple<QString,QString,QString> getData() const;

private slots:
    void choosePhoto();
};