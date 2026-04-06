#pragma once
/**
 * @file GpuPanel.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 * @brief Painel GPU completo: gauge, métricas, sparklines e sliders de tuning.
 */
#ifndef NY_GPU_PANEL_HPP
#define NY_GPU_PANEL_HPP

#include <QFrame>
#include "viewmodels/DashboardViewModel.hpp"

class QLabel;
class QSlider;
class QWidget;
class QVBoxLayout;

namespace ny::ui::widgets {
class CircularGauge;
class SparklineWidget;
}

namespace ny::ui::dashboard {

class GpuPanel : public QFrame {
    Q_OBJECT

public:
    explicit GpuPanel(QWidget* parent = nullptr);

    void updateData(const ny::ui::viewmodels::DashboardViewModel::GpuViewData& gpu);
    [[nodiscard]] int gaugeContainerHeightHint() const;
    [[nodiscard]] int infoRowContainerHeightHint() const;
    [[nodiscard]] int usageGraphContainerHeightHint() const;
    [[nodiscard]] int usageGraphRelativeIndex() const;
    [[nodiscard]] bool isUsageImmediatelyAfterInfoRow() const;
    [[nodiscard]] bool hasTopBlockOrder() const;

private:
    // ── Header ──────────────────────────────────────────────────────────
    QLabel* m_nameLabel    { nullptr };
    QLabel* m_vendorLabel  { nullptr };

    // ── Gauge + right metrics ────────────────────────────────────────────
    ny::ui::widgets::CircularGauge* m_gauge { nullptr };

    // ── Left side metrics (flanking gauge) ──────────────────────────────
    QLabel* m_fanTitleLabel      { nullptr };   // "FAN %"
    QLabel* m_fanValueLabel      { nullptr };
    QLabel* m_fanRpmTitleLabel   { nullptr };   // "RPM"
    QLabel* m_fanRpmValueLabel   { nullptr };

    // ── Right side metrics (flanking gauge) ─────────────────────────────
    QLabel* m_memTitleLabel      { nullptr };   // "MEMORY"
    QLabel* m_memValueLabel      { nullptr };
    QLabel* m_memFreqTitleLabel  { nullptr };   // "MEM FREQ"
    QLabel* m_memFreqValueLabel  { nullptr };

    // ── Hidden (stored, not in layout) ──────────────────────────────────
    QLabel* m_pwrTitleLabel  { nullptr };
    QLabel* m_pwrValueLabel  { nullptr };

    ny::ui::widgets::SparklineWidget* m_usageSpark { nullptr };  ///< GPU Usage (%)
    QWidget* m_gaugeContainer { nullptr };
    QWidget* m_infoRowContainer { nullptr };
    QWidget* m_usageGraphContainer { nullptr };
    QVBoxLayout* m_topSectionLayout { nullptr };

    // ── Tuning sliders ───────────────────────────────────────────────────
    QSlider* m_coreSlider   { nullptr };
    QSlider* m_memSlider    { nullptr };
    QSlider* m_voltSlider   { nullptr };
    QSlider* m_powerSlider  { nullptr };

    QLabel*  m_coreVal      { nullptr };
    QLabel*  m_memVal       { nullptr };
    QLabel*  m_voltVal      { nullptr };
    QLabel*  m_powerVal     { nullptr };

    static float parseFirstFloat(const QString& s, float fallback = 0.0f);
    QWidget* buildSliderRow(const QString& label, QSlider*& slider, QLabel*& valLabel,
                            int minVal, int maxVal, int defaultVal,
                            const QString& suffix);
};

} // namespace ny::ui::dashboard

#endif // NY_GPU_PANEL_HPP
