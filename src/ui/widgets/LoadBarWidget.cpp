/**
 * @file LoadBarWidget.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */
#include "LoadBarWidget.hpp"

#include <QPainter>
#include <algorithm>

namespace ny::ui::widgets {

LoadBarWidget::LoadBarWidget(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void LoadBarWidget::setBarColor(const QColor& c) { m_color = c; update(); }
void LoadBarWidget::setPercent(float p)           { m_percent = std::clamp(p, 0.0f, 100.0f); update(); }

QSize LoadBarWidget::sizeHint() const        { return { 200, 18 }; }
QSize LoadBarWidget::minimumSizeHint() const { return { 80, 14 }; }

void LoadBarWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const float W = static_cast<float>(width());
    const float H = static_cast<float>(height());
    const float r = H * 0.5f;

    // Track
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0x0A, 0x18, 0x26));
    p.drawRoundedRect(QRectF(0, 0, W, H), r, r);

    if (m_percent > 0.01f) {
        const float fillW = (m_percent / 100.0f) * W;

        // Glow layers
        for (int gi = 3; gi >= 1; --gi) {
            QColor gc = m_color; gc.setAlpha(18 * gi);
            p.setBrush(gc);
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(QRectF(0, -gi * 2.2f, fillW, H + gi * 4.4f),
                              r + gi * 1.4f, r + gi * 1.4f);
        }

        // Gradient fill: darker at start, brighter at tip
        QLinearGradient grad(0, 0, fillW, 0);
        QColor c1 = m_color; c1.setAlpha(150);
        grad.setColorAt(0.0, c1);
        grad.setColorAt(1.0, m_color);
        p.setBrush(grad);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(QRectF(0, 0, fillW, H), r, r);

        // Bright end-cap highlight
        if (fillW > H) {
            QColor tip = m_color.lighter(135);
            tip.setAlpha(190);
            p.setBrush(tip);
            p.drawEllipse(QRectF(fillW - H * 0.5f - H * 0.5f, 0, H, H));
        }
    }
}

} // namespace ny::ui::widgets

