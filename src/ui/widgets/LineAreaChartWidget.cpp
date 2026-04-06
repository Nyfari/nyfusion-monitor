#include "LineAreaChartWidget.hpp"

#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QRectF>
#include <algorithm>
#include <cmath>

namespace ny::ui::widgets {

namespace {

std::vector<float> smoothSeries(const std::deque<float>& data)
{
    std::vector<float> result(data.begin(), data.end());
    if (result.size() < 3) return result;

    std::vector<float> smoothed = result;
    for (size_t i = 1; i + 1 < result.size(); ++i) {
        smoothed[i] = (result[i - 1] + result[i] + result[i + 1]) / 3.0f;
    }

    return smoothed;
}

std::vector<QPointF> mapSeriesToPlotPoints(
    const std::vector<float>& data,
    const QRectF& plotRect,
    float minValue,
    float maxValue)
{
    std::vector<QPointF> points;
    const int n = static_cast<int>(data.size());
    if (n < 2 || plotRect.width() <= 1.0 || plotRect.height() <= 1.0) return points;

    points.reserve(static_cast<size_t>(n));

    const float left = static_cast<float>(plotRect.left());
    const float width = static_cast<float>(plotRect.width());
    const float top = static_cast<float>(plotRect.top());
    const float bottom = static_cast<float>(plotRect.bottom());
    const float height = static_cast<float>(plotRect.height());
    const float range = std::max(1.0f, maxValue - minValue);

    for (int i = 0; i < n; ++i) {
        const float t = (n <= 1) ? 0.0f : static_cast<float>(i) / static_cast<float>(n - 1);
        const float x = left + t * width;

        float normalized = (data[static_cast<size_t>(i)] - minValue) / range;
        normalized = std::clamp(normalized, 0.0f, 1.0f);

        const float y = bottom - normalized * height;
        points.emplace_back(x, y);
    }

    return points;
}

QPainterPath buildSmoothLinePath(const std::vector<QPointF>& points)
{
    QPainterPath path;
    if (points.empty()) return path;

    if (points.size() == 1) {
        path.moveTo(points.front());
        return path;
    }

    path.moveTo(points.front());

    if (points.size() == 2) {
        path.lineTo(points.back());
        return path;
    }

    for (size_t i = 1; i < points.size(); ++i) {
        const QPointF& prev = points[i - 1];
        const QPointF& curr = points[i];
        const QPointF mid((prev.x() + curr.x()) * 0.5, (prev.y() + curr.y()) * 0.5);
        path.quadTo(prev, mid);
    }

    path.lineTo(points.back());
    return path;
}

void drawLineAreaSeries(
    QPainter& painter,
    const std::vector<QPointF>& points,
    const QColor& color,
    const QRectF& plotRect,
    float lineWidth,
    int fillTopAlpha,
    int fillBottomAlpha,
    int outerGlowAlpha,
    int innerGlowAlpha)
{
    if (points.size() < 2) return;

    const QPainterPath linePath = buildSmoothLinePath(points);

    QPainterPath fillPath = linePath;
    const float baseY = static_cast<float>(plotRect.bottom());
    fillPath.lineTo(points.back().x(), baseY);
    fillPath.lineTo(points.front().x(), baseY);
    fillPath.closeSubpath();

    QLinearGradient grad(plotRect.left(), plotRect.top(), plotRect.left(), plotRect.bottom());

    QColor fillTop = color;
    fillTop.setAlpha(std::clamp(fillTopAlpha, 0, 255));

    QColor fillBottom = color;
    fillBottom.setAlpha(std::clamp(fillBottomAlpha, 0, 255));

    grad.setColorAt(0.0, fillTop);
    grad.setColorAt(0.75, QColor(fillTop.red(), fillTop.green(), fillTop.blue(), std::max(6, fillBottomAlpha + 8)));
    grad.setColorAt(1.0, fillBottom);

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(grad);
    painter.drawPath(fillPath);
    painter.restore();

    QColor outerGlow = color;
    outerGlow.setAlpha(std::clamp(outerGlowAlpha, 0, 255));
    painter.setPen(QPen(outerGlow, lineWidth + 3.0f, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(linePath);

    QColor innerGlow = color;
    innerGlow.setAlpha(std::clamp(innerGlowAlpha, 0, 255));
    painter.setPen(QPen(innerGlow, lineWidth + 1.2f, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(linePath);

    painter.setPen(QPen(color, lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(linePath);
}

} // namespace

LineAreaChartWidget::LineAreaChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void LineAreaChartWidget::setColor(const QColor& color) { m_color = color; update(); }
void LineAreaChartWidget::setTitle(const QString& title) { m_title = title; update(); }
void LineAreaChartWidget::setMaxPoints(int maxPoints) {
    m_maxPts = qMax(2, maxPoints);
    while (static_cast<int>(m_data.size()) > m_maxPts) m_data.pop_front();
    update();
}
void LineAreaChartWidget::addPoint(float value) {
    m_data.push_back(value);
    while (static_cast<int>(m_data.size()) > m_maxPts) m_data.pop_front();
    update();
}
void LineAreaChartWidget::clearPoints() { m_data.clear(); update(); }
void LineAreaChartWidget::setShowTitle(bool enabled) { m_showTitle = enabled; update(); }
void LineAreaChartWidget::setShowGrid(bool enabled) { m_showGrid = enabled; update(); }
void LineAreaChartWidget::setShowCurrentValue(bool enabled) { m_showCurrentValue = enabled; update(); }
void LineAreaChartWidget::setRoundedBackground(bool enabled) { m_roundedBackground = enabled; update(); }
void LineAreaChartWidget::setCurrentValueSuffix(const QString& suffix) { m_valueSuffix = suffix; update(); }
void LineAreaChartWidget::setCurrentValueDecimals(int decimals) { m_valueDecimals = decimals; update(); }
void LineAreaChartWidget::setLineWidth(float width) { m_lineWidth = width; update(); }
void LineAreaChartWidget::setChartMargins(float left, float right, float top, float bottom) {
    m_marginLeft = left;
    m_marginRight = right;
    m_marginTop = top;
    m_marginBottom = bottom;
    update();
}
void LineAreaChartWidget::setPlotPadding(float top, float bottom) {
    m_plotPaddingTop = top;
    m_plotPaddingBottom = bottom;
    update();
}
void LineAreaChartWidget::setTitleHeight(float height) { m_titleHeight = height; update(); }
void LineAreaChartWidget::setRangeConfig(float minVisualRange, float marginRatio, float minMargin) {
    m_minVisualRange = minVisualRange;
    m_marginRatio = marginRatio;
    m_minMargin = minMargin;
    update();
}
void LineAreaChartWidget::setChartSizeHints(const QSize& preferred, const QSize& minimum) {
    m_sizeHint = preferred;
    m_minimumSizeHint = minimum;
}
void LineAreaChartWidget::setFillOpacity(int topAlpha, int bottomAlpha) {
    m_fillTopAlpha = std::clamp(topAlpha, 0, 255);
    m_fillBottomAlpha = std::clamp(bottomAlpha, 0, 255);
    update();
}
void LineAreaChartWidget::setGlowOpacity(int outerAlpha, int innerAlpha) {
    m_outerGlowAlpha = std::clamp(outerAlpha, 0, 255);
    m_innerGlowAlpha = std::clamp(innerAlpha, 0, 255);
    update();
}

QSize LineAreaChartWidget::sizeHint() const { return m_sizeHint; }
QSize LineAreaChartWidget::minimumSizeHint() const { return m_minimumSizeHint; }

void LineAreaChartWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const float W = static_cast<float>(width());
    const float H = static_cast<float>(height());

    if (m_roundedBackground) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0x08, 0x12, 0x1E));
        p.drawRoundedRect(QRectF(0, 0, W, H), 4.0, 4.0);
    } else {
        p.fillRect(rect(), QColor(0x0A, 0x14, 0x20));
    }

    const float titleOffset = m_showTitle ? m_titleHeight : 0.0f;
    const QRectF plotRect(
        m_marginLeft,
        titleOffset + m_marginTop + m_plotPaddingTop,
        W - m_marginLeft - m_marginRight,
        H - titleOffset - m_marginTop - m_marginBottom - m_plotPaddingTop - m_plotPaddingBottom
    );

    if (m_showTitle && !m_title.isEmpty()) {
        p.setPen(QPen(QColor(0x14, 0x24, 0x38), 1));
        p.drawLine(QPointF(m_marginLeft * 0.6f, m_titleHeight - 3.0f), QPointF(W - m_marginRight, m_titleHeight - 3.0f));

        QColor dotGlow = m_color;
        dotGlow.setAlpha(45);
        p.setPen(Qt::NoPen);
        p.setBrush(dotGlow);
        p.drawEllipse(QPointF(m_marginLeft - 10.0f, m_titleHeight * 0.5f), 5.5, 5.5);
        p.setBrush(m_color);
        p.drawEllipse(QPointF(m_marginLeft - 10.0f, m_titleHeight * 0.5f), 3.0, 3.0);

        QFont tf;
        tf.setFamily("Inter");
        tf.setPixelSize(qMax(9, static_cast<int>(H * 0.038)));
        tf.setBold(true);
        tf.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
        p.setFont(tf);
        p.setPen(QColor(0x80, 0x9A, 0xB2));
        p.drawText(
            QRectF(m_marginLeft, 0, plotRect.width() * 0.75f, m_titleHeight - 2.0f),
            Qt::AlignVCenter | Qt::AlignLeft,
            m_title
        );
    }

    if (m_data.size() < 2) {
        p.setPen(QColor(0x28, 0x40, 0x54));
        p.drawText(plotRect, Qt::AlignCenter, m_showTitle ? "Aguardando dados..." : "?");
        return;
    }

    const std::vector<float> series = smoothSeries(m_data);

    float minV = *std::min_element(series.begin(), series.end());
    float maxV = *std::max_element(series.begin(), series.end());
    float span = maxV - minV;

    if (span < m_minVisualRange) {
        const float center = (maxV + minV) * 0.5f;
        minV = center - (m_minVisualRange * 0.5f);
        maxV = center + (m_minVisualRange * 0.5f);
        span = m_minVisualRange;
    }

    const float margin = qMax(m_minMargin, span * m_marginRatio);
    const float rangeMin = minV - margin;
    const float rangeMax = maxV + margin;

    if (m_showGrid) {
        QFont gf;
        gf.setFamily("Inter");
        gf.setPixelSize(qMax(7, static_cast<int>(H * 0.028)));
        p.setFont(gf);

        p.setPen(QPen(QColor(0x16, 0x28, 0x3A), 1));
        p.drawLine(QPointF(plotRect.left(), plotRect.top()), QPointF(plotRect.left(), plotRect.bottom()));

        for (int gi = 1; gi <= 4; ++gi) {
            const float frac = static_cast<float>(gi) / 4.0f;
            const float y = static_cast<float>(plotRect.bottom()) - frac * static_cast<float>(plotRect.height());
            const float val = rangeMin + frac * (rangeMax - rangeMin);

            p.setPen(QPen(QColor(0x14, 0x26, 0x38), 1, Qt::DashLine));
            p.drawLine(QPointF(plotRect.left() + 1.0f, y), QPointF(plotRect.right(), y));

            p.setPen(QColor(0x3E, 0x5A, 0x72));
            p.drawText(
                QRectF(0, y - 9.0f, m_marginLeft - 6.0f, 18.0f),
                Qt::AlignRight | Qt::AlignVCenter,
                QString::number(static_cast<int>(val))
            );
        }
    }

    const std::vector<QPointF> points = mapSeriesToPlotPoints(series, plotRect, rangeMin, rangeMax);

    drawLineAreaSeries(
        p,
        points,
        m_color,
        plotRect,
        m_lineWidth,
        m_fillTopAlpha,
        m_fillBottomAlpha,
        m_outerGlowAlpha,
        m_innerGlowAlpha
    );

    if (m_showCurrentValue && !points.empty()) {
        const float lastVal = m_data.back();
        const QPointF lastPt = points.back();

        QFont vf;
        vf.setFamily("Inter");
        vf.setPixelSize(qMax(8, static_cast<int>(H * 0.036)));
        vf.setBold(true);
        p.setFont(vf);

        const QString valStr = QString::number(lastVal, 'f', m_valueDecimals) + m_valueSuffix;
        QFontMetrics vfm(vf);
        const float tw = vfm.horizontalAdvance(valStr) + 10.0f;
        const float th = vfm.height() + 4.0f;
        const float px = qMin(static_cast<float>(lastPt.x()) - 2.0f,
                              static_cast<float>(plotRect.right()) - tw - 2.0f);
        const float py = qMax(static_cast<float>(plotRect.top()) + 2.0f,
                              static_cast<float>(lastPt.y()) - th - 3.0f);

        QColor bg(0x0A, 0x14, 0x20, 210);
        p.setBrush(bg);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(QRectF(px, py, tw, th), 3, 3);

        p.setPen(m_color);
        p.setBrush(Qt::NoBrush);
        p.drawText(QRectF(px, py, tw, th), Qt::AlignCenter, valStr);
    }
}

} // namespace ny::ui::widgets