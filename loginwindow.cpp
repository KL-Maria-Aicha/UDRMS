#include "loginwindow.h"
#include "mainwindow.h"
#include "databasemanager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QSvgRenderer>

// ═══════════════════════════════════════════════════════════════════════════
//  ArtPanel — decorative left panel with SVG logo + illustrated buildings
// ═══════════════════════════════════════════════════════════════════════════
class ArtPanel : public QWidget {
    QSvgRenderer* logoRenderer = nullptr;
public:
    explicit ArtPanel(QWidget* p = nullptr) : QWidget(p) {
        setFixedWidth(400);
        logoRenderer = new QSvgRenderer(QString(":/icons/logo.svg"), this);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        const int W = width(), H = height();

        // ── Deep gradient background
        QLinearGradient bg(0, 0, W, H);
        bg.setColorAt(0.0, QColor("#0A2A1F"));
        bg.setColorAt(0.45, QColor("#12503A"));
        bg.setColorAt(1.0, QColor("#1A7A50"));
        p.fillRect(rect(), bg);

        // ── Subtle grid mesh
        p.setPen(QPen(QColor(255, 255, 255, 9), 1));
        for (int x = 0; x < W; x += 32) p.drawLine(x, 0, x, H);
        for (int y = 0; y < H; y += 32) p.drawLine(0, y, W, y);

        // ── Large background circles (decorative blobs)
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(46, 204, 138, 20));
        p.drawEllipse(W - 120, -80, 260, 260);
        p.setBrush(QColor(46, 204, 138, 12));
        p.drawEllipse(-60, H - 200, 220, 220);
        p.setBrush(QColor(33, 137, 126, 18));
        p.drawEllipse(W / 2 - 60, H / 2 - 60, 120, 120);

        // ── Stylised campus illustration
        QLinearGradient gnd(0, H - 90, 0, H - 60);
        gnd.setColorAt(0, QColor("#1A9B60"));
        gnd.setColorAt(1, QColor("#0A2A1F"));
        p.setBrush(gnd);
        p.drawRect(0, H - 90, W, 40);

        auto drawBuilding = [&](int bx, int by, int bw, int bh,
                                QColor bodyTop, QColor bodyBot,
                                QColor roofC, bool flatRoof,
                                QVector<QPair<int,int>> winPositions,
                                QVector<bool> winLit) {
            QLinearGradient body(bx, by, bx, by + bh);
            body.setColorAt(0, bodyTop);
            body.setColorAt(1, bodyBot);
            p.setBrush(body);
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(bx, by, bw, bh, 3, 3);

            if (flatRoof) {
                p.setBrush(roofC);
                p.drawRect(bx - 3, by - 7, bw + 6, 10);
                p.setBrush(QColor("#5CE89A"));
                p.drawRect(bx - 3, by - 7, bw + 6, 4);
            } else {
                QPainterPath roof;
                roof.moveTo(bx - 6, by);
                roof.lineTo(bx + bw / 2, by - bh * 0.32);
                roof.lineTo(bx + bw + 6, by);
                roof.closeSubpath();
                p.setBrush(roofC);
                p.drawPath(roof);
                p.setPen(QPen(QColor("#5CE89A"), 2));
                p.drawLine(bx - 6, by, bx + bw / 2, by - (int)(bh * 0.32));
                p.drawLine(bx + bw / 2, by - (int)(bh * 0.32), bx + bw + 6, by);
                p.setPen(Qt::NoPen);
            }

            for (int i = 0; i < winPositions.size(); ++i) {
                int wx = bx + winPositions[i].first;
                int wy = by + winPositions[i].second;
                bool lit = i < winLit.size() ? winLit[i] : false;
                if (lit) {
                    QLinearGradient wg(wx, wy, wx, wy + 18);
                    wg.setColorAt(0, QColor("#FFFDE7"));
                    wg.setColorAt(1, QColor("#FFE082"));
                    p.setBrush(wg);
                } else {
                    p.setBrush(QColor(0, 0, 0, 100));
                }
                p.drawRoundedRect(wx, wy, 20, 18, 2, 2);
                p.setPen(QPen(QColor(255, 255, 255, 50), 1));
                p.drawLine(wx + 10, wy, wx + 10, wy + 18);
                p.drawLine(wx, wy + 9,  wx + 20, wy + 9);
                p.setPen(Qt::NoPen);
            }
        };

