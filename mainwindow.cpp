#include "mainwindow.h"
#include "studentdialog.h"
#include "roomdialog.h"
#include "menuitemdialog.h"
#include "dormdialog.h"
#include "databasemanager.h"
#include "loginwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>
#include <QDateEdit>
#include <QGroupBox>
#include <QSvgRenderer>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QButtonGroup>
#include <cmath>

// ═══════════════════════════════════════════════════════════════════════════
//  Design tokens — soft, flat, card-based palette matching the reference
// ═══════════════════════════════════════════════════════════════════════════
namespace C {
    const QString BG     = "#F3F8F5";   // page background
    const QString SIDEBAR= "#FFFFFF";
    const QString CARD   = "#FFFFFF";
    const QString BDR    = "#E3EFE9";
    const QString TXT    = "#16261F";
    const QString SUBT   = "#6B8478";
    const QString MUTED  = "#9FB6AB";

    const QString PRIMARY     = "#1E9E6B";  // main accent green
    const QString PRIMARY_DK  = "#0F6B47";
    const QString PRIMARY_LT  = "#E6F6EE";  // pale green tint for badges
    const QString PRIMARY_LT2 = "#D3F0E2";

    const QString NAVY     = "#3457D5";
    const QString NAVY_LT  = "#E8EDFC";
    const QString PURPLE   = "#8B5CF6";
    const QString PURPLE_LT= "#F1EBFE";
    const QString AMBER    = "#E8A93B";
    const QString AMBER_LT = "#FCF1DD";
    const QString RED      = "#E25C5C";
    const QString RED_LT   = "#FCEAEA";
    const QString CYAN     = "#2BAFC4";
    const QString CYAN_LT  = "#E3F6F9";
}

static QGraphicsDropShadowEffect* mkShadow(int blur=18, int dy=4, int alpha=22) {
    auto* s = new QGraphicsDropShadowEffect;
    s->setBlurRadius(blur);
    s->setOffset(0, dy);
    s->setColor(QColor(20, 40, 35, alpha));
    return s;
}

static QPixmap renderSvgTinted(const QString& resPath, const QColor& color, int sz) {
    QFile f(resPath);
    QPixmap pm(sz, sz);
    pm.fill(Qt::transparent);
    if (!f.open(QIODevice::ReadOnly)) return pm;
    QByteArray data = f.readAll();
    data.replace("currentColor", color.name(QColor::HexRgb).toLatin1());
    QSvgRenderer ren(data);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    ren.render(&p);
    return pm;
}

QIcon MainWindow::svgIcon(const QString& resPath, const QColor& color) {
    return QIcon(renderSvgTinted(resPath, color, 22));
}

static void styleTable(QTableWidget* t) {
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setAlternatingRowColors(true);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    t->verticalHeader()->setVisible(false);
    t->setShowGrid(false);
    t->setFocusPolicy(Qt::NoFocus);
    t->setFrameShape(QFrame::NoFrame);
    t->verticalHeader()->setDefaultSectionSize(46);
    t->setStyleSheet(
        "QTableWidget {"
        "  background:#FFFFFF; alternate-background-color:#F7FBF9;"
        "  font-size:13px; font-family:'Segoe UI';"
        "  gridline-color:transparent; border:none;"
        "  selection-background-color:#D3F0E2; selection-color:#16261F;"
        "  color:#16261F;"
        "}"
        "QHeaderView::section {"
        "  background:#FFFFFF; color:#6B8478; font-weight:700; font-size:11px;"
        "  font-family:'Segoe UI'; text-transform:uppercase; letter-spacing:0.3px;"
        "  padding:12px 10px; border:none; border-bottom:2px solid #E3EFE9;"
        "}"
        "QScrollBar:vertical { background:#F3F8F5; width:8px; border-radius:4px; }"
        "QScrollBar::handle:vertical { background:#B7E4CB; border-radius:4px; min-height:24px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }"
        );
}

static QWidget* wrapCard(QWidget* inner, int margin = 0) {
    auto* card = new QWidget;
    card->setStyleSheet("QWidget{background:white; border-radius:16px;}");
    card->setGraphicsEffect(mkShadow());
    auto* l = new QVBoxLayout(card);
    l->setContentsMargins(margin, margin, margin, margin);
    l->addWidget(inner);
    return card;
}

enum class BtnStyle { Primary, Secondary, Danger, Ghost };

static QPushButton* makeBtn(const QString& text, BtnStyle v = BtnStyle::Primary) {
    auto* b = new QPushButton(text);
    b->setCursor(Qt::PointingHandCursor);
    b->setMinimumHeight(40);
    switch (v) {
    case BtnStyle::Primary:
        b->setStyleSheet(
            "QPushButton { background:#1E9E6B; color:white; border:none;"
            "  border-radius:10px; padding:0 18px; font-size:13px;"
            "  font-weight:700; font-family:'Segoe UI'; }"
            "QPushButton:hover { background:#23B57B; }"
            "QPushButton:pressed { background:#0F6B47; }"
            );
        break;
    case BtnStyle::Secondary:
        b->setStyleSheet(
            "QPushButton { background:#E6F6EE; color:#0F6B47; border:none;"
            "  border-radius:10px; padding:0 18px; font-size:13px;"
            "  font-weight:700; font-family:'Segoe UI'; }"
            "QPushButton:hover { background:#D3F0E2; }"
            "QPushButton:pressed { background:#B7E4CB; }"
            );
        break;
    case BtnStyle::Danger:
        b->setStyleSheet(
            "QPushButton { background:#FCEAEA; color:#C84444; border:none;"
            "  border-radius:10px; padding:0 18px; font-size:13px;"
            "  font-weight:700; font-family:'Segoe UI'; }"
            "QPushButton:hover { background:#FAD7D7; }"
            "QPushButton:pressed { background:#F5BFBF; }"
            );
        break;
    case BtnStyle::Ghost:
        b->setStyleSheet(
            "QPushButton { background:transparent; color:#6B8478; border:2px solid #E3EFE9;"
            "  border-radius:10px; padding:0 18px; font-size:13px;"
            "  font-weight:600; font-family:'Segoe UI'; }"
            "QPushButton:hover { background:#F3F8F5; border-color:#B7E4CB; }"
            );
        break;
    }
    return b;
}

// ═══════════════════════════════════════════════════════════════════════════
//  DonutChart
// ═══════════════════════════════════════════════════════════════════════════
class DonutChart : public QWidget {
    QVector<QPair<QString,double>> slices;
    QVector<QColor> colors = {
        {"#1E9E6B"},{"#3457D5"},{"#E8A93B"},{"#8B5CF6"},{"#E25C5C"},{"#2BAFC4"}
    };
public:
    explicit DonutChart(QWidget* p = nullptr) : QWidget(p) { setMinimumSize(180,180); }
    void setData(const QVector<QPair<QString,double>>& d) { slices = d; update(); }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        int W = width(), H = height();
        int sz = qMin(W, H) - 16;
        QRectF r((W - sz) / 2.0, (H - sz) / 2.0, sz, sz);

        double total = 0;
        for (auto& s : slices) total += s.second;
        if (total <= 0) {
            p.setPen(QPen(QColor("#E3EFE9"), 14));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(r.adjusted(7,7,-7,-7));
            p.setPen(QColor("#9FB6AB"));
            p.setFont(QFont("Segoe UI", 9));
            p.drawText(r, Qt::AlignCenter, "No data");
            return;
        }
        double angle = -90.0 * 16;
        for (int i = 0; i < slices.size(); ++i) {
            double span = (slices[i].second / total) * 360.0 * 16;
            p.setPen(QPen(colors[i % colors.size()], sz*0.26, Qt::SolidLine, Qt::FlatCap));
            p.setBrush(Qt::NoBrush);
            p.drawArc(r.adjusted(sz*0.13,sz*0.13,-sz*0.13,-sz*0.13), (int)angle, (int)span);
            angle += span;
        }
        p.setPen(QColor("#16261F"));
        p.setFont(QFont("Segoe UI", 13, QFont::Bold));
        p.drawText(r, Qt::AlignCenter, QString::number((int)total));
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  BarChart — smooth line/area trend style
// ═══════════════════════════════════════════════════════════════════════════
class BarChart : public QWidget {
    QVector<QPair<QString,double>> bars;
public:
    explicit BarChart(QWidget* p = nullptr) : QWidget(p) { setMinimumHeight(160); }
    void setData(const QVector<QPair<QString,double>>& d) { bars = d; update(); }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        int W = width(), H = height();
        if (bars.isEmpty()) {
            p.setPen(QColor("#9FB6AB"));
            p.setFont(QFont("Segoe UI", 9));
            p.drawText(rect(), Qt::AlignCenter, "No data");
            return;
        }
        double maxVal = 1;
        for (auto& b : bars) maxVal = qMax(maxVal, b.second);

        int topY = 14, botY = H - 28;
        int chartH = botY - topY;
        int n = bars.size();
        int leftPad = 16, rightPad = 16;
        double stepX = n > 1 ? double(W - leftPad - rightPad) / (n - 1) : 0;

        p.setPen(QPen(QColor("#EFF6F2"), 1));
        for (int i = 0; i <= 3; ++i) {
            int gy = topY + (int)(chartH * i / 3.0);
            p.drawLine(leftPad, gy, W - rightPad, gy);
        }

        QVector<QPointF> pts;
        for (int i = 0; i < n; ++i) {
            double x = leftPad + stepX * i;
            double y = botY - (bars[i].second / maxVal) * chartH;
            pts << QPointF(x, y);
        }

        QPainterPath area;
        area.moveTo(pts.first().x(), botY);
        for (auto& pt : pts) area.lineTo(pt);
        area.lineTo(pts.last().x(), botY);
        area.closeSubpath();
        QLinearGradient grad(0, topY, 0, botY);
        grad.setColorAt(0, QColor(30,158,107,70));
        grad.setColorAt(1, QColor(30,158,107,4));
        p.setPen(Qt::NoPen);
        p.setBrush(grad);
        p.drawPath(area);

