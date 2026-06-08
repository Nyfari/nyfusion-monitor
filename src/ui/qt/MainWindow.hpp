#pragma once
/**
 * @file MainWindow.hpp
 * @author Marcos Henrique
 * @date 04/01/2026
 * @brief Janela principal com sidebar, topbar e dashboard.
 */
#ifndef NY_FUSION_MONITOR_MAINWINDOW_HPP
#define NY_FUSION_MONITOR_MAINWINDOW_HPP

#include <memory>
#include <QMainWindow>

class QFrame;
class QLabel;
class QToolButton;

namespace ny::ui::dashboard {
class DashboardPage;
}

namespace ny::ui::qt {
class OverlayWindow;
}

namespace ny::ui::viewmodels {
class DashboardViewModel;
}

namespace ny::ui::services {
class OverlayRuntimeController;
}

namespace ny::ui::qt {

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void buildSidebar(QWidget* parent);
    void buildTopbar(QWidget* parent);
    void applyTheme();
    void wireSignals();
    void openOverlaySettingsDialog();
    void refreshOverlayRuntimeStatus();

    // ── Sidebar ──────────────────────────────────────────────────────────
    QFrame*      m_sidebar      { nullptr };
    QToolButton* m_homeBtn      { nullptr };
    QToolButton* m_overlayBtn   { nullptr };
    QToolButton* m_activeSideBtn{ nullptr };
    QLabel*      m_overlayStatusBadge{ nullptr };
    QLabel*      m_overlayDetailLabel{ nullptr };

    // ── Content ──────────────────────────────────────────────────────────
    ny::ui::dashboard::DashboardPage* m_dashboardPage{ nullptr };
    std::unique_ptr<ny::ui::qt::OverlayWindow> m_overlayWindow;

    // ── Backend ──────────────────────────────────────────────────────────
    std::unique_ptr<ny::ui::viewmodels::DashboardViewModel> m_viewModel;
    std::unique_ptr<ny::ui::services::OverlayRuntimeController> m_overlayRuntimeController;
};

} // namespace ny::ui::qt

#endif // NY_FUSION_MONITOR_MAINWINDOW_HPP
