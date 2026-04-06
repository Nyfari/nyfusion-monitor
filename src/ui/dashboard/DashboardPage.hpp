#pragma once
/**
 * @file DashboardPage.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 * @brief Layout principal do dashboard: GPU | Charts | CPU em 3 colunas.
 */
#ifndef NY_DASHBOARD_PAGE_HPP
#define NY_DASHBOARD_PAGE_HPP

#include <QWidget>
#include <QList>
#include "viewmodels/DashboardViewModel.hpp"

namespace ny::ui::dashboard {
class GpuPanel;
class CpuPanel;
class ChartAreaPanel;
}

namespace ny::ui::dashboard {

class DashboardPage : public QWidget {
    Q_OBJECT

public:
    explicit DashboardPage(QWidget* parent = nullptr);

    void updateDashboard(
        const ny::ui::viewmodels::DashboardViewModel::CpuViewData&          cpu,
        const ny::ui::viewmodels::DashboardViewModel::MemoryViewData&       mem,
        const QList<ny::ui::viewmodels::DashboardViewModel::GpuViewData>&   gpus
    );

private:
    GpuPanel*       m_gpuPanel   { nullptr };
    ChartAreaPanel* m_chartArea  { nullptr };
    CpuPanel*       m_cpuPanel   { nullptr };
};

} // namespace ny::ui::dashboard

#endif // NY_DASHBOARD_PAGE_HPP

