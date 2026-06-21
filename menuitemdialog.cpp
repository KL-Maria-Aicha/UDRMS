#include "menuitemdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
#include <QPixmap>

const QString MenuItemDialog::kDialogSS =
    "QDialog { background:#F4F9F6; }"
    "QLabel { color:#2C5C47; font-size:12px; font-family:'Segoe UI'; }"
    "QLineEdit, QComboBox {"
    "  border:2px solid #C8E6D8; border-radius:9px;"
    "  padding:8px 12px; font-size:13px; font-family:'Segoe UI';"
    "  background:#FFFFFF; color:#1A2E25; min-height:18px;"
    "}"
    "QLineEdit:focus, QComboBox:focus { border-color:#21897E; background:#F0FAF6; }"
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
    "QPushButton#photoBtn {"
    "  background:#E8F5EE; color:#21897E; border:2px dashed #4CAF8A;"
    "  border-radius:9px; padding:10px 18px;"
    "}"
    "QPushButton#photoBtn:hover { background:#D0F0E0; }"
    "QPushButton[text='Cancel'] { background:#F0FAF6; color:#21897E; border:2px solid #C8E6D8; }"
    "QPushButton[text='Cancel']:hover { background:#D8F2E8; }";

MenuItemDialog::MenuItemDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Add Menu Item");
    setFixedWidth(400);
    setStyleSheet(kDialogSS);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 20);
    root->setSpacing(0);

    auto* title = new QLabel("Add Menu Item");
    title->setStyleSheet(
        "color:#0A2A1F; font-size:17px; font-weight:800; font-family:'Segoe UI';"
        );
    root->addWidget(title);
    auto* sub = new QLabel("Add a dish to the cafeteria menu. Meals are subsidised — no price needed.");
    sub->setStyleSheet(
        "color:#6B8F7E; font-size:11px; font-family:'Segoe UI'; margin-bottom:4px;"
        );
    sub->setWordWrap(true);
    root->addWidget(sub);
    root->addSpacing(16);

    auto* form = new QFormLayout;
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    editName = new QLineEdit;
    editName->setPlaceholderText("e.g. Lentil Soup + Bread");
    form->addRow("Dish Name:", editName);

    comboType = new QComboBox;
    comboType->addItem("Breakfast", "breakfast");
    comboType->addItem("Lunch",     "lunch");
    comboType->addItem("Dinner",    "dinner");
    form->addRow("Meal Type:", comboType);

    root->addLayout(form);

    root->addSpacing(14);
    auto* photoSection = new QLabel("Food Photo  (optional)");
    photoSection->setStyleSheet(
        "color:#21897E; font-size:11px; font-weight:700; font-family:'Segoe UI';"
        " text-transform:uppercase; letter-spacing:0.5px;"
        );
    root->addWidget(photoSection);
    root->addSpacing(6);

    auto* photoRow = new QHBoxLayout;
    photoRow->setSpacing(12);

    photoPreview = new QLabel;
    photoPreview->setFixedSize(72, 72);
    photoPreview->setStyleSheet(
        "background:#E8F5EE; border:2px dashed #4CAF8A; border-radius:10px;"
        );
    photoPreview->setAlignment(Qt::AlignCenter);
    photoPreview->setText("No\nphoto");
    photoRow->addWidget(photoPreview);

    auto* photoRight = new QVBoxLayout;
    photoRight->setSpacing(6);
    btnPhoto = new QPushButton("Choose Image…");
    btnPhoto->setObjectName("photoBtn");
    btnPhoto->setCursor(Qt::PointingHandCursor);
    photoRight->addWidget(btnPhoto);
    auto* photoHint = new QLabel("PNG, JPG, or WEBP. Recommended: 400×300 px.");
    photoHint->setStyleSheet(
        "color:#9AB8AA; font-size:10px; font-family:'Segoe UI'; border:none; background:transparent;"
        );
    photoHint->setWordWrap(true);
    photoRight->addWidget(photoHint);
    photoRight->addStretch();
    photoRow->addLayout(photoRight, 1);
    root->addLayout(photoRow);

    root->addSpacing(20);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    root->addWidget(buttons);

    connect(btnPhoto, &QPushButton::clicked, this, &MenuItemDialog::choosePhoto);

    connect(buttons, &QDialogButtonBox::accepted, [this]() {
        if (editName->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation", "Dish name is required.");
            return;
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    adjustSize();
}

void MenuItemDialog::choosePhoto() {
    QString path = QFileDialog::getOpenFileName(
        this, "Choose Food Photo", "",
        "Images (*.png *.jpg *.jpeg *.webp *.bmp)");
    if (path.isEmpty()) return;
    chosenPhotoPath = path;
    QPixmap px(path);
    if (!px.isNull())
        photoPreview->setPixmap(
            px.scaled(photoPreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    btnPhoto->setText("✓  " + QFileInfo(path).fileName());
}

std::tuple<QString,QString,QString> MenuItemDialog::getData() const {
    return { editName->text().trimmed(),
             comboType->currentData().toString(),
             chosenPhotoPath };
}