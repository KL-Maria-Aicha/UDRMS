#include "studentdialog.h"
#include "user.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QIntValidator>
#include <QFrame>
#include <QLabel>

QString StudentDialog::dialogStyleSheet() {
    return
        "QDialog { background:#F4F9F6; }"
        "QLabel.section {"
        "  color:#0A2A1F; font-size:12px; font-weight:700;"
        "  font-family:'Segoe UI'; }"
        "QLabel { color:#2C5C47; font-size:12px; font-family:'Segoe UI'; }"
        "QLineEdit, QSpinBox, QComboBox {"
        "  border:2px solid #C8E6D8; border-radius:9px;"
        "  padding:8px 12px;"
        "  font-size:13px; font-family:'Segoe UI';"
        "  background:#FFFFFF; color:#1A2E25; min-height:18px;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus {"
        "  border-color:#21897E; background:#F0FAF6;"
        "}"
        "QLineEdit:hover:!focus, QSpinBox:hover:!focus, QComboBox:hover:!focus {"
        "  border-color:#4CAF8A;"
        "}"
        "QSpinBox::up-button, QSpinBox::down-button {"
        "  width:22px; border:none; background:#E8F5EE;"
        "  border-radius:4px; margin:2px 2px;"
        "}"
        "QSpinBox::up-button:hover, QSpinBox::down-button:hover {"
        "  background:#C0E8D0;"
        "}"
        "QSpinBox::up-arrow   { image:none; width:0; height:0;"
        "  border-left:5px solid transparent; border-right:5px solid transparent;"
        "  border-bottom:6px solid #21897E; }"
        "QSpinBox::down-arrow { image:none; width:0; height:0;"
        "  border-left:5px solid transparent; border-right:5px solid transparent;"
        "  border-top:6px solid #21897E; }"
        "QComboBox::drop-down { border:none; padding-right:10px; }"
        "QComboBox QAbstractItemView {"
        "  border:2px solid #C8E6D8; border-radius:8px; background:white;"
        "  selection-background-color:#E8F5EE; color:#1A2E25; padding:4px;"
        "}"
        "QPushButton {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "    stop:0 #1A7A50, stop:1 #2ECC8A);"
        "  color:white; border:none; border-radius:9px;"
        "  padding:9px 22px; font-size:13px; font-weight:700;"
        "  font-family:'Segoe UI';"
        "}"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "    stop:0 #218A60, stop:1 #27D478); }"
        "QPushButton[text='Cancel'], QPushButton[text='Annuler'] {"
        "  background:#F0FAF6; color:#21897E;"
        "  border:2px solid #C8E6D8;"
        "}"
        "QPushButton[text='Cancel']:hover { background:#D8F2E8; }";
}

