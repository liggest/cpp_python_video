#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPainter>

class VideoWidget : public QWidget
{

public:
	explicit VideoWidget(QWidget *parent = nullptr);
	void setImage(const QImage &image);
	void setImageAndSize(const QImage& image);

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	QImage m_image;
};

#endif // VIDEOWIDGET_H