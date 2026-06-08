#pragma once
/**
 * @file OverlayWindow.hpp
 * @author Marcos Henrique
 * @date 16/05/2026
 * @brief Janela flutuante multiplataforma do overlay em tempo real.
 */
#ifndef NY_FUSION_MONITOR_OVERLAY_WINDOW_HPP
#define NY_FUSION_MONITOR_OVERLAY_WINDOW_HPP

#include <memory>
#include <optional>

#include <QHash>
#include <QList>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include "services/OverlayRuntimeController.hpp"
#include "viewmodels/DashboardViewModel.hpp"

class QLabel;
class QGridLayout;
class QScreen;
class QVBoxLayout;

namespace ny::ui::viewmodels {
class OverlaySettingsViewModel;
}

namespace ny::ui::qt {

class OverlayWindow final : public QWidget {
    Q_OBJECT

public:
    explicit OverlayWindow(QWidget* parent = nullptr);
    ~OverlayWindow() override;

    void reloadSettings();
    void setRuntimeState(const ny::ui::services::OverlayRuntimeController::RuntimeState& state);
    void updateMetrics(
        const ny::ui::viewmodels::DashboardViewModel::CpuViewData& cpu,
        const ny::ui::viewmodels::DashboardViewModel::MemoryViewData& mem,
        const QList<ny::ui::viewmodels::DashboardViewModel::GpuViewData>& gpus
    );

protected:
    void showEvent(QShowEvent* event) override;

private:
    void buildUi();
    void refreshContent();
    void syncVisibility();
    void applyPassiveVisibility(bool active);
    void moveToConfiguredPosition();
    void updateHistory();
    [[nodiscard]] QScreen* targetScreen() const;

    [[nodiscard]] QString currentMetricValue(const QString& metricId) const;
    [[nodiscard]] QString formattedMetricValue(const QString& rawValue) const;
    [[nodiscard]] QString trendString(const QString& metricId) const;
    [[nodiscard]] static std::optional<float> parseNumericValue(const QString& rawValue);
    [[nodiscard]] static QString metricAccentColor(const QString& metricId);

    std::unique_ptr<ny::ui::viewmodels::OverlaySettingsViewModel> m_settings;
    ny::ui::services::OverlayRuntimeController::RuntimeState m_runtimeState;

    ny::ui::viewmodels::DashboardViewModel::CpuViewData m_cpuData;
    ny::ui::viewmodels::DashboardViewModel::MemoryViewData m_memoryData;
    QList<ny::ui::viewmodels::DashboardViewModel::GpuViewData> m_gpuData;
    QHash<QString, QVector<float>> m_metricHistory;

    QWidget* m_surface{ nullptr };
    QLabel* m_titleLabel{ nullptr };
    QLabel* m_subtitleLabel{ nullptr };
    QLabel* m_modeBadge{ nullptr };
    QGridLayout* m_metricsLayout{ nullptr };
    bool m_isMapped{ false };
    QTimer m_raiseTimer;
};

} // namespace ny::ui::qt

#endif // NY_FUSION_MONITOR_OVERLAY_WINDOW_HPP



