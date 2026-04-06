#pragma once
/**
 * @file CpuPanel.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 * @brief Painel CPU: gauge, métricas, sparklines, load bar, RAM e power.
 */
#ifndef NY_CPU_PANEL_HPP
#define NY_CPU_PANEL_HPP

#include <QFrame>
#include "viewmodels/DashboardViewModel.hpp"

class QLabel;
class QWidget;
class QVBoxLayout;

namespace ny::ui::widgets {
class CircularGauge;
class SparklineWidget;
class LoadBarWidget;
}

namespace ny::ui::dashboard {

class CpuPanel : public QFrame {
    Q_OBJECT

public:
    explicit CpuPanel(QWidget* parent = nullptr);

    void updateData(
        const ny::ui::viewmodels::DashboardViewModel::CpuViewData&    cpu,
        const ny::ui::viewmodels::DashboardViewModel::MemoryViewData& mem
    );
    [[nodiscard]] int gaugeContainerHeightHint() const;
    [[nodiscard]] int infoRowContainerHeightHint() const;
    [[nodiscard]] int usageGraphContainerHeightHint() const;
    [[nodiscard]] int usageGraphRelativeIndex() const;
    [[nodiscard]] bool isUsageImmediatelyAfterInfoRow() const;
    [[nodiscard]] bool hasTopBlockOrder() const;

private:
    // Header
    QLabel* m_nameLabel  { nullptr };

    // Gauge + metrics
    ny::ui::widgets::CircularGauge* m_gauge { nullptr };

    QLabel* m_voltTitleLabel { nullptr };
    QLabel* m_voltValueLabel { nullptr };
    QLabel* m_fanTitleLabel  { nullptr };
    QLabel* m_fanValueLabel  { nullptr };
    QLabel* m_usageTitleLabel{ nullptr };
    QLabel* m_usageValueLabel{ nullptr };

    ny::ui::widgets::SparklineWidget* m_usageSpark { nullptr };  ///< CPU Usage (%)
    QWidget* m_gaugeContainer { nullptr };
    QWidget* m_infoRowContainer { nullptr };
    QWidget* m_usageGraphContainer { nullptr };
    QVBoxLayout* m_topSectionLayout { nullptr };

    // Load bar
    ny::ui::widgets::LoadBarWidget* m_loadBar  { nullptr };
    QLabel*                         m_loadPct  { nullptr };

    // RAM card
    QLabel* m_ramLabel  { nullptr };

    // Power card
    QLabel* m_powerLabel{ nullptr };

    static float parseFirstFloat(const QString& s, float fallback = 0.0f);
};

} // namespace ny::ui::dashboard

#endif // NY_CPU_PANEL_HPP
