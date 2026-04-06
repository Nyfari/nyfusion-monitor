#pragma once
/**
 * @file LoadBarWidget.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 * @brief Barra horizontal de carga com glow e label de percentual.
 */
#ifndef NY_LOAD_BAR_WIDGET_HPP
#define NY_LOAD_BAR_WIDGET_HPP

#include <QColor>
#include <QWidget>

namespace ny::ui::widgets {

class LoadBarWidget : public QWidget {
    Q_OBJECT

public:
    explicit LoadBarWidget(QWidget* parent = nullptr);

    void setBarColor(const QColor& color);
    void setPercent(float percent); // 0.0 – 100.0

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QColor m_color   { 0xFF, 0x8C, 0x00 };
    float  m_percent { 0.0f };
};

} // namespace ny::ui::widgets

#endif // NY_LOAD_BAR_WIDGET_HPP