        drawBuilding(50, H - 295, 160, 210,
                     QColor("#2ECC8A"), QColor("#1A8050"),
                     QColor("#145C39"), false,
                     {{10,20},{45,20},{80,20},{115,20},
                      {10,55},{45,55},{80,55},{115,55},
                      {10,90},{45,90},{80,90},{115,90}},
                     {true,false,true,true,false,true,true,false,true,true,false,true});
        p.setBrush(QColor(0, 0, 0, 120));
        p.drawRoundedRect(50 + 60, H - 295 + 145, 40, 65, 3, 3);
        p.setPen(QPen(QColor("#5CE89A"), 1.5));
        p.drawLine(50 + 80, H - 295 + 155, 50 + 80, H - 295 + 200);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#5CE89A"));
        p.drawEllipse(50 + 72, H - 295 + 175, 6, 6);

        drawBuilding(230, H - 245, 110, 160,
                     QColor("#27AE74"), QColor("#186845"),
                     QColor("#0F4A30"), true,
                     {{12,15},{55,15},{12,45},{55,45},{12,75},{55,75}},
                     {false,true,true,false,false,true});

        drawBuilding(10, H - 210, 35, 125,
                     QColor("#1A9060"), QColor("#125C40"),
                     QColor("#0D3B2E"), true,
                     {{7, 10},{7, 38},{7, 66}},
                     {true, false, true});

