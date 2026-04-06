/**
 * @file DashboardPage.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */
#include "DashboardPage.hpp"

#include <QHBoxLayout>
#include <QRegularExpression>
#include <QtGlobal>
#include "GpuPanel.hpp"
#include "CpuPanel.hpp"
#include "ChartAreaPanel.hpp"

namespace ny::ui::dashboard {

DashboardPage::DashboardPage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("dashboardPage");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(7);

    m_gpuPanel  = new GpuPanel;
    m_chartArea = new ChartAreaPanel;
    m_cpuPanel  = new CpuPanel;

    // Structural alignment validation between GPU and CPU top sections.
    Q_ASSERT(m_gpuPanel->gaugeContainerHeightHint() == m_cpuPanel->gaugeContainerHeightHint());
    Q_ASSERT(m_gpuPanel->infoRowContainerHeightHint() == m_cpuPanel->infoRowContainerHeightHint());
    Q_ASSERT(m_gpuPanel->usageGraphContainerHeightHint() == m_cpuPanel->usageGraphContainerHeightHint());
    Q_ASSERT(m_gpuPanel->usageGraphRelativeIndex() == m_cpuPanel->usageGraphRelativeIndex());
    Q_ASSERT(m_gpuPanel->isUsageImmediatelyAfterInfoRow());
    Q_ASSERT(m_cpuPanel->isUsageImmediatelyAfterInfoRow());
    Q_ASSERT(m_gpuPanel->hasTopBlockOrder());
    Q_ASSERT(m_cpuPanel->hasTopBlockOrder());

    // QSizePolicy::Ignored: o sizeHint() é desconsiderado para distribuição de espaço.
    // Apenas os stretch factors determinam as larguras → GPU = CPU garantido matematicamente.
    m_gpuPanel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    m_cpuPanel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    m_chartArea->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);

    // 3 : 6 : 3  — GPU = CPU (ambos 3), charts = 2× cada lateral
    layout->addWidget(m_gpuPanel,  3);
    layout->addWidget(m_chartArea, 6);
    layout->addWidget(m_cpuPanel,  3);
}

void DashboardPage::updateDashboard(
    const ny::ui::viewmodels::DashboardViewModel::CpuViewData&        cpu,
    const ny::ui::viewmodels::DashboardViewModel::MemoryViewData&     mem,
    const QList<ny::ui::viewmodels::DashboardViewModel::GpuViewData>& gpus)
{
    m_cpuPanel->updateData(cpu, mem);

    if (!gpus.isEmpty()) {
        m_gpuPanel->updateData(gpus.first());

        // Chart history
        static QRegularExpression re(R"([-+]?\d+\.?\d*)");
        auto parseF = [&](const QString& s) -> float {
            auto m = re.match(s);
            if (!m.hasMatch()) return 0.0f;
            bool ok; float v = m.captured().toFloat(&ok);
            return ok ? v : 0.0f;
        };

        const float gpuTemp = parseF(gpus.first().temperature);
        if (gpuTemp > 0.0f) m_chartArea->addGpuTempPoint(gpuTemp);
    }

    const float cpuTemp = [&]() -> float {
        static QRegularExpression re2(R"([-+]?\d+\.?\d*)");
        auto m = re2.match(cpu.temperature);
        if (!m.hasMatch()) return 0.0f;
        bool ok; float v = m.captured().toFloat(&ok);
        return ok ? v : 0.0f;
    }();
    if (cpuTemp > 0.0f) m_chartArea->addCpuTempPoint(cpuTemp);
}

} // namespace ny::ui::dashboard
