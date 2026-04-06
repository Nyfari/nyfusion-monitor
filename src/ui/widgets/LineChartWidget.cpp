#include "LineChartWidget.hpp"

namespace ny::ui::widgets {

LineChartWidget::LineChartWidget(QWidget* parent)
    : LineAreaChartWidget(parent)
{
    setShowTitle(true);
    setShowGrid(false);
    setShowCurrentValue(true);
    setRoundedBackground(true);
    setCurrentValueSuffix(QStringLiteral("\u00B0"));
    setCurrentValueDecimals(1);
    setLineWidth(1.5f);
    setFillOpacity(24, 2);
    setGlowOpacity(12, 28);
    setTitleHeight(24.0f);
    setChartMargins(10.0f, 10.0f, 4.0f, 8.0f);
    setPlotPadding(2.0f, 2.0f);
    setRangeConfig(2.0f, 0.10f, 0.15f);
    setChartSizeHints(QSize(300, 150), QSize(120, 70));
    setMaxPoints(120);
}

} // namespace ny::ui::widgets
