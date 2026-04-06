#pragma once
/**
 * @file CircularGauge.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 * @brief Gauge circular arc-based com glow, escalonável ao container.
 */
#ifndef NY_CIRCULAR_GAUGE_HPP
#define NY_CIRCULAR_GAUGE_HPP

#include <QColor>
#include <QString>
#include <QWidget>

namespace ny::ui::widgets {

class CircularGauge : public QWidget {
    Q_OBJECT

public:
    explicit CircularGauge(QWidget* parent = nullptr);

    void setGaugeColor(const QColor& color);
    void setPercent(float percent);             // 0.0 – 100.0
    void setPrimaryTitle(const QString& title); // ex: "TEMPERATURE"
    void setPrimaryValue(const QString& value); // ex: "68 C"
    void setSecondaryTitle(const QString& title);
    void setSecondaryValue(const QString& value);
    void setSecondaryUnit(const QString& unit);

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QColor  m_color   { 0, 191, 255 };
    float   m_percent { 0.0f };
    QString m_primaryTitle   { "TEMPERATURE" };
    QString m_primaryValue   { "--" };
    QString m_secondaryTitle { "CLOCK" };
    QString m_secondaryValue { "--" };
    QString m_secondaryUnit  { "" };
};

} // namespace ny::ui::widgets

#endif // NY_CIRCULAR_GAUGE_HPP

