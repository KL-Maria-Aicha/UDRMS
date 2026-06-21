#include "dormdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QLabel>

static const QString kDormSS =
    "QDialog { background:#F4F9F6; }"
    "QLabel  { color:#2C5C47; font-size:12px; font-family:'Segoe UI'; }"
    "QLineEdit, QSpinBox {"
    "  border:2px solid #C8E6D8; border-radius:9px;"
    "  padding:8px 12px; font-size:13px; font-family:'Segoe UI';"
    "  background:#FFFFFF; color:#1A2E25; min-height:18px;"
    "}"
    "QLineEdit:focus, QSpinBox:focus { border-color:#21897E; background:#F0FAF6; }"
    "QLineEdit:hover:!focus, QSpinBox:hover:!focus { border-color:#4CAF8A; }"
    "QSpinBox::up-button, QSpinBox::down-button {"
    "  width:22px; border:none; background:#E8F5EE; border-radius:4px; margin:2px 2px;"
    "}"
    "QSpinBox::up-button:hover, QSpinBox::down-button:hover { background:#C0E8D0; }"
    "QSpinBox::up-arrow   { image:none; width:0; height:0;"
    "  border-left:5px solid transparent; border-right:5px solid transparent;"
    "  border-bottom:6px solid #21897E; }"
    "QSpinBox::down-arrow { image:none; width:0; height:0;"
    "  border-left:5px solid transparent; border-right:5px solid transparent;"
    "  border-top:6px solid #21897E; }"
    "QPushButton {"
    "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
    "    stop:0 #1A7A50, stop:1 #2ECC8A);"
    "  color:white; border:none; border-radius:9px;"
    "  padding:9px 22px; font-size:13px; font-weight:700; font-family:'Segoe UI';"
    "}"
    "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
    "    stop:0 #218A60, stop:1 #27D478); }"
    "QPushButton[text='Cancel'] { background:#F0FAF6; color:#21897E; border:2px solid #C8E6D8; }"
    "QPushButton[text='Cancel']:hover { background:#D8F2E8; }";

DormDialog::DormDialog(QWidget* parent, const DormData* existing)
    : QDialog(parent)
{
    bool editMode = (existing != nullptr);
    setWindowTitle(editMode ? "Edit Dormitory" : "Add Dormitory");
    setFixedWidth(420);
    setStyleSheet(kDormSS);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 20);
    root->setSpacing(0);

    auto* title = new QLabel(editMode ? "Edit Dormitory" : "Add New Dormitory");
    title->setStyleSheet(
        "color:#0A2A1F; font-size:17px; font-weight:800; font-family:'Segoe UI';"
        );
    root->addWidget(title);
    auto* sub = new QLabel(
        "The code (e.g. A, B) is used in room IDs like A2-3. "
        "The display name is shown in reports.");
    sub->setStyleSheet(
        "color:#6B8F7E; font-size:11px; font-family:'Segoe UI'; margin-bottom:4px;"
        );
    sub->setWordWrap(true);
    root->addWidget(sub);
    root->addSpacing(16);

    auto* form = new QFormLayout;
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    editCode = new QLineEdit;
    editCode->setPlaceholderText("e.g. A  (single letter or short code)");
    editCode->setMaxLength(3);
    if (editMode) editCode->setText(existing->code);
    form->addRow("Code:", editCode);

    editName = new QLineEdit;
    editName->setPlaceholderText("e.g. Résidence Universitaire A");
    if (editMode) editName->setText(existing->displayName);
    form->addRow("Display Name:", editName);

    editAddr = new QLineEdit;
    editAddr->setPlaceholderText("Campus address");
    if (editMode) editAddr->setText(existing->address);
    form->addRow("Address:", editAddr);

    spinCap = new QSpinBox;
    spinCap->setRange(1, 9999);
    spinCap->setValue(editMode ? existing->capacity : 120);
    form->addRow("Total Capacity:", spinCap);

    editRest = new QLineEdit;
    editRest->setPlaceholderText("e.g. Cafétéria A");
    if (editMode) editRest->setText(existing->restaurantName);
    else          editRest->setText("Cafétéria");
    form->addRow("Restaurant Name:", editRest);

    root->addLayout(form);
    root->addSpacing(20);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    root->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, [this]() {
        if (editCode->text().trimmed().isEmpty() ||
            editName->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation",
                                 "Code and Display Name are required.");
            return;
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    adjustSize();
}

DormData DormDialog::getData() const {
    return { editCode->text().trimmed().toUpper(),
             editName->text().trimmed(),
             editAddr->text().trimmed(),
             spinCap->value(),
             editRest->text().trimmed().isEmpty() ? "Cafétéria" : editRest->text().trimmed() };
}