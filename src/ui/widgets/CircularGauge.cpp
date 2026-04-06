/**
 * @file CircularGauge.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */
#include "CircularGauge.hpp"

#include <QPainter>
#include <QPen>
#include <algorithm>

namespace ny::ui::widgets {

CircularGauge::CircularGauge(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void CircularGauge::setGaugeColor(const QColor& c)     { m_color = c; update(); }
void CircularGauge::setPercent(float p)                  { m_percent = std::clamp(p, 0.0f, 100.0f); update(); }
void CircularGauge::setPrimaryTitle(const QString& t)    { m_primaryTitle = t;   update(); }
void CircularGauge::setPrimaryValue(const QString& v)    { m_primaryValue = v;   update(); }
void CircularGauge::setSecondaryTitle(const QString& t)  { m_secondaryTitle = t; update(); }
void CircularGauge::setSecondaryValue(const QString& v)  { m_secondaryValue = v; update(); }
void CircularGauge::setSecondaryUnit(const QString& u)   { m_secondaryUnit = u;  update(); }

QSize CircularGauge::sizeHint() const        { return { 240, 240 }; }
QSize CircularGauge::minimumSizeHint() const { return { 150, 150 }; }

void CircularGauge::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    const int W = width(), H = height();
    const int side = qMin(W, H);
    const double cx = W / 2.0, cy = H / 2.0;

    // ── Arc geometry ─────────────────────────────────────────────────────
    const double mg   = side * 0.065;
    const double arcW = qMax(9.0, side * 0.095);  // arco ligeiramente mais espesso

    QRectF arcRect(
        cx - side * 0.5 + mg,
        cy - side * 0.5 + mg,
        side - 2.0 * mg,
        side - 2.0 * mg
    );

    constexpr int kStartDeg = 225;
    constexpr int kSpanDeg  = 270;

    const int startQt = kStartDeg * 16;
    const int fullQt  = -kSpanDeg * 16;
    const int valQt   = -static_cast<int>((m_percent / 100.0f) * kSpanDeg * 16);

    // ── Track ─────────────────────────────────────────────────────────────
    p.setPen(QPen(QColor(0x0D, 0x1C, 0x2C), arcW, Qt::SolidLine, Qt::FlatCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(arcRect, startQt, fullQt);

    // ── Glow + value arc ──────────────────────────────────────────────────
    if (m_percent > 0.01f) {
        // 4 passes de glow com espalhamento reduzido (mais limpo, menos difuso)
        for (int gi = 4; gi >= 1; --gi) {
            QColor gc = m_color;
            gc.setAlpha(12 * gi);
            p.setPen(QPen(gc, arcW + gi * 3.5, Qt::SolidLine, Qt::FlatCap));
            p.drawArc(arcRect, startQt, valQt);
        }
        // Solid arc
        p.setPen(QPen(m_color, arcW, Qt::SolidLine, Qt::FlatCap));
        p.drawArc(arcRect, startQt, valQt);
    }

    // ── Center text area ──────────────────────────────────────────────────
    // innerR = raio utilizável para textos dentro do arco
    const double innerR = (side * 0.5 - mg - arcW) * 0.86;

    // Helper: desenha texto centralizado em (cx, yCenter)
    auto drawC = [&](const QString& text, int pxSize, bool bold,
                     const QColor& color, double yCenter) {
        QFont f;
        f.setFamily("Inter");
        f.setPixelSize(qMax(7, pxSize));
        f.setBold(bold);
        if (!bold) f.setLetterSpacing(QFont::AbsoluteSpacing, 0.7);
        p.setFont(f);
        p.setPen(color);
        const double lh = QFontMetrics(f).height() * 1.18;
        QRectF r(cx - innerR, yCenter - lh * 0.5, innerR * 2.0, lh);
        p.drawText(r, Qt::AlignHCenter | Qt::AlignVCenter, text);
    };

    // Escala de fontes — szBig reduzido para equilibrar proporção visual
    const int szTiny  = qMax(7,  static_cast<int>(side * 0.056));
    const int szSmall = qMax(8,  static_cast<int>(side * 0.073));
    const int szMed   = qMax(9,  static_cast<int>(side * 0.092));
    const int szBig   = qMax(14, static_cast<int>(side * 0.175));  // era 0.195

    // ── Layout de texto (topo → base dentro do círculo interno) ──────────
    // [primaryTitle]   cy - R*0.60   → "TEMPERATURE"  tiny, muted
    // [primaryValue]   cy - R*0.10   → "68.5°C"       BIG accent ← hero
    // [divider]        cy + R*0.26
    // [usage%]         cy + R*0.43   → "45%"          small, dimmed
    // [secondaryTitle] cy + R*0.64   → "CLOCK"        tiny, muted
    // [secondaryValue] cy + R*0.87   → "1855 MHz"     medium, white

    drawC(m_primaryTitle, szTiny, false, QColor(0x34, 0x52, 0x6A), cy - innerR * 0.60);
    drawC(m_primaryValue, szBig,  true,  m_color,                   cy - innerR * 0.10);

    // Divisor horizontal fino
    p.setPen(QPen(QColor(0x12, 0x22, 0x32), 1));
    p.drawLine(QPointF(cx - innerR * 0.52, cy + innerR * 0.26),
               QPointF(cx + innerR * 0.52, cy + innerR * 0.26));

    drawC(QString::number(static_cast<int>(m_percent)) + "%",
          szSmall, false, QColor(0x48, 0x68, 0x84), cy + innerR * 0.43);

    drawC(m_secondaryTitle, szTiny, false, QColor(0x34, 0x52, 0x6A), cy + innerR * 0.64);

    const QString secLine = m_secondaryValue
        + (m_secondaryUnit.isEmpty() ? QString() : " " + m_secondaryUnit);
    drawC(secLine, szMed, true, QColor(0xA8, 0xC4, 0xDC), cy + innerR * 0.87);
}

} // namespace ny::ui::widgets