        QPainterPath line;
        line.moveTo(pts.first());
        for (int i = 1; i < pts.size(); ++i) line.lineTo(pts[i]);
        p.setPen(QPen(QColor("#1E9E6B"), 2.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(Qt::NoBrush);
        p.drawPath(line);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#1E9E6B"));
        for (auto& pt : pts) p.drawEllipse(pt, 3.4, 3.4);
        p.setBrush(Qt::white);
        for (auto& pt : pts) p.drawEllipse(pt, 1.6, 1.6);

        p.setPen(QColor("#9FB6AB"));
        p.setFont(QFont("Segoe UI", 8));
        for (int i = 0; i < n; ++i) {
            p.drawText(QRectF(pts[i].x()-24, botY+6, 48, 16),
                       Qt::AlignCenter, bars[i].first.left(8));
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  Banner — soft illustrated header card used at top of every page
// ═══════════════════════════════════════════════════════════════════════════
QWidget* MainWindow::makeBanner(const QString& svgRes, const QString& title,
                                 const QString& subtitle) {
    auto* w = new QWidget;
    w->setMinimumHeight(132);
    w->setMaximumHeight(150);
    w->setStyleSheet(
        "QWidget { background: qlineargradient(x1:0,y1:0,x2:1,y2:0.3,"
        "  stop:0 #E6F6EE, stop:1 #F3FBF7); border-radius:18px; }"
        );
    auto* l = new QHBoxLayout(w);
    l->setContentsMargins(28, 20, 0, 0);
    l->setSpacing(0);

    auto* tv = new QVBoxLayout;
    tv->setSpacing(8);
    tv->addStretch();
    auto* tl = new QLabel(title);
    tl->setStyleSheet(
        "color:#0F6B47; font-size:21px; font-weight:800; font-family:'Segoe UI';"
        " background:transparent;"
        );
    tv->addWidget(tl);
    auto* sl = new QLabel(subtitle);
    sl->setStyleSheet(
        "color:#4D7363; font-size:12px; font-family:'Segoe UI'; background:transparent;"
        );
    sl->setWordWrap(true);
    sl->setMaximumWidth(360);
    tv->addWidget(sl);
    tv->addStretch();
    l->addLayout(tv, 1);

    auto* ic = new QLabel;
    ic->setStyleSheet("background:transparent;");
    QFile svgFile(svgRes);
    if (svgFile.open(QIODevice::ReadOnly)) {
        QByteArray data = svgFile.readAll();
        QSvgRenderer ren(data);
        QPixmap pm(300, 150);
        pm.fill(Qt::transparent);
        QPainter painter(&pm);
        painter.setRenderHint(QPainter::Antialiasing);
        ren.render(&painter);
        ic->setPixmap(pm);
    }
    l->addWidget(ic);
    return w;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Stat card — flat colored icon badge + big number (matches reference)
// ═══════════════════════════════════════════════════════════════════════════
QWidget* MainWindow::makeStatCard(const QString& value, const QString& label,
                                   const QString& accent, const QString& bgTint,
                                   const QString& iconPath) {
    auto* card = new QWidget;
    card->setMinimumWidth(150);
    card->setMinimumHeight(98);
    card->setStyleSheet("QWidget{background:white;border-radius:16px;}");
    card->setGraphicsEffect(mkShadow());

    auto* l = new QHBoxLayout(card);
    l->setContentsMargins(18, 16, 18, 16);
    l->setSpacing(14);

    auto* badge = new QLabel;
    badge->setFixedSize(46, 46);
    badge->setAlignment(Qt::AlignCenter);
    badge->setStyleSheet(QString(
        "background:%1; border-radius:13px;").arg(bgTint));
    badge->setPixmap(renderSvgTinted(iconPath, QColor(accent), 22));
    l->addWidget(badge);

    auto* tv = new QVBoxLayout;
    tv->setSpacing(2);
    auto* vl = new QLabel(value);
    vl->setObjectName("statValue");
    vl->setStyleSheet(
        "font-size:24px; font-weight:800; color:#16261F;"
        " font-family:'Segoe UI'; background:transparent;");
    auto* ll = new QLabel(label);
    ll->setStyleSheet(
        "font-size:11px; color:#6B8478; font-family:'Segoe UI'; background:transparent;"
        );
    tv->addWidget(vl);
    tv->addWidget(ll);
    l->addLayout(tv, 1);
    return card;
}

// ═══════════════════════════════════════════════════════════════════════════
//  MainWindow constructor / destructor
// ═══════════════════════════════════════════════════════════════════════════
MainWindow::MainWindow(User* user, QWidget* parent)
    : QMainWindow(parent), currentUser(user)
{
    setWindowTitle("UDRMS — University Dormitory & Restaurant System");
    setMinimumSize(1240, 780);
    applyGlobalStyle();

    auto* central = new QWidget(this);
    central->setObjectName("centralWidget");
    setCentralWidget(central);

    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    buildSidebar();
    root->addWidget(sidebar);

    auto* rightCol = new QVBoxLayout;
    rightCol->setContentsMargins(28, 22, 28, 22);
    rightCol->setSpacing(18);
    buildTopBar(rightCol);

    stack = new QStackedWidget;
    rightCol->addWidget(stack, 1);

    auto* rightWrap = new QWidget;
    rightWrap->setLayout(rightCol);
    root->addWidget(rightWrap, 1);

    if (currentUser->isStudent()) {
        stack->addWidget(pageStudentPortal());
    } else {
        stack->addWidget(pageStudents());
        stack->addWidget(pageRooms());
        stack->addWidget(pageRestaurant());
        stack->addWidget(pageReports());
        if (currentUser->isAdmin())
            stack->addWidget(pageUsers());
    }

    if (!navButtons.isEmpty()) navButtons.first()->click();
}

MainWindow::~MainWindow() { delete currentUser; }
void MainWindow::applyGlobalStyle() {
    setStyleSheet(QString(
        "QMainWindow, QWidget#centralWidget { background:%1; }"
        "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit {"
        "  border:2px solid %2; border-radius:10px;"
        "  padding:8px 12px; font-size:13px; font-family:'Segoe UI';"
        "  background:white; color:%3;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border-color:%4; }"
        "QComboBox::drop-down, QDateEdit::drop-down { border:none; padding-right:8px; }"
        "QComboBox QAbstractItemView {"
        "  border:2px solid %2; border-radius:8px; background:white;"
        "  selection-background-color:%5; color:%3; padding:4px;"
        "}"
        "QMessageBox, QInputDialog { background:%1; font-family:'Segoe UI'; }"
        "QMessageBox QLabel, QInputDialog QLabel { color:%3; }"
        "QMessageBox QPushButton, QInputDialog QPushButton {"
        "  background:%4; color:white; border:none; border-radius:8px;"
        "  padding:7px 18px; font-weight:700; min-width:70px;"
        "}"
        "QMessageBox QPushButton:hover { background:%6; }"
        "QGroupBox { border:2px solid %2; border-radius:12px; margin-top:8px;"
        "  font-family:'Segoe UI'; font-size:11px; font-weight:700; color:%4;"
        "  padding-top:6px; }"
        "QGroupBox::title { subcontrol-origin:margin; left:12px; padding:0 6px; }"
        ).arg(C::BG, C::BDR, C::TXT, C::PRIMARY, C::PRIMARY_LT2, C::PRIMARY_DK));
}

void MainWindow::fillDormCombo(QComboBox* combo) {
    combo->clear();
    for (const Dormitory& d : DatabaseManager::getInstance()->getDormitories())
        combo->addItem(d.getCode() + " — " + d.getDisplayName(), d.getId());
}

// ═══════════════════════════════════════════════════════════════════════════
//  SIDEBAR
// ═══════════════════════════════════════════════════════════════════════════
void MainWindow::buildSidebar() {
    sidebar = new QWidget;
    sidebar->setFixedWidth(232);
    sidebar->setStyleSheet(QString(
        "QWidget { background:%1; border-right:1px solid %2; }").arg(C::SIDEBAR, C::BDR));

    auto* l = new QVBoxLayout(sidebar);
    l->setContentsMargins(20, 24, 20, 20);
    l->setSpacing(4);

    // Logo row
    auto* logoRow = new QHBoxLayout;
    logoRow->setSpacing(10);
    auto* logoIc = new QLabel;
    logoIc->setFixedSize(34, 34);
    QFile logoFile(":/icons/logo.svg");
    if (logoFile.open(QIODevice::ReadOnly)) {
        QSvgRenderer ren(logoFile.readAll());
        QPixmap pm(34, 34); pm.fill(Qt::transparent);
        QPainter pt(&pm); pt.setRenderHint(QPainter::Antialiasing);
        ren.render(&pt);
        logoIc->setPixmap(pm);
    }
    logoRow->addWidget(logoIc);
    auto* logoTxt = new QLabel("UDRMS");
    logoTxt->setStyleSheet(
        "color:#16261F; font-size:17px; font-weight:800; font-family:'Segoe UI';"
        );
    logoRow->addWidget(logoTxt);
    logoRow->addStretch();
    l->addLayout(logoRow);

    l->addSpacing(28);

    // Nav button factory
    auto* navGroup = new QButtonGroup(this);
    navGroup->setExclusive(true);

    auto addNav = [&](const QString& iconRes, const QString& text,
                      const QString& subtitle, int pageIndex) {
        auto* btn = new QPushButton;
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(46);
        btn->setStyleSheet(
            "QPushButton {"
            "  text-align:left; padding-left:14px; border:none; border-radius:11px;"
            "  background:transparent; color:#52685D;"
            "  font-size:13.5px; font-family:'Segoe UI'; font-weight:600;"
            "}"
            "QPushButton:hover:!checked { background:#F3F8F5; }"
            "QPushButton:checked { background:#E6F6EE; color:#0F6B47; font-weight:700; }"
            );
        btn->setIcon(svgIcon(iconRes, QColor(C::SUBT)));
        btn->setIconSize(QSize(19, 19));
        btn->setText("  " + text);

        connect(btn, &QPushButton::toggled, this, [btn, iconRes](bool checked) {
            btn->setIcon(QIcon(renderSvgTinted(
                iconRes, QColor(checked ? C::PRIMARY_DK : C::SUBT), 22)));
        });
        connect(btn, &QPushButton::clicked, this, [this, pageIndex, text, subtitle]() {
            switchPage(pageIndex, text, subtitle);
        });

        navGroup->addButton(btn);
        navButtons.append(btn);
        l->addWidget(btn);
    };

    if (currentUser->isStudent()) {
        addNav(":/icons/ic_portal.svg", "My Portal",
               "View your dormitory assignment and reserve your meals", 0);
    } else {
        int idx = 0;
        addNav(":/icons/ic_students.svg", "Students",
               "Register students and manage dormitory assignments", idx++);
        addNav(":/icons/ic_rooms.svg", "Rooms",
               "Manage dormitories and room allocations", idx++);
        addNav(":/icons/ic_restaurant.svg", "Restaurant",
               "Manage cafeteria menus and meal reservations", idx++);
        addNav(":/icons/ic_reports.svg", "Reports",
               "Live occupancy statistics and reservation summaries", idx++);
        if (currentUser->isAdmin())
            addNav(":/icons/ic_users.svg", "Users",
                   "Manage staff, manager and admin accounts", idx++);
    }

    l->addStretch();

    auto* div = new QFrame; div->setFrameShape(QFrame::HLine);
    div->setStyleSheet(QString("color:%1; background:%1; max-height:1px;").arg(C::BDR));
    l->addWidget(div);
    l->addSpacing(12);

    auto* userCard = new QWidget;
    userCard->setStyleSheet("QWidget{background:#F3F8F5; border-radius:12px;}");
    auto* ucl = new QHBoxLayout(userCard);
    ucl->setContentsMargins(10, 8, 10, 8);
    ucl->setSpacing(10);

    auto* avatar = new QLabel(currentUser->getUsername().left(1).toUpper());
    avatar->setFixedSize(34, 34);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(
        "background:#1E9E6B; color:white; border-radius:17px;"
        " font-weight:800; font-family:'Segoe UI'; font-size:13px;"
        );
    ucl->addWidget(avatar);

    auto* uv = new QVBoxLayout;
    uv->setSpacing(0);
    auto* uName = new QLabel(currentUser->getUsername());
    uName->setStyleSheet(
        "color:#16261F; font-size:12px; font-weight:700; font-family:'Segoe UI';"
        " background:transparent;"
        );
    auto* uRole = new QLabel(currentUser->getRole().toUpper());
    uRole->setStyleSheet(
        "color:#6B8478; font-size:9.5px; font-family:'Segoe UI'; background:transparent;"
        );
    uv->addWidget(uName);
    uv->addWidget(uRole);
    ucl->addLayout(uv, 1);

    auto* btnLogout = new QPushButton;
    btnLogout->setFixedSize(30, 30);
    btnLogout->setCursor(Qt::PointingHandCursor);
    btnLogout->setIcon(svgIcon(":/icons/ic_logout.svg", QColor(C::RED)));
    btnLogout->setIconSize(QSize(16,16));
    btnLogout->setStyleSheet(
        "QPushButton { background:transparent; border:none; border-radius:8px; }"
        "QPushButton:hover { background:#FCEAEA; }"
        );
    btnLogout->setToolTip("Logout");
    connect(btnLogout, &QPushButton::clicked, this, &MainWindow::onLogout);
    ucl->addWidget(btnLogout);

    l->addWidget(userCard);
}

void MainWindow::switchPage(int index, const QString& title, const QString& sub) {
    if (stack) stack->setCurrentIndex(index);
    if (topBarTitle) topBarTitle->setText(title);
    if (topBarSub)   topBarSub->setText(sub);
}

// ═══════════════════════════════════════════════════════════════════════════
//  TOP BAR
// ═══════════════════════════════════════════════════════════════════════════
void MainWindow::buildTopBar(QVBoxLayout* rightCol) {
    auto* row = new QHBoxLayout;
    row->setSpacing(14);

    auto* tv = new QVBoxLayout;
    tv->setSpacing(2);
    topBarTitle = new QLabel("Students");
    topBarTitle->setStyleSheet(
        "color:#16261F; font-size:22px; font-weight:800; font-family:'Segoe UI';"
        );
    topBarSub = new QLabel("Manage students and their dormitory assignments");
    topBarSub->setStyleSheet(
        "color:#6B8478; font-size:12.5px; font-family:'Segoe UI';"
        );
    tv->addWidget(topBarTitle);
    tv->addWidget(topBarSub);
    row->addLayout(tv);
    row->addStretch();

    auto* bell = new QPushButton;
    bell->setFixedSize(40, 40);
    bell->setCursor(Qt::PointingHandCursor);
    bell->setIcon(svgIcon(":/icons/ic_bell.svg", QColor(C::SUBT)));
    bell->setIconSize(QSize(18,18));
    bell->setStyleSheet(
        "QPushButton { background:white; border:none; border-radius:12px; }"
        "QPushButton:hover { background:#F3F8F5; }"
        );
    bell->setGraphicsEffect(mkShadow(10,2,14));
    row->addWidget(bell);

    rightCol->addLayout(row);
}

// ═══════════════════════════════════════════════════════════════════════════
//  STUDENTS PAGE
// ═══════════════════════════════════════════════════════════════════════════
QWidget* MainWindow::pageStudents() {
    auto* page = new QWidget;
    auto* l = new QVBoxLayout(page);
    l->setContentsMargins(0,0,0,0);
    l->setSpacing(16);

    l->addWidget(makeBanner(":/icons/banner_students.svg", "Students Management",
        "Register new students, manage their information, and assign them to dormitory rooms."));

    auto* toolbar = new QWidget;
    toolbar->setStyleSheet("QWidget{background:white;border-radius:14px;}");
    toolbar->setGraphicsEffect(mkShadow());
    auto* tbl = new QHBoxLayout(toolbar);
    tbl->setContentsMargins(16, 12, 16, 12);
    tbl->setSpacing(12);

    auto* searchBox = new QWidget;
    searchBox->setStyleSheet(
        "QWidget{background:#F3F8F5; border-radius:10px;}"
        );
    auto* sbl = new QHBoxLayout(searchBox);
    sbl->setContentsMargins(12, 0, 12, 0);
    sbl->setSpacing(8);
    auto* searchIco = new QLabel;
    searchIco->setPixmap(renderSvgTinted(":/icons/ic_search.svg", QColor(C::MUTED), 16));
    searchIco->setStyleSheet("background:transparent;");
    studentSearch = new QLineEdit;
    studentSearch->setPlaceholderText("Search by name or student ID…");
    studentSearch->setStyleSheet(
        "border:none; background:transparent; font-size:13px;"
        " color:#16261F; padding:10px 0;"
        );
    sbl->addWidget(searchIco);
    sbl->addWidget(studentSearch, 1);
    tbl->addWidget(searchBox, 1);

    auto* bAdd = makeBtn("  Add Student", BtnStyle::Primary);
    bAdd->setIcon(svgIcon(":/icons/ic_plus.svg", Qt::white));
    bAdd->setMinimumWidth(150);
    tbl->addWidget(bAdd);
    l->addWidget(toolbar);
connect(studentSearch, &QLineEdit::textChanged, this, &MainWindow::refreshStudentTable);
    connect(bAdd, &QPushButton::clicked, this, &MainWindow::onAddStudent);

    studentTable = new QTableWidget(0, 6);
    studentTable->setHorizontalHeaderLabels(
        {"Student ID", "Full Name", "Year", "Gender", "Room Status", "Actions"});
    styleTable(studentTable);
    studentTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    studentTable->setColumnWidth(5, 230);
    l->addWidget(wrapCard(studentTable, 8), 1);

    refreshStudentTable();
    return page;
}

void MainWindow::refreshStudentTable() {
    if (!studentTable) return;
    QString f = studentSearch ? studentSearch->text() : "";
    auto students = DatabaseManager::getInstance()->getStudents(f);
    studentTable->setRowCount(0);
    for (const Student& s : students) {
        int row = studentTable->rowCount();
        studentTable->insertRow(row);
        studentTable->setRowHeight(row, 56);

        auto* idItem = new QTableWidgetItem(QString::number(s.getId()));
        idItem->setTextAlignment(Qt::AlignCenter);
        idItem->setForeground(QColor(C::PRIMARY));
        QFont idFont = idItem->font(); idFont.setBold(true);
        idItem->setFont(idFont);
        studentTable->setItem(row, 0, idItem);

        studentTable->setItem(row, 1, new QTableWidgetItem(s.getName()));

        auto* yr = new QTableWidgetItem(QString("Year %1").arg(s.getAcademicYear()));
        yr->setTextAlignment(Qt::AlignCenter);
        studentTable->setItem(row, 2, yr);

        auto* gen = new QTableWidgetItem(s.getGender() == "M" ? "Male" : "Female");
        gen->setTextAlignment(Qt::AlignCenter);
        gen->setForeground(QColor(s.getGender() == "M" ? C::NAVY : C::PURPLE));
        studentTable->setItem(row, 3, gen);

        bool has = DatabaseManager::getInstance()->studentHasRoom(s.getId());
        auto* pillWrap = new QWidget;
        auto* pillL = new QHBoxLayout(pillWrap);
        pillL->setContentsMargins(0,0,0,0);
        pillL->setAlignment(Qt::AlignCenter);
        auto* pill = new QLabel(has ? "Assigned" : "Unassigned");
        pill->setAlignment(Qt::AlignCenter);
        pill->setFixedHeight(26);
        pill->setMinimumWidth(86);
        pill->setStyleSheet(QString(
            "background:%1; color:%2; border-radius:13px; font-size:11px;"
            " font-weight:700; font-family:'Segoe UI'; padding:0 10px;")
            .arg(has ? C::PRIMARY_LT : C::AMBER_LT)
            .arg(has ? C::PRIMARY_DK : C::AMBER));
        pillL->addWidget(pill);
        studentTable->setCellWidget(row, 4, pillWrap);

        auto* actWrap = new QWidget;
        auto* actL = new QHBoxLayout(actWrap);
        actL->setContentsMargins(4,4,4,4);
        actL->setSpacing(6);
        actL->setAlignment(Qt::AlignCenter);

        auto mkIconBtn = [](const QString& iconRes, const QColor& tint,
                            const QString& bg, const QString& tooltip) {
            auto* b = new QPushButton;
            b->setFixedSize(30, 30);
            b->setCursor(Qt::PointingHandCursor);
            b->setIcon(QIcon(renderSvgTinted(iconRes, tint, 16)));
            b->setIconSize(QSize(15,15));
            b->setToolTip(tooltip);
            b->setStyleSheet(QString(
                "QPushButton{background:%1;border:none;border-radius:8px;}"
                "QPushButton:hover{background:%1;}").arg(bg));
            return b;
        };

        auto* bEdit = mkIconBtn(":/icons/ic_edit.svg",  QColor(C::NAVY), C::NAVY_LT, "Edit");
        auto* bDel  = mkIconBtn(":/icons/ic_trash.svg", QColor(C::RED),  C::RED_LT,  "Delete");
        auto* bAsgn = mkIconBtn(":/icons/ic_key.svg",   QColor(C::PRIMARY_DK), C::PRIMARY_LT, "Assign Room");
        auto* bVac  = mkIconBtn(":/icons/ic_undo.svg",  QColor(C::AMBER), C::AMBER_LT, "Vacate Room");

        int sid = s.getId();
        auto selectStudentRow = [this](int studentId) {
            for (int r = 0; r < studentTable->rowCount(); ++r) {
                if (studentTable->item(r, 0)->text().toInt() == studentId) {
                    studentTable->selectRow(r);
                    return;
                }
            }
        };
        connect(bEdit, &QPushButton::clicked, this, [this, sid, selectStudentRow]() {
            selectStudentRow(sid); onEditStudent();
        });
        connect(bDel, &QPushButton::clicked, this, [this, sid, selectStudentRow]() {
            selectStudentRow(sid); onDeleteStudent();
        });
        connect(bAsgn, &QPushButton::clicked, this, [this, sid, selectStudentRow]() {
            selectStudentRow(sid); onAssignRoom();
        });
        connect(bVac, &QPushButton::clicked, this, [this, sid, selectStudentRow]() {
            selectStudentRow(sid); onVacateStudent();
        });

        actL->addWidget(bEdit);
        actL->addWidget(bDel);
        actL->addWidget(bAsgn);
        actL->addWidget(bVac);
        studentTable->setCellWidget(row, 5, actWrap);
    }
}

void MainWindow::onAddStudent() {
    StudentDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    Student s = dlg.getStudent();
    if (!DatabaseManager::getInstance()->addStudent(
            s, dlg.getUsername(), dlg.getPassword())) {
        QMessageBox::warning(this, "Error",
            "Could not add student.\n"
            "Student ID or username may already exist, "
            "or password does not meet requirements (min. 8 chars).");
    }
    refreshStudentTable();
}

void MainWindow::onEditStudent() {
    int row = studentTable->currentRow();
    if (row < 0) { QMessageBox::information(this,"Edit","Select a student first."); return; }
    int sid = studentTable->item(row, 0)->text().toInt();
    auto list = DatabaseManager::getInstance()->getStudents(QString::number(sid));
    if (list.isEmpty()) return;
    StudentDialog dlg(this, &list[0]);
    if (dlg.exec() == QDialog::Accepted) {
        DatabaseManager::getInstance()->updateStudent(dlg.getStudent());
        refreshStudentTable();
    }
}

void MainWindow::onDeleteStudent() {
    int row = studentTable->currentRow();
    if (row < 0) return;
    int sid = studentTable->item(row,0)->text().toInt();
    QString name = studentTable->item(row,1)->text();
    if (QMessageBox::question(this,"Confirm Delete",
            QString("Delete student %1 (ID %2)?\n\n"
                    "This will also remove their login account and all records.")
                .arg(name).arg(sid)) == QMessageBox::Yes) {
        DatabaseManager::getInstance()->deleteStudent(sid);
        refreshStudentTable();
    }
}

void MainWindow::onAssignRoom() {
    int row = studentTable->currentRow();
    if (row < 0) { QMessageBox::information(this,"Assign","Select a student first."); return; }
    int sid = studentTable->item(row,0)->text().toInt();
    if (DatabaseManager::getInstance()->studentHasRoom(sid)) {
        QMessageBox::warning(this,"Assign Room","Student already has a room. Vacate first.");
        return;
    }
    auto dorms = DatabaseManager::getInstance()->getDormitories();
    QStringList choices;
    QVector<int> roomIds;
    for (const Dormitory& d : dorms) {
        auto rooms = DatabaseManager::getInstance()->getRooms(d.getId());
        for (const Room& r : rooms) {
            if (r.isAvailable()) {
                QString dispId = QString("%1%2-%3")
                    .arg(d.getCode()).arg(r.getFloor()).arg(r.getRoomNumber());
                choices << QString("%1  |  %2  (%3 beds free)  —  %4")
                    .arg(dispId).arg(r.getRoomType())
                    .arg(r.getAvailableBeds()).arg(d.getDisplayName());
                roomIds << r.getId();
            }
        }
    }
    if (choices.isEmpty()) {
        QMessageBox::information(this,"No Rooms","No available rooms found."); return;
    }
    bool ok;
    QString sel = QInputDialog::getItem(this,"Assign Room",
        "Select an available room:", choices, 0, false, &ok);
    if (!ok) return;
    int idx = choices.indexOf(sel);
    if (idx < 0 || idx >= roomIds.size()) return;
    if (!DatabaseManager::getInstance()->assignStudent(sid, roomIds[idx]))
        QMessageBox::warning(this,"Failed","Could not assign room.");
    else {
        QMessageBox::information(this,"Assigned","Student assigned successfully.");
        refreshStudentTable(); refreshRoomTable();
    }
}

void MainWindow::onVacateStudent() {
    int row = studentTable->currentRow();
    if (row < 0) return;
    int sid = studentTable->item(row,0)->text().toInt();
    if (!DatabaseManager::getInstance()->studentHasRoom(sid)) {
        QMessageBox::information(this,"Vacate","Student has no active room."); return;
    }
    if (QMessageBox::question(this,"Vacate Room","Remove student from their room?")
        == QMessageBox::Yes) {
        DatabaseManager::getInstance()->vacateStudent(sid);
        refreshStudentTable(); refreshRoomTable();
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  ROOMS PAGE
// ═══════════════════════════════════════════════════════════════════════════
QWidget* MainWindow::pageRooms() {
    auto* page = new QWidget;
    auto* l = new QVBoxLayout(page);
    l->setContentsMargins(0,0,0,0);
    l->setSpacing(16);

    l->addWidget(makeBanner(":/icons/banner_rooms.svg", "Room Management",
        "Manage dormitories and rooms — Room ID format is DormCode+Floor-Number, e.g. A2-5."));

    auto* toolbar = new QWidget;
    toolbar->setStyleSheet("QWidget{background:white;border-radius:14px;}");
    toolbar->setGraphicsEffect(mkShadow());
    auto* tbl = new QHBoxLayout(toolbar);
    tbl->setContentsMargins(16, 12, 16, 12);
    tbl->setSpacing(10);

    auto* dormLbl = new QLabel("Dormitory:");
    dormLbl->setStyleSheet("color:#6B8478; font-size:12px; font-weight:600; font-family:'Segoe UI';");
    tbl->addWidget(dormLbl);

    dormSelector = new QComboBox;
    dormSelector->setFixedHeight(40);
    dormSelector->setMinimumWidth(260);
    fillDormCombo(dormSelector);
    tbl->addWidget(dormSelector);
    tbl->addStretch();

    auto* bAddDorm = makeBtn("  Add Dormitory", BtnStyle::Secondary);
    bAddDorm->setIcon(svgIcon(":/icons/ic_plus.svg", QColor(C::PRIMARY_DK)));
    auto* bEditDorm = makeBtn("  Edit Dorm", BtnStyle::Ghost);
    bEditDorm->setIcon(svgIcon(":/icons/ic_edit.svg", QColor(C::SUBT)));
    auto* bAddRoom = makeBtn("  Add Room", BtnStyle::Primary);
    bAddRoom->setIcon(svgIcon(":/icons/ic_plus.svg", Qt::white));
    tbl->addWidget(bAddDorm);
    tbl->addWidget(bEditDorm);
    tbl->addWidget(bAddRoom);
    l->addWidget(toolbar);

    roomTable = new QTableWidget(0, 6);
    roomTable->setHorizontalHeaderLabels(
        {"Room ID", "Floor", "Number", "Type", "Capacity", "Status"});
    styleTable(roomTable);
    l->addWidget(wrapCard(roomTable, 8), 1);

    auto* bottomRow = new QHBoxLayout;
    auto* bDelRoom = makeBtn("  Delete Selected Room", BtnStyle::Danger);
    bDelRoom->setIcon(svgIcon(":/icons/ic_trash.svg", QColor(C::RED)));
    bottomRow->addWidget(bDelRoom);
    bottomRow->addStretch();
    l->addLayout(bottomRow);

    connect(dormSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::refreshRoomTable);
    connect(bAddDorm,  &QPushButton::clicked, this, &MainWindow::onAddDormitory);
    connect(bEditDorm, &QPushButton::clicked, this, &MainWindow::onEditDormitory);
    connect(bAddRoom,  &QPushButton::clicked, this, &MainWindow::onAddRoom);
    connect(bDelRoom,  &QPushButton::clicked, this, &MainWindow::onDeleteRoom);

    refreshRoomTable();
    return page;
}

void MainWindow::refreshRoomTable() {
    if (!dormSelector || dormSelector->count() == 0) { if(roomTable) roomTable->setRowCount(0); return; }
    int dormId = dormSelector->currentData().toInt();
    QString dormCode;
    for (const Dormitory& d : DatabaseManager::getInstance()->getDormitories())
        if (d.getId() == dormId) { dormCode = d.getCode(); break; }

    auto rooms = DatabaseManager::getInstance()->getRooms(dormId);
    roomTable->setRowCount(0);
    for (const Room& r : rooms) {
        int row = roomTable->rowCount();
        roomTable->insertRow(row);
        roomTable->setRowHeight(row, 50);

        QString dispId = QString("%1%2-%3").arg(dormCode).arg(r.getFloor()).arg(r.getRoomNumber());
        auto* idItem = new QTableWidgetItem(dispId);
        idItem->setTextAlignment(Qt::AlignCenter);
        idItem->setForeground(QColor(C::PRIMARY_DK));
QFont bf = idItem->font(); bf.setBold(true); idItem->setFont(bf);
        idItem->setData(Qt::UserRole, r.getId());
        roomTable->setItem(row, 0, idItem);

        auto* flItem = new QTableWidgetItem(QString("Floor %1").arg(r.getFloor()));
        flItem->setTextAlignment(Qt::AlignCenter);
        roomTable->setItem(row, 1, flItem);

        auto* numItem = new QTableWidgetItem(QString::number(r.getRoomNumber()));
        numItem->setTextAlignment(Qt::AlignCenter);
        roomTable->setItem(row, 2, numItem);

        roomTable->setItem(row, 3, new QTableWidgetItem(r.getRoomType()));

        auto* cap = new QTableWidgetItem(QString("%1 beds").arg(r.getCapacity()));
        cap->setTextAlignment(Qt::AlignCenter);
        roomTable->setItem(row, 4, cap);

        QString txt; QString bg; QString fg;
        if (r.isFull()) {
            txt = QString("Full · %1/%1").arg(r.getCapacity());
            bg = C::RED_LT; fg = C::RED;
        } else if (r.getCurrentOccupancy() > 0) {
            txt = QString("Partial · %1/%2").arg(r.getCurrentOccupancy()).arg(r.getCapacity());
            bg = C::AMBER_LT; fg = C::AMBER;
        } else {
            txt = QString("Available · 0/%1").arg(r.getCapacity());
            bg = C::PRIMARY_LT; fg = C::PRIMARY_DK;
        }
        auto* pillWrap = new QWidget;
        auto* pillL = new QHBoxLayout(pillWrap);
        pillL->setContentsMargins(0,0,0,0);
        pillL->setAlignment(Qt::AlignCenter);
        auto* pill = new QLabel(txt);
        pill->setAlignment(Qt::AlignCenter);
        pill->setFixedHeight(26);
        pill->setStyleSheet(QString(
            "background:%1; color:%2; border-radius:13px; font-size:11px;"
            " font-weight:700; font-family:'Segoe UI'; padding:0 12px;").arg(bg, fg));
        pillL->addWidget(pill);
        roomTable->setCellWidget(row, 5, pillWrap);
    }
}

void MainWindow::onAddDormitory() {
    DormDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    auto d = dlg.getData();
    if (!DatabaseManager::getInstance()->addDormitory(
            d.code, d.displayName, d.address, d.capacity, d.restaurantName)) {
        QMessageBox::warning(this,"Error","Could not add dormitory. Code may already exist.");
        return;
    }
    fillDormCombo(dormSelector);
    if (restDormSelector) fillDormCombo(restDormSelector);
    refreshRoomTable();
}

void MainWindow::onEditDormitory() {
    if (!dormSelector || dormSelector->count() == 0) return;
    int dormId = dormSelector->currentData().toInt();
    DormData existing;
    for (const Dormitory& d : DatabaseManager::getInstance()->getDormitories()) {
        if (d.getId() == dormId) {
            existing.code        = d.getCode();
            existing.displayName = d.getDisplayName();
            existing.address     = d.getAddress();
            existing.capacity    = 0;
            existing.restaurantName = d.getRestaurant().getName();
            break;
        }
    }
    DormDialog dlg(this, &existing);
    if (dlg.exec() != QDialog::Accepted) return;
    auto nd = dlg.getData();
    if (!DatabaseManager::getInstance()->updateDormitory(
            dormId, nd.code, nd.displayName, nd.address, nd.restaurantName)) {
        QMessageBox::warning(this,"Error","Could not update dormitory.");
        return;
    }
    fillDormCombo(dormSelector);
    if (restDormSelector) fillDormCombo(restDormSelector);
    refreshRoomTable();
}

void MainWindow::onAddRoom() {
    if (!dormSelector || dormSelector->count() == 0) return;
    int dormId = dormSelector->currentData().toInt();
    RoomDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    auto [num, floor, cap, type] = dlg.getRoomData();
    if (!DatabaseManager::getInstance()->addRoom(dormId, num, floor, cap, type))
        QMessageBox::warning(this,"Error",
            "Could not add room.\nA room with that number on that floor may already exist.");
    else refreshRoomTable();
}

void MainWindow::onDeleteRoom() {
    int row = roomTable->currentRow();
    if (row < 0) { QMessageBox::information(this,"Delete","Select a room first."); return; }
    int roomId = roomTable->item(row, 0)->data(Qt::UserRole).toInt();
    auto* pillWidget = roomTable->cellWidget(row, 5);
    QString statusText = pillWidget ? pillWidget->findChild<QLabel*>()->text() : "";
    if (!statusText.contains("Available")) {
        QMessageBox::warning(this,"Cannot Delete",
            "Room has occupants. Vacate all students first."); return;
    }
    if (QMessageBox::question(this,"Confirm Delete","Delete this room?")
        == QMessageBox::Yes) {
        DatabaseManager::getInstance()->deleteRoom(roomId);
        refreshRoomTable();
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  RESTAURANT PAGE
// ═══════════════════════════════════════════════════════════════════════════
QWidget* MainWindow::pageRestaurant() {
    auto* page = new QWidget;
    auto* l = new QVBoxLayout(page);
    l->setContentsMargins(0,0,0,0);
    l->setSpacing(16);

    l->addWidget(makeBanner(":/icons/banner_restaurant.svg", "Restaurant Management",
        "Meals are subsidised for all students — manage the menu and view daily reservations."));

    auto* toolbar = new QWidget;
    toolbar->setStyleSheet("QWidget{background:white;border-radius:14px;}");
    toolbar->setGraphicsEffect(mkShadow());
    auto* tbl = new QHBoxLayout(toolbar);
    tbl->setContentsMargins(16, 12, 16, 12);
    tbl->setSpacing(10);
    auto* lbl = new QLabel("Cafeteria:");
    lbl->setStyleSheet("color:#6B8478; font-size:12px; font-weight:600; font-family:'Segoe UI';");
    tbl->addWidget(lbl);
    restDormSelector = new QComboBox;
    restDormSelector->setFixedHeight(40);
    restDormSelector->setMinimumWidth(260);
    fillDormCombo(restDormSelector);
    tbl->addWidget(restDormSelector);
    tbl->addStretch();
    l->addWidget(toolbar);

    auto* segWrap = new QWidget;
    auto* segL = new QHBoxLayout(segWrap);
    segL->setContentsMargins(0,0,0,0);
    segL->setSpacing(8);
    auto* segMenu  = new QPushButton("Menu Items");
    auto* segRes   = new QPushButton("Reservations");
    for (auto* b : {segMenu, segRes}) {
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        b->setFixedHeight(38);
        b->setStyleSheet(
            "QPushButton { background:white; color:#6B8478; border:none;"
            "  border-radius:10px; padding:0 18px; font-size:12.5px;"
            "  font-weight:700; font-family:'Segoe UI'; }"
            "QPushButton:checked { background:#1E9E6B; color:white; }"
            "QPushButton:hover:!checked { background:#E6F6EE; }"
            );
    }
    auto* segGroup = new QButtonGroup(this);
    segGroup->setExclusive(true);
    segGroup->addButton(segMenu);
    segGroup->addButton(segRes);
    segMenu->setChecked(true);
    segL->addWidget(segMenu);
    segL->addWidget(segRes);
    segL->addStretch();
    l->addWidget(segWrap);

    auto* innerStack = new QStackedWidget;

    auto* mt = new QWidget;
    auto* ml2 = new QVBoxLayout(mt);
    ml2->setContentsMargins(0,0,0,0);
    ml2->setSpacing(12);

    auto* mAddRow = new QHBoxLayout;
    mAddRow->addStretch();
    auto* bAddItem = makeBtn("  Add Dish", BtnStyle::Primary);
    bAddItem->setIcon(svgIcon(":/icons/ic_plus.svg", Qt::white));
    mAddRow->addWidget(bAddItem);
    ml2->addLayout(mAddRow);

    menuTable = new QTableWidget(0, 4);
    menuTable->setHorizontalHeaderLabels({"Meal Type", "Dish Name", "Photo", "Available"});
    styleTable(menuTable);
    menuTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    menuTable->setColumnWidth(2, 80);
    ml2->addWidget(wrapCard(menuTable, 8), 1);

    auto* mDelRow = new QHBoxLayout;
    auto* bDelItem = makeBtn("  Remove Selected", BtnStyle::Danger);
    bDelItem->setIcon(svgIcon(":/icons/ic_trash.svg", QColor(C::RED)));
    mDelRow->addWidget(bDelItem);
    mDelRow->addStretch();
    ml2->addLayout(mDelRow);
    innerStack->addWidget(mt);

    auto* pt = new QWidget;
    auto* pl = new QVBoxLayout(pt);
    pl->setContentsMargins(0,0,0,0);
    pl->setSpacing(12);

    auto* dateRow = new QHBoxLayout;
    auto* dateLbl = new QLabel("Show reservations for:");
    dateLbl->setStyleSheet("color:#6B8478; font-size:12px; font-weight:600; font-family:'Segoe UI';");
    dateRow->addWidget(dateLbl);
    reservDateEdit = new QDateEdit(QDate::currentDate());
    reservDateEdit->setCalendarPopup(true);
    reservDateEdit->setFixedHeight(38);
    reservDateEdit->setFixedWidth(150);
    dateRow->addWidget(reservDateEdit);
    auto* bRefRes = makeBtn("  Refresh", BtnStyle::Secondary);
    bRefRes->setIcon(svgIcon(":/icons/ic_refresh.svg", QColor(C::PRIMARY_DK)));
    bRefRes->setFixedHeight(38);
    dateRow->addWidget(bRefRes);
    dateRow->addStretch();
    pl->addLayout(dateRow);

    reservTable = new QTableWidget(0, 5);
    reservTable->setHorizontalHeaderLabels(
        {"Student Name", "Student ID", "Dish", "Meal Type", "Reserved At"});
    styleTable(reservTable);
    pl->addWidget(wrapCard(reservTable, 8), 1);
    innerStack->addWidget(pt);

    l->addWidget(innerStack, 1);

    connect(segMenu, &QPushButton::clicked, this, [innerStack]() { innerStack->setCurrentIndex(0); });
    connect(segRes,  &QPushButton::clicked, this, [innerStack]() { innerStack->setCurrentIndex(1); });

    connect(restDormSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::refreshMenuTable);
    connect(bAddItem, &QPushButton::clicked, this, &MainWindow::onAddMenuItem);
    connect(bDelItem, &QPushButton::clicked, this, &MainWindow::onDeleteMenuItem);
    connect(bRefRes,  &QPushButton::clicked, this, &MainWindow::refreshReservTable);
    connect(reservDateEdit, &QDateEdit::dateChanged, this, &MainWindow::refreshReservTable);

    refreshMenuTable();
    return page;
}

void MainWindow::refreshMenuTable() {
    if (!restDormSelector || restDormSelector->count() == 0) { if(menuTable) menuTable->setRowCount(0); return; }
    int dormId = restDormSelector->currentData().toInt();
    auto items = DatabaseManager::getInstance()->getMenuItems(dormId);
    menuTable->setRowCount(0);
    for (const MenuItem& m : items) {
        int row = menuTable->rowCount();
        menuTable->insertRow(row);
        menuTable->setRowHeight(row, 54);

        QString mealLabel = m.getMealType();
        mealLabel[0] = mealLabel[0].toUpper();
        auto* ti = new QTableWidgetItem(mealLabel);
        ti->setTextAlignment(Qt::AlignCenter);
        menuTable->setItem(row, 0, ti);

        auto* ni = new QTableWidgetItem(m.getName());
        ni->setData(Qt::UserRole, m.getId());
        menuTable->setItem(row, 1, ni);

        auto* photoCell = new QLabel;
        photoCell->setAlignment(Qt::AlignCenter);
        photoCell->setFixedSize(56, 46);
        if (!m.getPhotoPath().isEmpty() && QFile::exists(m.getPhotoPath())) {
            QPixmap px(m.getPhotoPath());
            if (!px.isNull())
                photoCell->setPixmap(px.scaled(48, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            photoCell->setText("—");
            photoCell->setStyleSheet("color:#9FB6AB; font-size:11px;");
        }
        menuTable->setCellWidget(row, 2, photoCell);

        auto* pillWrap = new QWidget;
        auto* pillL = new QHBoxLayout(pillWrap);
        pillL->setContentsMargins(0,0,0,0);
        pillL->setAlignment(Qt::AlignCenter);
        auto* pill = new QLabel(m.isAvailable() ? "Available" : "Unavailable");
        pill->setAlignment(Qt::AlignCenter);
        pill->setFixedHeight(26);
        pill->setStyleSheet(QString(
            "background:%1; color:%2; border-radius:13px; font-size:11px;"
            " font-weight:700; font-family:'Segoe UI'; padding:0 10px;")
            .arg(m.isAvailable() ? C::PRIMARY_LT : C::BDR)
            .arg(m.isAvailable() ? C::PRIMARY_DK : C::MUTED));
        pillL->addWidget(pill);
        menuTable->setCellWidget(row, 3, pillWrap);
    }
    refreshReservTable();
}

void MainWindow::refreshReservTable() {
    if (!restDormSelector || restDormSelector->count() == 0) return;
    if (!reservTable || !reservDateEdit) return;
    int dormId = restDormSelector->currentData().toInt();
    QString date = reservDateEdit->date().toString("yyyy-MM-dd");
    auto rows = DatabaseManager::getInstance()->getReservationsForDate(dormId, date);
    reservTable->setRowCount(0);
    for (const auto& r : rows) {
        int row = reservTable->rowCount();
        reservTable->insertRow(row);
        reservTable->setRowHeight(row, 46);
        for (int c = 0; c < (int)r.size() && c < 5; ++c) {
            QString text = r[c];
            if (c == 3) { text[0] = text[0].toUpper(); }
            auto* item = new QTableWidgetItem(text);
            if (c >= 1 && c != 2) item->setTextAlignment(Qt::AlignCenter);
            reservTable->setItem(row, c, item);
        }
    }
}

void MainWindow::onAddMenuItem() {
    if (!restDormSelector || restDormSelector->count() == 0) return;
    int dormId = restDormSelector->currentData().toInt();
    MenuItemDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    auto [name, type, photoSrcPath] = dlg.getData();

    QString storedPath;
    if (!photoSrcPath.isEmpty() && QFile::exists(photoSrcPath)) {
QString ext = QFileInfo(photoSrcPath).suffix();
        QString destName = QString("food_%1_%2.%3")
            .arg(dormId).arg(QDateTime::currentMSecsSinceEpoch()).arg(ext);
        QString dest = DatabaseManager::getInstance()->getPhotoDir() + "/" + destName;
        if (QFile::copy(photoSrcPath, dest)) storedPath = dest;
    }
    DatabaseManager::getInstance()->addMenuItem(dormId, name, type, storedPath);
    refreshMenuTable();
}

void MainWindow::onDeleteMenuItem() {
    int row = menuTable->currentRow();
    if (row < 0) { QMessageBox::information(this,"Remove","Select a dish first."); return; }
    int id = menuTable->item(row, 1)->data(Qt::UserRole).toInt();
    if (QMessageBox::question(this,"Confirm","Remove this menu item?") == QMessageBox::Yes) {
        DatabaseManager::getInstance()->deleteMenuItem(id);
        refreshMenuTable();
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  REPORTS PAGE
// ═══════════════════════════════════════════════════════════════════════════
QWidget* MainWindow::pageReports() {
    auto* page = new QWidget;
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea{background:transparent;border:none;}");

    auto* content = new QWidget;
    content->setStyleSheet("background:transparent;");
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(0,0,4,0);
    l->setSpacing(16);

    l->addWidget(makeBanner(":/icons/banner_reports.svg", "Reports & Analytics",
        "Live occupancy statistics and meal reservation summaries across all dormitories."));

    auto* statsRow = new QHBoxLayout;
    statsRow->setSpacing(14);
    auto* sc1 = makeStatCard("—", "Total Capacity", C::PRIMARY_DK, C::PRIMARY_LT, ":/icons/ic_rooms.svg");
    auto* sc2 = makeStatCard("—", "Occupied",       C::NAVY,       C::NAVY_LT,    ":/icons/ic_students.svg");
    auto* sc3 = makeStatCard("—", "Available Beds", C::CYAN,       C::CYAN_LT,    ":/icons/ic_key.svg");
    auto* sc4 = makeStatCard("—", "Occupancy Rate", C::AMBER,      C::AMBER_LT,   ":/icons/ic_reports.svg");
    sc1->setObjectName("sc1"); sc2->setObjectName("sc2");
    sc3->setObjectName("sc3"); sc4->setObjectName("sc4");
    statsRow->addWidget(sc1); statsRow->addWidget(sc2);
    statsRow->addWidget(sc3); statsRow->addWidget(sc4);
    l->addLayout(statsRow);

    auto* midRow = new QHBoxLayout;
    midRow->setSpacing(14);

    auto* trendInner = new QWidget;
    auto* trendL = new QVBoxLayout(trendInner);
    trendL->setContentsMargins(18,16,18,12);
    trendL->setSpacing(8);
    auto* trendTitle = new QLabel("Occupancy by Dormitory");
    trendTitle->setStyleSheet("color:#16261F; font-size:13px; font-weight:700; font-family:'Segoe UI';");
    trendL->addWidget(trendTitle);
    barChart = new BarChart;
    trendL->addWidget(barChart, 1);
    midRow->addWidget(wrapCard(trendInner), 3);

    auto* donutInner = new QWidget;
    auto* donutL = new QVBoxLayout(donutInner);
    donutL->setContentsMargins(18,16,18,12);
    donutL->setSpacing(8);
    auto* donutTitle = new QLabel("Rooms by Status");
    donutTitle->setStyleSheet("color:#16261F; font-size:13px; font-weight:700; font-family:'Segoe UI';");
    donutL->addWidget(donutTitle);
    donutChart = new DonutChart;
    donutL->addWidget(donutChart, 1);
    midRow->addWidget(wrapCard(donutInner), 2);

    l->addLayout(midRow);

    auto* occTitle = new QLabel("Dormitory Occupancy Detail");
    occTitle->setStyleSheet("color:#16261F; font-size:14px; font-weight:800; font-family:'Segoe UI';");
    l->addWidget(occTitle);

    reportTable = new QTableWidget(0, 5);
    reportTable->setHorizontalHeaderLabels(
        {"Dormitory", "Total Capacity", "Occupied", "Available", "% Full"});
    styleTable(reportTable);
    reportTable->setMinimumHeight(220);
    l->addWidget(wrapCard(reportTable, 8));

    auto* rptTitle = new QLabel("Meal Reservations by Date Range");
    rptTitle->setStyleSheet("color:#16261F; font-size:14px; font-weight:800; font-family:'Segoe UI';");
    l->addWidget(rptTitle);

    auto* rptToolbar = new QWidget;
    rptToolbar->setStyleSheet("QWidget{background:white;border-radius:14px;}");
    rptToolbar->setGraphicsEffect(mkShadow());
    auto* rptCtl = new QHBoxLayout(rptToolbar);
    rptCtl->setContentsMargins(16,10,16,10);
    rptCtl->setSpacing(10);
    auto* fromLbl = new QLabel("From:");
    fromLbl->setStyleSheet("color:#6B8478; font-size:12px; font-weight:600; font-family:'Segoe UI';");
    rptCtl->addWidget(fromLbl);
    rptFrom = new QDateEdit(QDate::currentDate().addDays(-7));
    rptFrom->setCalendarPopup(true); rptFrom->setFixedHeight(38);
    rptCtl->addWidget(rptFrom);
    auto* toLbl = new QLabel("To:");
    toLbl->setStyleSheet("color:#6B8478; font-size:12px; font-weight:600; font-family:'Segoe UI';");
    rptCtl->addWidget(toLbl);
    rptTo = new QDateEdit(QDate::currentDate());
    rptTo->setCalendarPopup(true); rptTo->setFixedHeight(38);
    rptCtl->addWidget(rptTo);
    auto* bRpt = makeBtn("  Generate", BtnStyle::Secondary);
    bRpt->setIcon(svgIcon(":/icons/ic_refresh.svg", QColor(C::PRIMARY_DK)));
    bRpt->setFixedHeight(38);
    rptCtl->addWidget(bRpt);
    rptCtl->addStretch();
    l->addWidget(rptToolbar);

    reservReport = new QTableWidget(0, 5);
    reservReport->setHorizontalHeaderLabels({"Date", "Breakfast", "Lunch", "Dinner", "Total"});
    styleTable(reservReport);
    reservReport->setMinimumHeight(200);
    l->addWidget(wrapCard(reservReport, 8));

    scroll->setWidget(content);
    auto* outer = new QVBoxLayout(page);
    outer->setContentsMargins(0,0,0,0);
    outer->addWidget(scroll);

    connect(bRpt, &QPushButton::clicked, this, &MainWindow::refreshReservReport);
    refreshOccupancyReport();
    refreshReservReport();
    return page;
}

void MainWindow::refreshOccupancyReport() {
    if (!reportTable) return;
    auto rows = DatabaseManager::getInstance()->getOccupancyReport();
    reportTable->setRowCount(0);

    int totalCap = 0, totalOcc = 0;
    QVector<QPair<QString,double>> donutData, barData;
    int fullRooms = 0, partialRooms = 0, freeRooms = 0;

    for (const auto& r : rows) {
        int row = reportTable->rowCount();
        reportTable->insertRow(row);
        reportTable->setRowHeight(row, 48);
        for (int c = 0; c < (int)r.size() && c < 5; ++c) {
            auto* item = new QTableWidgetItem(r[c]);
            if (c >= 1) item->setTextAlignment(Qt::AlignCenter);
            if (c == 4) {
                double pct = r[c].chopped(1).toDouble();
                item->setForeground(QColor(pct > 85 ? C::RED : pct > 60 ? C::AMBER : C::PRIMARY));
            }
            reportTable->setItem(row, c, item);
        }
        totalCap += r[1].toInt();
        totalOcc += r[2].toInt();
        barData.append({r[0], (double)r[1].toInt()});
    }

    for (const Dormitory& d : DatabaseManager::getInstance()->getDormitories()) {
        for (const Room& rm : DatabaseManager::getInstance()->getRooms(d.getId())) {
            if (rm.isFull()) ++fullRooms;
            else if (rm.getCurrentOccupancy() > 0) ++partialRooms;
            else ++freeRooms;
        }
    }
    donutData = { {"Available", (double)freeRooms},
                  {"Partial",   (double)partialRooms},
                  {"Full",      (double)fullRooms} };

    double pct = totalCap > 0 ? 100.0 * totalOcc / totalCap : 0.0;
    auto updateCard = [&](const QString& name, const QString& val) {
        if (auto* c = findChild<QWidget*>(name)) {
            if (auto* lbl = c->findChild<QLabel*>("statValue"))
                lbl->setText(val);
        }
    };
    updateCard("sc1", QString::number(totalCap));
    updateCard("sc2", QString::number(totalOcc));
    updateCard("sc3", QString::number(totalCap - totalOcc));
    updateCard("sc4", QString::number(pct, 'f', 1) + "%");

    if (donutChart) donutChart->setData(donutData);
    if (barChart)   barChart->setData(barData);
}

void MainWindow::refreshReservReport() {
    if (!reservReport || !rptFrom || !rptTo) return;
    QString from = rptFrom->date().toString("yyyy-MM-dd");
    QString to   = rptTo->date().toString("yyyy-MM-dd");
    auto rows = DatabaseManager::getInstance()->getReservationReport(from, to);
    reservReport->setRowCount(0);
    for (const auto& r : rows) {
        int row = reservReport->rowCount();
        reservReport->insertRow(row);
        reservReport->setRowHeight(row, 44);
        for (int c = 0; c < (int)r.size() && c < 5; ++c) {
            auto* item = new QTableWidgetItem(r[c]);
            if (c >= 1) item->setTextAlignment(Qt::AlignCenter);
            reservReport->setItem(row, c, item);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  USERS PAGE  (admin only)
// ═══════════════════════════════════════════════════════════════════════════
QWidget* MainWindow::pageUsers() {
    auto* page = new QWidget;
    auto* l = new QVBoxLayout(page);
    l->setContentsMargins(0,0,0,0);
    l->setSpacing(16);

    l->addWidget(makeBanner(":/icons/banner_users.svg", "User Management",
        "Admin only — manage staff, manager, and admin accounts with system access."));

    auto* toolbar = new QWidget;
    toolbar->setStyleSheet("QWidget{background:white;border-radius:14px;}");
    toolbar->setGraphicsEffect(mkShadow());
    auto* tbl = new QHBoxLayout(toolbar);
    tbl->setContentsMargins(16, 12, 16, 12);
    tbl->addStretch();
    auto* bAdd = makeBtn("  Add Staff Account", BtnStyle::Primary);
    bAdd->setIcon(svgIcon(":/icons/ic_plus.svg", Qt::white));
    tbl->addWidget(bAdd);
    l->addWidget(toolbar);

    userTable = new QTableWidget(0, 6);
    userTable->setHorizontalHeaderLabels(
        {"ID", "Username", "Role", "Status", "Linked Student", "Actions"});
    styleTable(userTable);
    userTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    userTable->setColumnWidth(5, 130);
    l->addWidget(wrapCard(userTable, 8), 1);

    connect(bAdd, &QPushButton::clicked, this, &MainWindow::onAddUser);
    refreshUserTable();
    return page;
}

void MainWindow::refreshUserTable() {
    if (!userTable) return;
    auto rows = DatabaseManager::getInstance()->getUsers();
    userTable->setRowCount(0);
    for (const auto& r : rows) {
        int row = userTable->rowCount();
        userTable->insertRow(row);
        userTable->setRowHeight(row, 52);
        for (int c = 0; c < 5; ++c) {
            auto* item = new QTableWidgetItem(r[c]);
            if (c == 2) {
                QString role = r[c]; role[0] = role[0].toUpper();
                item->setText(role);
                item->setTextAlignment(Qt::AlignCenter);
                if (r[c] == "admin")        item->setForeground(QColor(C::RED));
                else if (r[c] == "manager") item->setForeground(QColor(C::NAVY));
                else if (r[c] == "student") item->setForeground(QColor(C::PURPLE));
                else                        item->setForeground(QColor(C::PRIMARY_DK));
            }
            if (c == 4) {
                item->setText(r[c] == "0" ? "—" : "Student #" + r[c]);
                item->setTextAlignment(Qt::AlignCenter);
                item->setForeground(QColor(r[c] == "0" ? C::MUTED : C::PURPLE));
            }
            userTable->setItem(row, c, item);
        }

        bool active = r[3] == "1";
        auto* pillWrap = new QWidget;
        auto* pillL = new QHBoxLayout(pillWrap);
        pillL->setContentsMargins(0,0,0,0);
        pillL->setAlignment(Qt::AlignCenter);
        auto* pill = new QLabel(active ? "Active" : "Inactive");
        pill->setAlignment(Qt::AlignCenter);
        pill->setFixedHeight(26);
        pill->setStyleSheet(QString(
            "background:%1; color:%2; border-radius:13px; font-size:11px;"
            " font-weight:700; font-family:'Segoe UI'; padding:0 12px;")
            .arg(active ? C::PRIMARY_LT : C::BDR)
            .arg(active ? C::PRIMARY_DK : C::MUTED));
        pillL->addWidget(pill);
        userTable->setCellWidget(row, 3, pillWrap);

        auto* actWrap = new QWidget;
        auto* actL = new QHBoxLayout(actWrap);
        actL->setContentsMargins(4,4,4,4);
        actL->setAlignment(Qt::AlignCenter);
        auto* bToggle = new QPushButton(active ? "Deactivate" : "Activate");
        bToggle->setCursor(Qt::PointingHandCursor);
        bToggle->setFixedHeight(30);
        bToggle->setStyleSheet(QString(
            "QPushButton{background:%1;color:%2;border:none;border-radius:8px;"
            " font-size:10.5px; font-weight:700; font-family:'Segoe UI'; padding:0 10px;}"
            "QPushButton:hover{background:%3;}")
            .arg(active ? C::RED_LT : C::PRIMARY_LT)
            .arg(active ? C::RED : C::PRIMARY_DK)
            .arg(active ? "#FAD7D7" : C::PRIMARY_LT2));
        int uid = r[0].toInt();
        connect(bToggle, &QPushButton::clicked, this, [this, uid, active]() {
            DatabaseManager::getInstance()->setUserActive(uid, !active);
            refreshUserTable();
        });
        actL->addWidget(bToggle);
        userTable->setCellWidget(row, 5, actWrap);
    }
}

void MainWindow::onAddUser() {
    bool ok;
    QString u = QInputDialog::getText(this,"New Staff Account","Username:",
QLineEdit::Normal, "", &ok);
    if (!ok || u.isEmpty()) return;
    QString p = QInputDialog::getText(this,"New Staff Account",
                                      "Password (min. 8 characters):",
                                      QLineEdit::Password, "", &ok);
    if (!ok || p.isEmpty()) return;
    if (!User::isPasswordValid(p)) {
        QMessageBox::warning(this,"Invalid Password","Password must be at least 8 characters.");
        return;
    }
    QString r = QInputDialog::getItem(this,"New Staff Account","Role:",
                                      {"admin","manager","staff"}, 2, false, &ok);
    if (!ok) return;
    if (!DatabaseManager::getInstance()->addUser(u, p, r))
        QMessageBox::warning(this,"Error","Username already exists.");
    else refreshUserTable();
}

void MainWindow::onToggleUserActive() {
    // Retained for compatibility; per-row toggle buttons now handle this directly.
}

void MainWindow::onLogout() {
    if (QMessageBox::question(this,"Logout","Sign out?") == QMessageBox::Yes) {
        auto* login = new LoginWindow;
        login->show();
        close();
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  STUDENT PORTAL PAGE
// ═══════════════════════════════════════════════════════════════════════════
QWidget* MainWindow::pageStudentPortal() {
    int sid = currentUser->getStudentId();

    auto* page = new QWidget;
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea{background:transparent;border:none;}");

    auto* content = new QWidget;
    content->setStyleSheet("background:transparent;");
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(0,0,4,0);
    l->setSpacing(16);

    l->addWidget(makeBanner(":/icons/banner_portal.svg", "My Portal",
        "View your dormitory assignment and reserve your meals for upcoming days."));

    auto* infoInner = new QWidget;
    auto* infoL = new QHBoxLayout(infoInner);
    infoL->setContentsMargins(20, 18, 20, 18);
    infoL->setSpacing(28);

    auto students = DatabaseManager::getInstance()->getStudents(QString::number(sid));
    QString studentName = students.isEmpty() ? "?" : students[0].getName();

    auto* avatar = new QLabel(studentName.isEmpty() ? "?" : QString(studentName[0].toUpper()));
    avatar->setFixedSize(64, 64);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(
        "background:#1E9E6B; color:white; font-size:24px; font-weight:800;"
        " font-family:'Segoe UI'; border-radius:32px;"
        );
    infoL->addWidget(avatar);

    auto* infoRight = new QVBoxLayout;
    infoRight->setSpacing(4);
    spName = new QLabel(studentName);
    spName->setStyleSheet("color:#16261F; font-size:17px; font-weight:800; font-family:'Segoe UI';");
    infoRight->addWidget(spName);
    spId = new QLabel(QString("Student ID: %1").arg(sid));
    spId->setStyleSheet("color:#6B8478; font-size:12px; font-family:'Segoe UI';");
    infoRight->addWidget(spId);
    infoRight->addStretch();
    infoL->addLayout(infoRight, 1);

    auto roomInfo = DatabaseManager::getInstance()->getStudentRoomInfo(sid);
    auto* roomGroup = new QGroupBox("My Room");
    auto* rgl = new QVBoxLayout(roomGroup);
    rgl->setSpacing(4);
    if (roomInfo.isEmpty()) {
        spDorm = new QLabel("Not yet assigned");
        spDorm->setStyleSheet("color:#9FB6AB; font-size:12px; font-family:'Segoe UI'; border:none;");
        spRoom = new QLabel("Contact the dormitory office.");
        spRoom->setStyleSheet("color:#9FB6AB; font-size:11px; font-family:'Segoe UI'; border:none;");
    } else {
        spDorm = new QLabel(roomInfo[0]);
        spDorm->setStyleSheet("color:#6B8478; font-size:11px; font-family:'Segoe UI'; border:none;");
        spRoom = new QLabel("Room " + roomInfo[1] + "   ·  " + roomInfo[3] +
                            "   ·  " + roomInfo[4] + "/" + roomInfo[2] + " occupants");
        spRoom->setStyleSheet(
            "color:#16261F; font-size:13px; font-weight:700; font-family:'Segoe UI'; border:none;");
    }
    rgl->addWidget(spDorm);
    rgl->addWidget(spRoom);
    infoL->addWidget(roomGroup);
    l->addWidget(wrapCard(infoInner));

    auto* mealInner = new QWidget;
    auto* mealL = new QVBoxLayout(mealInner);
    mealL->setContentsMargins(20, 18, 20, 18);
    mealL->setSpacing(12);

    auto* mealTitle = new QLabel("Reserve a Meal");
    mealTitle->setStyleSheet("color:#16261F; font-size:15px; font-weight:800; font-family:'Segoe UI';");
    mealL->addWidget(mealTitle);
    auto* mealSub = new QLabel("Select a date and cafeteria, then click a dish to reserve or cancel your slot.");
    mealSub->setStyleSheet("color:#6B8478; font-size:11.5px; font-family:'Segoe UI';");
    mealSub->setWordWrap(true);
    mealL->addWidget(mealSub);

    auto* ctlRow = new QHBoxLayout;
    ctlRow->setSpacing(12);
    auto* dateLbl = new QLabel("Date:");
    dateLbl->setStyleSheet("color:#6B8478; font-size:12px; font-weight:600; font-family:'Segoe UI';");
    ctlRow->addWidget(dateLbl);
    spDateEdit = new QDateEdit(QDate::currentDate());
    spDateEdit->setCalendarPopup(true);
    spDateEdit->setFixedHeight(40);
    spDateEdit->setMinimumDate(QDate::currentDate());
    spDateEdit->setMaximumDate(QDate::currentDate().addDays(30));
    ctlRow->addWidget(spDateEdit);
    auto* dormLbl = new QLabel("Cafeteria:");
    dormLbl->setStyleSheet("color:#6B8478; font-size:12px; font-weight:600; font-family:'Segoe UI';");
    ctlRow->addWidget(dormLbl);
    spDormCombo = new QComboBox;
    spDormCombo->setFixedHeight(40);
    fillDormCombo(spDormCombo);
    ctlRow->addWidget(spDormCombo);
    ctlRow->addStretch();
    mealL->addLayout(ctlRow);

    spMenuTable = new QTableWidget(0, 4);
    spMenuTable->setHorizontalHeaderLabels({"Meal", "Dish", "Photo", "My Status"});
    styleTable(spMenuTable);
    spMenuTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    spMenuTable->setColumnWidth(2, 80);
    spMenuTable->setMinimumHeight(220);
    mealL->addWidget(spMenuTable);

    auto* mealBtns = new QHBoxLayout;
    mealBtns->setSpacing(10);
    auto* bRes = makeBtn("  Reserve Selected", BtnStyle::Primary);
    bRes->setIcon(svgIcon(":/icons/ic_check.svg", Qt::white));
    auto* bCan = makeBtn("  Cancel Reservation", BtnStyle::Danger);
    bCan->setIcon(svgIcon(":/icons/ic_close.svg", QColor(C::RED)));
    mealBtns->addWidget(bRes); mealBtns->addWidget(bCan); mealBtns->addStretch();
    mealL->addLayout(mealBtns);
    l->addWidget(wrapCard(mealInner));

    auto* histInner = new QWidget;
    auto* histL = new QVBoxLayout(histInner);
    histL->setContentsMargins(20, 18, 20, 18);
    histL->setSpacing(10);
    auto* histTitle = new QLabel("My Reservation History");
    histTitle->setStyleSheet("color:#16261F; font-size:15px; font-weight:800; font-family:'Segoe UI';");
    histL->addWidget(histTitle);
    spReservTable = new QTableWidget(0, 3);
    spReservTable->setHorizontalHeaderLabels({"Date", "Meal Type", "Dish"});
    styleTable(spReservTable);
    spReservTable->setMaximumHeight(220);
    histL->addWidget(spReservTable);
    l->addWidget(wrapCard(histInner));

    scroll->setWidget(content);
    auto* outer = new QVBoxLayout(page);
    outer->setContentsMargins(0,0,0,0);
    outer->addWidget(scroll);

    connect(spDateEdit,  &QDateEdit::dateChanged, this, &MainWindow::spRefreshMenu);
    connect(spDormCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::spRefreshMenu);
    connect(bRes, &QPushButton::clicked, this, &MainWindow::spReserve);
    connect(bCan, &QPushButton::clicked, this, &MainWindow::spCancel);

    spRefreshMenu();
    spRefreshReservations();
    return page;
}

void MainWindow::spRefreshMenu() {
    if (!spMenuTable || !spDateEdit || !spDormCombo) return;
    int sid = currentUser->getStudentId();
    if (spDormCombo->count() == 0) { spMenuTable->setRowCount(0); return; }
    int dormId = spDormCombo->currentData().toInt();
    QString date = spDateEdit->date().toString("yyyy-MM-dd");

    auto items = DatabaseManager::getInstance()->getMenuItems(dormId);
    spMenuTable->setRowCount(0);
    for (const MenuItem& m : items) {
        int row = spMenuTable->rowCount();
        spMenuTable->insertRow(row);
        spMenuTable->setRowHeight(row, 54);

        QString mealLabel = m.getMealType(); mealLabel[0] = mealLabel[0].toUpper();
        auto* ti = new QTableWidgetItem(mealLabel);
        ti->setTextAlignment(Qt::AlignCenter);
        spMenuTable->setItem(row, 0, ti);

        auto* ni = new QTableWidgetItem(m.getName());
        ni->setData(Qt::UserRole, m.getId());
        spMenuTable->setItem(row, 1, ni);

        auto* photoLbl = new QLabel;
        photoLbl->setAlignment(Qt::AlignCenter);
        if (!m.getPhotoPath().isEmpty() && QFile::exists(m.getPhotoPath())) {
            QPixmap px(m.getPhotoPath());
            if (!px.isNull())
                photoLbl->setPixmap(px.scaled(48, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            photoLbl->setText("—");
            photoLbl->setStyleSheet("color:#9FB6AB; font-size:11px;");
        }
        spMenuTable->setCellWidget(row, 2, photoLbl);

        bool reserved = DatabaseManager::getInstance()->hasReservation(sid, m.getMealType(), date);
        auto* pillWrap = new QWidget;
        auto* pillL = new QHBoxLayout(pillWrap);
        pillL->setContentsMargins(0,0,0,0);
        pillL->setAlignment(Qt::AlignCenter);
        auto* pill = new QLabel(reserved ? "Reserved" : "Not reserved");
        pill->setAlignment(Qt::AlignCenter);
        pill->setFixedHeight(26);
        pill->setStyleSheet(QString(
            "background:%1; color:%2; border-radius:13px; font-size:11px;"
            " font-weight:700; font-family:'Segoe UI'; padding:0 10px;")
            .arg(reserved ? C::PRIMARY_LT : C::BDR)
            .arg(reserved ? C::PRIMARY_DK : C::MUTED));
        pillL->addWidget(pill);
        spMenuTable->setCellWidget(row, 3, pillWrap);
    }
}

void MainWindow::spRefreshReservations() {
    if (!spReservTable) return;
    int sid = currentUser->getStudentId();
    auto rows = DatabaseManager::getInstance()->getStudentReservations(sid);
    spReservTable->setRowCount(0);
    for (const auto& r : rows) {
        int row = spReservTable->rowCount();
        spReservTable->insertRow(row);
        spReservTable->setRowHeight(row, 42);
        spReservTable->setItem(row, 0, new QTableWidgetItem(r[0]));
        QString mealLabel = r[1]; mealLabel[0] = mealLabel[0].toUpper();
        auto* ti = new QTableWidgetItem(mealLabel);
        ti->setTextAlignment(Qt::AlignCenter);
        spReservTable->setItem(row, 1, ti);
        spReservTable->setItem(row, 2, new QTableWidgetItem(r[2]));
    }
}

void MainWindow::spReserve() {
    if (!spMenuTable || !spDateEdit) return;
    int row = spMenuTable->currentRow();
    if (row < 0) { QMessageBox::information(this,"Reserve","Select a dish first."); return; }
    int sid = currentUser->getStudentId();
    int itemId = spMenuTable->item(row, 1)->data(Qt::UserRole).toInt();
    QString date = spDateEdit->date().toString("yyyy-MM-dd");

    if (!DatabaseManager::getInstance()->reserveMeal(sid, itemId, date)) {
        QMessageBox::warning(this,"Already Reserved",
            "You already have a reservation for this meal type on that date.");
        return;
    }
    QMessageBox::information(this,"Reserved","Your meal has been reserved!");
    spRefreshMenu();
    spRefreshReservations();
}

void MainWindow::spCancel() {
    if (!spMenuTable || !spDateEdit) return;
    int row = spMenuTable->currentRow();
    if (row < 0) { QMessageBox::information(this,"Cancel","Select a dish first."); return; }
    int sid = currentUser->getStudentId();
    int itemId = spMenuTable->item(row, 1)->data(Qt::UserRole).toInt();
    QString date = spDateEdit->date().toString("yyyy-MM-dd");

    if (!DatabaseManager::getInstance()->cancelReservation(sid, itemId, date)) {
        QMessageBox::warning(this,"Not Found","No reservation found for this dish on that date.");
        return;
    }
    QMessageBox::information(this,"Cancelled","Your reservation has been cancelled.");
    spRefreshMenu();
    spRefreshReservations();
}