StudentDialog::StudentDialog(QWidget* parent, const Student* existing)
    : QDialog(parent), editMode(existing != nullptr)
{
    setWindowTitle(editMode ? "Edit Student" : "Add Student");
    setFixedWidth(420);
    setStyleSheet(dialogStyleSheet());

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 20);
    root->setSpacing(0);

    auto* dlgTitle = new QLabel(editMode ? "Edit Student Record" : "Register New Student");
    dlgTitle->setStyleSheet(
        "color:#0A2A1F; font-size:17px; font-weight:800; font-family:'Segoe UI';"
        );
    root->addWidget(dlgTitle);

    auto* dlgSub = new QLabel(editMode
        ? "Update the student's information below."
        : "Fill in the student's details and set login credentials.");
    dlgSub->setStyleSheet(
        "color:#6B8F7E; font-size:11px; font-family:'Segoe UI'; margin-bottom:4px;"
        );
    dlgSub->setWordWrap(true);
    root->addWidget(dlgSub);

    root->addSpacing(18);

    auto* div1 = new QFrame; div1->setFrameShape(QFrame::HLine);
    div1->setStyleSheet("color:#D0E8DC; margin-bottom:12px;");
    root->addWidget(div1);

    auto* secLabel1 = new QLabel("Personal Information");
    secLabel1->setStyleSheet(
        "color:#21897E; font-size:11px; font-weight:700;"
        " font-family:'Segoe UI'; text-transform:uppercase; letter-spacing:0.5px;"
        );
    root->addWidget(secLabel1);
    root->addSpacing(8);

    auto* form = new QFormLayout;
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    editId = new QLineEdit;
    editId->setValidator(new QIntValidator(1, 99999999, this));
    editId->setPlaceholderText("e.g. 20231234");
    if (editMode) { editId->setText(QString::number(existing->getId())); editId->setEnabled(false); }
    form->addRow("Student ID:", editId);

    editName = new QLineEdit;
    editName->setPlaceholderText("Full name");
    if (editMode) editName->setText(existing->getName());
    form->addRow("Full Name:", editName);

    spinYear = new QSpinBox;
    spinYear->setRange(1, 5);
    spinYear->setSuffix("  (1–5)");
    spinYear->setMinimumWidth(120);
    if (editMode) spinYear->setValue(existing->getAcademicYear());
    form->addRow("Year:", spinYear);

    comboGender = new QComboBox;
    comboGender->addItems({"M — Male", "F — Female"});
    if (editMode && existing->getGender() == "F")
        comboGender->setCurrentIndex(1);
    form->addRow("Gender:", comboGender);

    root->addLayout(form);

    if (!editMode) {
        root->addSpacing(16);
        auto* div2 = new QFrame; div2->setFrameShape(QFrame::HLine);
        div2->setStyleSheet("color:#D0E8DC; margin-bottom:12px;");
        root->addWidget(div2);

        auto* secLabel2 = new QLabel("Login Credentials");
        secLabel2->setStyleSheet(
            "color:#21897E; font-size:11px; font-weight:700;"
            " font-family:'Segoe UI'; text-transform:uppercase; letter-spacing:0.5px;"
            );
        root->addWidget(secLabel2);
        root->addSpacing(8);

        auto* form2 = new QFormLayout;
        form2->setSpacing(10);
        form2->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

        editUsername = new QLineEdit;
        editUsername->setPlaceholderText("e.g. student_20231234");
        form2->addRow("Username:", editUsername);

        editPassword = new QLineEdit;
        editPassword->setEchoMode(QLineEdit::Password);
        editPassword->setPlaceholderText("Min. 8 characters");
        form2->addRow("Password:", editPassword);

        root->addLayout(form2);

        auto* pwdHint = new QLabel("⚠  Share these credentials with the student.");
        pwdHint->setStyleSheet(
            "color:#B07B00; font-size:10px; font-family:'Segoe UI';"
            " background:#FFFBEA; border-radius:6px; padding:6px 10px;"
            " border:1px solid #FFE082;"
            );
        root->addSpacing(6);
        root->addWidget(pwdHint);
    } else {
        editUsername = new QLineEdit; // unused placeholder in edit mode
        editPassword = new QLineEdit;
    }

    root->addSpacing(20);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    root->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, [this]() {
        if (editId->text().isEmpty() || editName->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation", "Student ID and Name are required.");
            return;
        }
        if (!editMode) {
            if (editUsername->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, "Validation", "Username is required."); return;
            }
            if (!User::isPasswordValid(editPassword->text())) {
                QMessageBox::warning(this, "Validation",
                                     "Password must be at least 8 characters."); return;
            }
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    adjustSize();
}

Student StudentDialog::getStudent() const {
    QString genderRaw = comboGender->currentText();
    QString gender = genderRaw.startsWith("M") ? "M" : "F";
    return Student(editId->text().toInt(),
                   editName->text().trimmed(),
                   18, spinYear->value(), gender);
}

QString StudentDialog::getUsername() const {
    return editUsername->text().trimmed();
}

QString StudentDialog::getPassword() const {
    return editPassword->text();
}