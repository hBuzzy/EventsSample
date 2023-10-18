#include "paintwidget.h"

#include <QDateTime>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

PaintWidget::PaintWidget(QWidget *parent) : QWidget(parent) {
  isDrawingWeb_ = false;

  ellipseMoveTimer_ = new QTimer(this);
  AddToTimerInterval(1000);

  int positionYDivider = 2;
  windowRect_ = rect();
  ellipsePosition_ =
      QPoint(windowRect_.x(), windowRect_.height() / positionYDivider);

  // Cоединяем сигнал таймера о том, что время (интервал) прошло со слотом
  // вывода сообщения
  connect(ellipseMoveTimer_, &QTimer::timeout, this, &PaintWidget::ShowMessage);

  // Cоединяем сигнал таймера о том, что время (интервал) прошло со слотом
  // изменения позиции фигуры
  connect(ellipseMoveTimer_, &QTimer::timeout, this,
          &PaintWidget::UpdateEllipsePosition);
}

PaintWidget::~PaintWidget() {}

void PaintWidget::ShowMessage() { qDebug() << QDateTime::currentDateTime(); }

void PaintWidget::UpdateEllipsePosition() {
  int dx = 10;

  ellipsePosition_.setX(ellipsePosition_.x() + dx);

  repaint();  // Принудительно вызываем событие отрисовки
}

void PaintWidget::keyPressEvent(QKeyEvent *event) {
  int deltaTimerInterval = 50;

  if (event->key() == Qt::Key_Space &&
      event->modifiers() == Qt::ControlModifier) {
    ellipseMoveTimer_->start();
  } else if (event->key() == Qt::Key_Left) {
    AddToTimerInterval(deltaTimerInterval);
  } else if (event->key() == Qt::Key_Right) {
    AddToTimerInterval(-deltaTimerInterval);
  }
}

void PaintWidget::resizeEvent(QResizeEvent *event) {
  Q_UNUSED(event);
  windowRect_ = rect();
}

void PaintWidget::mousePressEvent(QMouseEvent *event) {
  // совершаем действия только при нажатии левой кнопки мыши
  if (event->button() == Qt::LeftButton) {
    cursorPosition_ = event->pos();  // Запоминаем локальное положение мыши
    isDrawingWeb_ = true;
    ShowMousePosition();
    setCursor(Qt::CrossCursor);  // При рисовании паутины меняем курсок на крест
    repaint();  // Принудительно вызываем событие отрисовки
  }
}

void PaintWidget::mouseMoveEvent(QMouseEvent *event) {
  if (isDrawingWeb_) {
    cursorPosition_ = event->pos();  // Запоминаем локальное положение мыши
    ShowMousePosition();
    repaint();  // Принудительно вызываем событие отрисовки
  }
}

void PaintWidget::mouseReleaseEvent(QMouseEvent *event) {
  Q_UNUSED(event)  // Убираем сообщение о неиспользуемой переменной event
  isDrawingWeb_ = false;
  unsetCursor();  // Как только заканчиваем рисовать паутину, меняем курсор на
                  // стандартный
}

void PaintWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  // Создаем экземпляр класса для низкоуровневого рисования
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);  // Добавляем сглаживание

  if (isDrawingWeb_) {  // Рисуем паутину и паука при зажатой левой кнопке мыши
    DrawWeb(&painter);
    DrawSpider(&painter);
  }

  if (ellipseMoveTimer_->isActive()) {
    DrawEllipse(&painter);
  }

  // Для очистки отрисованной области
  // painter.eraseRect(rect());
}

void PaintWidget::DrawWeb(QPainter *painter) {
  painter->setPen(QPen(Qt::black, 1, Qt::SolidLine));

  // Второй способ получения координат мыши, можно использовать вне событий
  // В отличии от event->pos() возвращает глобальные координаты
  // Которые в нашем случае нужно перевести в локальные
  // cursorPosition_ = mapFromGlobal(QCursor::pos());

  int halthDivider = 2;

  painter->drawLine(QPoint(windowRect_.topLeft()), cursorPosition_);
  painter->drawLine(QPoint(windowRect_.topRight()), cursorPosition_);
  painter->drawLine(QPoint(windowRect_.bottomRight()), cursorPosition_);
  painter->drawLine(QPoint(windowRect_.bottomLeft()), cursorPosition_);
  painter->drawLine(QPoint(windowRect_.width() / halthDivider, windowRect_.y()),
                    cursorPosition_);
}

void PaintWidget::DrawSpider(QPainter *painter) {
  int width = 5;
  int height = 5;

  painter->setBrush(QBrush(Qt::red, Qt::SolidPattern));
  painter->drawEllipse(cursorPosition_, width, height);
}

void PaintWidget::DrawEllipse(QPainter *painter) {
  int width = 20;
  int height = 20;

  painter->setBrush(QBrush(Qt::green));
  painter->drawEllipse(ellipsePosition_, width, height);
}

void PaintWidget::ShowMousePosition() {
  // Выводим положение мыши в заголовок окна
  setWindowTitle(
      QString("x: %1; y: %2;")
          .arg(cursorPosition_.x())  // аргумент подставляется вместо %1
          .arg(cursorPosition_.y()));  // аргумент подставляется вместо %2
}

void PaintWidget::AddToTimerInterval(int milliseconds) {
  int minInterval = 100;
  int maxInterval = 1000;

  // Добавляем к текущему интервалу мс для дальнейшей проверки
  int newInterval = ellipseMoveTimer_->interval() + milliseconds;

  // Не даем выйти значению интерваля за указанные границы [minInterval,
  // maxInterval]
  newInterval = qBound(minInterval, newInterval, maxInterval);

  ellipseMoveTimer_->setInterval(newInterval);
}
