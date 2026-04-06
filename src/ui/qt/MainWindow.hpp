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
class QToolButton;

namespace ny::ui::dashboard {
class DashboardPage;
}

namespace ny::ui::viewmodels {
class DashboardViewModel;
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

    // ── Sidebar ──────────────────────────────────────────────────────────
    QFrame*      m_sidebar      { nullptr };
    QToolButton* m_activeSideBtn{ nullptr };

    // ── Content ──────────────────────────────────────────────────────────
    ny::ui::dashboard::DashboardPage* m_dashboardPage{ nullptr };

    // ── Backend ──────────────────────────────────────────────────────────
    std::unique_ptr<ny::ui::viewmodels::DashboardViewModel> m_viewModel;
};

} // namespace ny::ui::qt

#endif // NY_FUSION_MONITOR_MAINWINDOW_HPP
