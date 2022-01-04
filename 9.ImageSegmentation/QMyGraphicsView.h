#ifndef QMYGRAPHICSVIEW_H
#define QMYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QMouseEvent>

class QMyGraphicsView : public QGraphicsView
{
	Q_OBJECT

signals:
	void updateItemPos(QPointF& pos);
	//����������źź���
public:
	QMyGraphicsView(QWidget* parent = nullptr);
protected:
	void mousePressEvent(QMouseEvent* event);
	//��д��������,ʹ�ô˺������������ź�
};

#endif // QMYGRAPHICSVIEW_H