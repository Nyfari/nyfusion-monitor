#pragma once
/**
 * @file LineChartWidget.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 * @brief Gráfico de linha completo com grade, glow e histórico rolling.
 *        Suporta série secundária opcional (ex: CPU temp no gráfico GPU).
 */
#ifndef NY_LINE_CHART_WIDGET_HPP
#define NY_LINE_CHART_WIDGET_HPP

#include "LineAreaChartWidget.hpp"

namespace ny::ui::widgets {

class LineChartWidget : public LineAreaChartWidget {
    Q_OBJECT

public:
    explicit LineChartWidget(QWidget* parent = nullptr);
};

} // namespace ny::ui::widgets

#endif // NY_LINE_CHART_WIDGET_HPP
