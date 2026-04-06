/**
 * @file MainWindow.cpp
 * @author Marcos Henrique
 * @date 04/01/2026
 */
#include "MainWindow.hpp"

#include <QApplication>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

#include "dashboard/DashboardPage.hpp"
#include "viewmodels/DashboardViewModel.hpp"

namespace ny::ui::qt {

// ── Sidebar helper ────────────────────────────────────────────────────────────
static QToolButton* makeSideBtn(const QString& icon, const QString& label,
                                 QWidget* parent)
{
    auto* btn = new QToolButton(parent);
    btn->setObjectName("sideBtn");
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setCheckable(true);
    btn->setAutoExclusive(true);

    // Icon via font/text trick: use the Unicode char as a label in a QIcon-less
    // button; the "icon" param is displayed as the large char above the text.
    btn->setText(label);
    btn->setProperty("iconChar", icon);

    // Create a custom widget-based button: icon label + text label
    // (simpler: just use rich-text in a QLabel sub-layout)
    // We override display in the stylesheet using ::before pseudo → not in Qt QSS
    // Instead, build a compound label:
    btn->setText(icon + "\n" + label);
    btn->setToolButtonStyle(Qt::ToolButtonTextOnly);

    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btn->setFixedHeight(56);
    return btn;
}

// ── buildSidebar ─────────────────────────────────────────────────────────────
void MainWindow::buildSidebar(QWidget* sideWidget) {
    auto* layout = new QVBoxLayout(sideWidget);
    layout->setContentsMargins(0, 8, 0, 8);
    layout->setSpacing(2);

    // Brand badge
    auto* badge = new QLabel("NF", sideWidget);
    badge->setObjectName("sidebarBadge");
    badge->setAlignment(Qt::AlignCenter);
    badge->setFixedHeight(44);
    layout->addWidget(badge);

    layout->addSpacing(6);

    // Nav items: icon char + label
    struct NavItem { QString icon; QString text; };
    const NavItem items[] = {
        { "⌂",  "HOME"      },
        { "⊞",  "TUNING"    },
        { "⊕",  "FANS"      },
        { "⚡",  "OVER" },
        { "▲",  "BENCH" },
    };

    bool first = true;
    for (auto& item : items) {
        auto* btn = makeSideBtn(item.icon, item.text, sideWidget);
        if (first) {
            btn->setChecked(true);
            m_activeSideBtn = btn;
            first = false;
        }
        layout->addWidget(btn);
    }

    layout->addStretch();

    auto* settingsBtn = makeSideBtn("⚙", "SETTINGS", sideWidget);
    layout->addWidget(settingsBtn);
}

// ── buildTopbar ───────────────────────────────────────────────────────────────
void MainWindow::buildTopbar(QWidget* topWidget) {
    auto* layout = new QHBoxLayout(topWidget);
    layout->setContentsMargins(16, 0, 20, 0);
    layout->setSpacing(6);

    // Left: logo accent + title
    auto* logoAccent = new QLabel("◈", topWidget);
    logoAccent->setObjectName("topbarAccent");

    auto* titleLabel = new QLabel(topWidget);
    titleLabel->setObjectName("topbarTitle");
    titleLabel->setTextFormat(Qt::RichText);
    titleLabel->setText(
        R"(<span style="color:#00CCFF;font-weight:700;letter-spacing:1px;">NyFusion</span>)"
        R"( <span style="color:#C8D8E8;font-weight:400;letter-spacing:2px;">MONITOR</span>)"
    );

    layout->addWidget(logoAccent);
    layout->addWidget(titleLabel);
    layout->addStretch();

    // Right: version
    auto* versionLabel = new QLabel("v0.2", topWidget);
    versionLabel->setObjectName("topbarVersion");

    layout->addSpacing(12);
    layout->addWidget(versionLabel);
}

// ── Constructor ───────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_viewModel(std::make_unique<ny::ui::viewmodels::DashboardViewModel>())
{
    setWindowTitle("NyFusion Monitor");
    resize(1360, 820);
    setMinimumSize(900, 580);

    // ── Root central widget ───────────────────────────────────────────────
    auto* central = new QWidget(this);
    central->setObjectName("mainSurface");
    setCentralWidget(central);

    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── Topbar ────────────────────────────────────────────────────────────
    auto* topbar = new QFrame(central);
    topbar->setObjectName("topbar");
    topbar->setFixedHeight(44);
    topbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buildTopbar(topbar);
    rootLayout->addWidget(topbar);

    // ── Body (sidebar + content) ──────────────────────────────────────────
    auto* body = new QWidget(central);
    auto* bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);
    rootLayout->addWidget(body, 1);

    // Sidebar
    m_sidebar = new QFrame(body);
    m_sidebar->setObjectName("sidebar");
    m_sidebar->setFixedWidth(72);
    m_sidebar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    buildSidebar(m_sidebar);
    bodyLayout->addWidget(m_sidebar);

    // Thin border line between sidebar and content
    auto* divider = new QFrame(body);
    divider->setObjectName("sidebarDivider");
    divider->setFixedWidth(1);
    bodyLayout->addWidget(divider);

    // Content area
    auto* contentArea = new QWidget(body);
    contentArea->setObjectName("contentArea");
    auto* contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    bodyLayout->addWidget(contentArea, 1);

    m_dashboardPage = new ny::ui::dashboard::DashboardPage(contentArea);
    contentLayout->addWidget(m_dashboardPage, 1);

    applyTheme();
    wireSignals();
}

MainWindow::~MainWindow() = default;

// ── applyTheme ────────────────────────────────────────────────────────────────
void MainWindow::applyTheme() {
    QFile file(":/styles/theme.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(file.readAll()));
        file.close();
    }
}

// ── wireSignals ───────────────────────────────────────────────────────────────
void MainWindow::wireSignals() {
    connect(
        m_viewModel.get(),
        &ny::ui::viewmodels::DashboardViewModel::dashboardUpdated,
        this,
        [this]() {
            m_dashboardPage->updateDashboard(
                m_viewModel->cpuData(),
                m_viewModel->memoryData(),
                m_viewModel->gpusData()
            );
        }
    );
}

} // namespace ny::ui::qt