        auto drawTree = [&](int tx, int ty, int sz) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor("#082A18"));
            p.drawRect(tx + sz/2 - 3, ty + sz, 6, sz/2);
            for (int tier = 0; tier < 3; ++tier) {
                int expand = (2 - tier) * 4;
                p.setBrush(QColor(0x1D, 0xB8, 0x64, 200 - tier * 30));
                QPainterPath crown;
                int cx = tx + sz / 2;
                int cy = ty + tier * (sz / 4);
                crown.moveTo(cx - sz / 2 - expand, cy + sz / 2);
                crown.lineTo(cx,                   cy);
                crown.lineTo(cx + sz / 2 + expand, cy + sz / 2);
                crown.closeSubpath();
                p.drawPath(crown);
            }
        };
        drawTree(350, H - 190, 32);
        drawTree(318, H - 175, 26);
        drawTree(22,  H - 175, 24);

        static const QPointF stars[] = {
            {55,45},{130,25},{220,60},{300,30},{360,80},{180,15},{95,100}
        };
        for (auto& s : stars) {
            double alpha = 0.3 + 0.7 * ((int(s.x() * s.y()) % 100) / 100.0);
            p.setBrush(QColor(92, 232, 154, (int)(alpha * 220)));
            p.drawEllipse(s, 2.5, 2.5);
            p.setPen(QPen(QColor(92, 232, 154, (int)(alpha * 140)), 1));
            p.drawLine(s.x() - 5, s.y(), s.x() + 5, s.y());
            p.drawLine(s.x(), s.y() - 5, s.x(), s.y() + 5);
            p.setPen(Qt::NoPen);
        }

        if (logoRenderer && logoRenderer->isValid()) {
            logoRenderer->render(&p, QRectF(W / 2 - 36, 36, 72, 72));
        }

        p.setPen(QColor(255, 255, 255, 230));
        QFont titleFont("Segoe UI", 20, QFont::Bold);
        p.setFont(titleFont);
        p.drawText(QRect(20, 124, W - 40, 36), Qt::AlignCenter, "UDRMS");

        QFont subFont("Segoe UI", 10);
        p.setFont(subFont);
        p.setPen(QColor(92, 230, 150, 200));
        p.drawText(QRect(20, 160, W - 40, 22), Qt::AlignCenter,
                   "University Dormitory & Restaurant");
        p.drawText(QRect(20, 180, W - 40, 22), Qt::AlignCenter,
                   "Management System — ENSIA");

        QLinearGradient ln(20, 0, W - 20, 0);
        ln.setColorAt(0,   QColor(92, 230, 150, 0));
        ln.setColorAt(0.5, QColor(92, 230, 150, 160));
        ln.setColorAt(1,   QColor(92, 230, 150, 0));
        p.setPen(QPen(QBrush(ln), 1.5));
        p.drawLine(20, 208, W - 20, 208);

        p.setBrush(QColor(255, 255, 255, 14));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(20, H - 54, W - 40, 34, 10, 10);
        p.setPen(QColor(92, 230, 150, 180));
        p.setFont(QFont("Segoe UI", 9));
        p.drawText(QRect(20, H - 54, W - 40, 34),
                   Qt::AlignCenter, "École Nationale Supérieure d'Informatique");
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  LoginWindow
// ═══════════════════════════════════════════════════════════════════════════
LoginWindow::LoginWindow(QWidget* parent) : QDialog(parent) {
    setWindowTitle("UDRMS — Sign In");
    setFixedSize(820, 560);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    root->addWidget(new ArtPanel(this));

    auto* formPanel = new QWidget;
    formPanel->setObjectName("formPanel");
    formPanel->setStyleSheet(
        "QWidget#formPanel { background:#F4F9F6; }"
        );
    auto* fl = new QVBoxLayout(formPanel);
    fl->setContentsMargins(52, 44, 52, 36);
    fl->setSpacing(0);

    auto* topRow = new QHBoxLayout;
    topRow->addStretch();
    auto* btnClose = new QPushButton("✕");
    btnClose->setFixedSize(32, 32);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet(
        "QPushButton { background:transparent; color:#90A89C; border:none; font-size:15px; }"
        "QPushButton:hover { color:#C0392B; background:#FFEEF0; border-radius:16px; }"
        );
    connect(btnClose, &QPushButton::clicked, this, &QDialog::reject);
    topRow->addWidget(btnClose);
    fl->addLayout(topRow);

    fl->addSpacing(14);

    auto* h1 = new QLabel("Welcome back");
    h1->setStyleSheet(
        "color:#0A2A1F; font-size:28px; font-weight:800; font-family:'Segoe UI';"
        " letter-spacing:-0.5px;"
        );
    fl->addWidget(h1);

    auto* h2 = new QLabel("Sign in to your UDRMS account");
    h2->setStyleSheet(
        "color:#6B8F7E; font-size:13px; font-family:'Segoe UI'; margin-top:2px;"
        );
    fl->addWidget(h2);

    fl->addSpacing(32);

    const QString fieldSS =
        "QLineEdit {"
        "  background:#FFFFFF; color:#1A2E25;"
        "  padding:13px 16px;"
        "  border:2px solid #D0E8DC;"
        "  border-radius:12px;"
        "  font-size:14px; font-family:'Segoe UI';"
        "}"
        "QLineEdit:focus { border-color:#21897E; background:#F0FAF6; }"
        "QLineEdit:hover:!focus { border-color:#4CAF8A; }";

    auto* lbUser = new QLabel("Username");
    lbUser->setStyleSheet(
        "color:#2C5C47; font-size:12px; font-weight:700; font-family:'Segoe UI';"
        );
    fl->addWidget(lbUser);
    fl->addSpacing(5);
    siUser = new QLineEdit;
    siUser->setPlaceholderText("Enter your username");
    siUser->setStyleSheet(fieldSS);
    siUser->setFixedHeight(50);
    fl->addWidget(siUser);

    fl->addSpacing(16);

    auto* lbPass = new QLabel("Password");
    lbPass->setStyleSheet(
        "color:#2C5C47; font-size:12px; font-weight:700; font-family:'Segoe UI';"
        );
    fl->addWidget(lbPass);
    fl->addSpacing(5);
    siPass = new QLineEdit;
    siPass->setPlaceholderText("Enter your password (min. 8 characters)");
    siPass->setEchoMode(QLineEdit::Password);
    siPass->setStyleSheet(fieldSS);
    siPass->setFixedHeight(50);
    fl->addWidget(siPass);

    fl->addSpacing(10);

    siError = new QLabel;
    siError->setStyleSheet(
        "color:#C0392B; font-size:12px; font-family:'Segoe UI';"
        " background:#FFEEF0; border-radius:8px; padding:8px 14px;"
        );
    siError->setAlignment(Qt::AlignCenter);
    siError->setVisible(false);
    siError->setWordWrap(true);
    fl->addWidget(siError);

    fl->addSpacing(22);

    auto* btnSignIn = new QPushButton("Sign In");
    btnSignIn->setFixedHeight(52);
    btnSignIn->setCursor(Qt::PointingHandCursor);
    btnSignIn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "    stop:0 #1A7A50, stop:1 #2ECC8A);"
        "  color:white; border:none; border-radius:12px;"
        "  font-size:15px; font-weight:700; font-family:'Segoe UI';"
        "  letter-spacing:0.3px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "    stop:0 #218A60, stop:1 #27D478);"
        "}"
        "QPushButton:pressed {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "    stop:0 #145C3A, stop:1 #1FAA5A);"
        "  padding-top:2px;"
        "}"
        );
    auto* btnShadow = new QGraphicsDropShadowEffect;
    btnShadow->setBlurRadius(28);
    btnShadow->setOffset(0, 6);
    btnShadow->setColor(QColor(26, 122, 80, 80));
    btnSignIn->setGraphicsEffect(btnShadow);
    fl->addWidget(btnSignIn);

    fl->addSpacing(24);

    auto* infoBox = new QWidget;
    infoBox->setStyleSheet(
        "QWidget { background:#E8F5EE; border-radius:10px;"
        "  border:1px solid #C0E8D0; }"
        );
    auto* infoLayout = new QHBoxLayout(infoBox);
    infoLayout->setContentsMargins(14, 10, 14, 10);
    infoLayout->setSpacing(10);
    auto* infoIcon = new QLabel("ℹ");
    infoIcon->setStyleSheet(
        "background:transparent; color:#21897E; font-size:16px; border:none;"
        );
    auto* infoText = new QLabel(
        "Student accounts are created by the dormitory\n"
        "administrator when you are registered."
        );
    infoText->setStyleSheet(
        "background:transparent; border:none; color:#2C5C47;"
        " font-size:11px; font-family:'Segoe UI';"
        );
    infoLayout->addWidget(infoIcon);
    infoLayout->addWidget(infoText, 1);
    fl->addWidget(infoBox);

    fl->addStretch();

    auto* footer = new QLabel("© 2025 ENSIA  ·  UDRMS v2.0");
    footer->setStyleSheet(
        "color:#B0CCB8; font-size:10px; font-family:'Segoe UI';"
        );
    footer->setAlignment(Qt::AlignCenter);
    fl->addWidget(footer);

    root->addWidget(formPanel, 1);

    connect(btnSignIn, &QPushButton::clicked, this, &LoginWindow::attemptLogin);
    connect(siPass, &QLineEdit::returnPressed, this, &LoginWindow::attemptLogin);
    connect(siUser, &QLineEdit::returnPressed, this, &LoginWindow::attemptLogin);
}

void LoginWindow::attemptLogin() {
    siError->setVisible(false);
    QString u = siUser->text().trimmed();
    QString pw = siPass->text();

    if (u.isEmpty() || pw.isEmpty()) {
        siError->setText("⚠  Please enter both username and password.");
        siError->setVisible(true);
        return;
    }

    User* user = DatabaseManager::getInstance()->authenticateUser(u, pw);
    if (!user) {
        siError->setText("⚠  Invalid username or password. Please try again.");
        siError->setVisible(true);
        auto* anim = new QPropertyAnimation(this, "pos");
        anim->setDuration(280);
        QPoint orig = pos();
        anim->setKeyValueAt(0.0,  orig);
        anim->setKeyValueAt(0.15, orig + QPoint(-8, 0));
        anim->setKeyValueAt(0.3,  orig + QPoint(8, 0));
        anim->setKeyValueAt(0.5,  orig + QPoint(-5, 0));
        anim->setKeyValueAt(0.7,  orig + QPoint(5, 0));
        anim->setKeyValueAt(0.85, orig + QPoint(-2, 0));
        anim->setKeyValueAt(1.0,  orig);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
        return;
    }

    auto* main = new MainWindow(user);
    main->show();
    accept();
}