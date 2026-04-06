/**
 * @file ChartAreaPanel.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */
#include "ChartAreaPanel.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "widgets/LineChartWidget.hpp"

namespace ny::ui::dashboard {


ChartAreaPanel::ChartAreaPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("chartArea");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);

    // GPU TEMP chart — série única, sem overlay de clock
    m_gpuChart = new ny::ui::widgets::LineChartWidget;
    m_gpuChart->setColor(QColor(0x00, 0xDD, 0xFF));
    m_gpuChart->setTitle("GPU TEMP");
    m_gpuChart->setObjectName("chartFrame");

    // CPU TEMP chart
    m_cpuChart = new ny::ui::widgets::LineChartWidget;
    m_cpuChart->setColor(QColor(0xFF, 0x9A, 0x00));
    m_cpuChart->setTitle("CPU TEMP");
    m_cpuChart->setObjectName("chartFrame");

    layout->addWidget(m_gpuChart, 1);
    layout->addWidget(m_cpuChart, 1);
}

void ChartAreaPanel::addGpuTempPoint(float v) { m_gpuChart->addPoint(v); }

void ChartAreaPanel::addCpuTempPoint(float v) {
    m_cpuChart->addPoint(v);
    // overlay removido — GPU chart exibe apenas temperatura da GPU
}

} // namespace ny::ui::dashboard

