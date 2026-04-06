#pragma once
/**
 * @file ChartAreaPanel.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 * @brief Área central com dois gráficos de linha: GPU Temp e CPU Temp.
 */
#ifndef NY_CHART_AREA_PANEL_HPP
#define NY_CHART_AREA_PANEL_HPP

#include <QWidget>

namespace ny::ui::widgets {
class LineChartWidget;
}

namespace ny::ui::dashboard {

class ChartAreaPanel : public QWidget {
    Q_OBJECT

public:
    explicit ChartAreaPanel(QWidget* parent = nullptr);

    void addGpuTempPoint(float value);
    void addCpuTempPoint(float value);

private:
    ny::ui::widgets::LineChartWidget* m_gpuChart { nullptr };
    ny::ui::widgets::LineChartWidget* m_cpuChart { nullptr };
};

} // namespace ny::ui::dashboard

#endif // NY_CHART_AREA_PANEL_HPP

