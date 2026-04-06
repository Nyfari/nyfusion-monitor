#pragma once
/**
 * @file LineAreaChartWidget.hpp
 * @brief Base compartilhada para gráficos de linha com área preenchida.
 */
#ifndef NY_LINE_AREA_CHART_WIDGET_HPP
#define NY_LINE_AREA_CHART_WIDGET_HPP

#include <QColor>
#include <QString>
#include <QWidget>
#include <deque>

namespace ny::ui::widgets {

class LineAreaChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit LineAreaChartWidget(QWidget* parent = nullptr);

    void setColor(const QColor& color);
    void setTitle(const QString& title);
    void setMaxPoints(int maxPoints);
    void addPoint(float value);
    void clearPoints();

    void setShowTitle(bool enabled);
    void setShowGrid(bool enabled);
    void setShowCurrentValue(bool enabled);
    void setRoundedBackground(bool enabled);
    void setCurrentValueSuffix(const QString& suffix);
    void setCurrentValueDecimals(int decimals);
    void setLineWidth(float width);
    void setChartMargins(float left, float right, float top, float bottom);
    void setPlotPadding(float top, float bottom);
    void setTitleHeight(float height);
    void setRangeConfig(float minVisualRange, float marginRatio, float minMargin);
    void setChartSizeHints(const QSize& preferred, const QSize& minimum);
    void setFillOpacity(int topAlpha, int bottomAlpha);
    void setGlowOpacity(int outerAlpha, int innerAlpha);

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QColor m_color { 0, 191, 255 };
    QString m_title;
    QString m_valueSuffix;
    int m_maxPts { 60 };
    int m_valueDecimals { 1 };
    bool m_showTitle { false };
    bool m_showGrid { false };
    bool m_showCurrentValue { false };
    bool m_roundedBackground { true };
    float m_lineWidth { 1.5f };
    float m_marginLeft { 3.0f };
    float m_marginRight { 3.0f };
    float m_marginTop { 3.0f };
    float m_marginBottom { 4.5f };
    float m_plotPaddingTop { 0.0f };
    float m_plotPaddingBottom { 0.0f };
    float m_titleHeight { 0.0f };
    float m_minVisualRange { 0.5f };
    float m_marginRatio { 0.10f };
    float m_minMargin { 0.1f };
    int m_fillTopAlpha { 72 };
    int m_fillBottomAlpha { 5 };
    int m_outerGlowAlpha { 30 };
    int m_innerGlowAlpha { 65 };
    QSize m_sizeHint { 120, 40 };
    QSize m_minimumSizeHint { 60, 24 };
    std::deque<float> m_data;
};

} // namespace ny::ui::widgets

#endif // NY_LINE_AREA_CHART_WIDGET_HPP
