#include "VideoWidget.h"


VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent) {}

void VideoWidget::setImage(const QImage &image)
{
	m_image = image;
	update();
}

void VideoWidget::copyImage()
{
	m_image = m_image.copy();
}

void VideoWidget::setSize(const int width, const int height)
{
	resize(width, height);
}

void VideoWidget::setSize(const QImage& image)
{
	setImage(image);
	resize(m_image.width(), m_image.height());
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.drawImage(0, 0, m_image);
}