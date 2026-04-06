#include "SparklineWidget.hpp"

namespace ny::ui::widgets {

SparklineWidget::SparklineWidget(QWidget* parent)
    : LineAreaChartWidget(parent)
{
    setShowTitle(false);
    setShowGrid(false);
    setShowCurrentValue(false);
    setRoundedBackground(true);
    setLineWidth(1.5f);
    setChartMargins(3.0f, 3.0f, 3.0f, 4.5f);
    setPlotPadding(0.0f, 0.0f);
    setRangeConfig(0.5f, 0.10f, 0.05f);
    setChartSizeHints(QSize(120, 40), QSize(60, 24));
    setMaxPoints(60);
}

} // namespace ny::ui::widgets

