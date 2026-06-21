#include "roomdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>

static const QString kDialogSS =
    "QDialog { background:#F4F9F6; }"
    "QLabel { color:#2C5C47; font-size:12px; font-family:'Segoe UI'; }"
    "QSpinBox, QComboBox {"
    "  border:2px solid #C8E6D8; border-radius:9px;"
    "  padding:8px 12px; font-size:13px; font-family:'Segoe UI';"
    "  background:#FFFFFF; color:#1A2E25; min-height:18px;"
    "}"
    "QSpinBox:focus, QComboBox:focus { border-color:#21897E; background:#F0FAF6; }"
    "QSpinBox:hover:!focus { border-color:#4CAF8A; }"
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
    "QComboBox::drop-down { border:none; padding-right:10px; }"
    "QComboBox QAbstractItemView {"
    "  border:2px solid #C8E6D8; border-radius:8px; background:white;"
    "  selection-background-color:#E8F5EE; color:#1A2E25; padding:4px;"
    "}"
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

RoomDialog::RoomDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Add Room");
    setFixedWidth(360);
    setStyleSheet(kDialogSS);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 20);
    root->setSpacing(0);

    auto* title = new QLabel("Add New Room");
    title->setStyleSheet(
        "color:#0A2A1F; font-size:17px; font-weight:800; font-family:'Segoe UI';"
        );
    root->addWidget(title);
    auto* sub = new QLabel("Room ID will be displayed as Dorm+Floor-Number (e.g. A2-5)");
    sub->setStyleSheet(
        "color:#6B8F7E; font-size:11px; font-family:'Segoe UI'; margin-bottom:4px;"
        );
    sub->setWordWrap(true);
    root->addWidget(sub);
    root->addSpacing(16);

    auto* form = new QFormLayout;
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    spinNumber   = new QSpinBox; spinNumber->setRange(1, 999);
    spinFloor    = new QSpinBox; spinFloor->setRange(1, 30);
    spinCapacity = new QSpinBox; spinCapacity->setRange(1, 10); spinCapacity->setValue(2);
    comboType    = new QComboBox;
    comboType->addItems({"Single","Double","Triple","Quad","Suite"});

    form->addRow("Room Number:", spinNumber);
    form->addRow("Floor:",       spinFloor);
    form->addRow("Capacity:",    spinCapacity);
    form->addRow("Type:",        comboType);
    root->addLayout(form);

    root->addSpacing(20);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    root->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    adjustSize();
}

std::tuple<int,int,int,QString> RoomDialog::getRoomData() const {
    return {spinNumber->value(), spinFloor->value(),
            spinCapacity->value(), comboType->currentText()};
}