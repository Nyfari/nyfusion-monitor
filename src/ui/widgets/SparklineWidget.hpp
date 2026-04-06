#pragma once
/**
 * @file SparklineWidget.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 * @brief Mini gráfico de linha com buffer circular, escalável ao container.
 */
#ifndef NY_SPARKLINE_WIDGET_HPP
#define NY_SPARKLINE_WIDGET_HPP

#include "LineAreaChartWidget.hpp"

namespace ny::ui::widgets {

class SparklineWidget : public LineAreaChartWidget {
    Q_OBJECT

public:
    explicit SparklineWidget(QWidget* parent = nullptr);
};

} // namespace ny::ui::widgets

#endif // NY_SPARKLINE_WIDGET_HPP
