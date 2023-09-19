#include "VideoWidget.h"


VideoWidget::VideoWidget(QWidget* parent) : QWidget(parent) {
	setImage(QImage(256, 256, QImage::Format::Format_ARGB32));
}

void VideoWidget::setImage(const QImage& image)
{
	m_image = image;
	update();
}

void VideoWidget::copyImage()
{
	m_image = m_image.copy();
}

void VideoWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.drawImage(0, 0, m_image);
}