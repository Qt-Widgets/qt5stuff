/*
 * Copyright (C) 2018 Istvan Simon -- stevens37 at gmail dot com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "QyController.h"
#include "QyController_p.h"
#include "QyStyleOption.h"
#include <QToolTip>




//bool QSliderPrivate::updateHoverControl(const QPoint &pos)
//{
//    Q_Q(QSlider);
//    QRect lastHoverRect = hoverRect;
//    QStyle::SubControl lastHoverControl = hoverControl;
//    bool doesHover = q->testAttribute(Qt::WA_Hover);
//    if (lastHoverControl != newHoverControl(pos) && doesHover) {
//        q->update(lastHoverRect);
//        q->update(hoverRect);
//        return true;
//    }
//    return !doesHover;
//}

//QStyle::SubControl QSliderPrivate::newHoverControl(const QPoint &pos)
//{
//    Q_Q(QSlider);
//    QStyleOptionSlider opt;
//    q->initStyleOption(&opt);
//    opt.subControls = QStyle::SC_All;
//    QRect handleRect = q->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, q);
//    QRect grooveRect = q->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, q);
//    QRect tickmarksRect = q->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderTickmarks, q);

//    if (handleRect.contains(pos)) {
//        hoverRect = handleRect;
//        hoverControl = QStyle::SC_SliderHandle;
//    } else if (grooveRect.contains(pos)) {
//        hoverRect = grooveRect;
//        hoverControl = QStyle::SC_SliderGroove;
//    } else if (tickmarksRect.contains(pos)) {
//        hoverRect = tickmarksRect;
//        hoverControl = QStyle::SC_SliderTickmarks;
//    } else {
//        hoverRect = QRect();
//        hoverControl = QStyle::SC_None;
//    }

//    return hoverControl;
//}
//bool QSlider::event(QEvent *event)
//{
//    Q_D(QSlider);

//    switch(event->type()) {
//    case QEvent::HoverEnter:
//    case QEvent::HoverLeave:
//    case QEvent::HoverMove:
//        if (const QHoverEvent *he = static_cast<const QHoverEvent *>(event))
//            d->updateHoverControl(he->pos());
//        break;
//    case QEvent::StyleChange:
//    case QEvent::MacSizeChange:
//        d->resetLayoutItemMargins();
//        break;
//    default:
//        break;
//    }
//    return QAbstractSlider::event(event);
//}

// https://stackoverflow.com/questions/7276330/qt-stylesheet-for-custom-widget?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa


// void CustomWidget::paintEvent(QPaintEvent *)
// {
//     QStyleOption opt;
//     opt.init(this);
//     QPainter p(this);
//     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
// }



QT_BEGIN_NAMESPACE

// https://locklessinc.com/articles/sat_arithmetic/
// http://wiki.qt.io/D-Pointer

void drawQyMinimalStyle( const QyStyleOption *option, QPainter *p );


void QyControllerPrivate::init()
{
    Q_Q(QyController);
    q->setFocusPolicy(Qt::WheelFocus);
}

void QyController::initStyleOption( QyStyleOption *option) const
{
    if (!option)
        return;

    Q_D(const QyController);
    option->initFrom(this);
    option->valueDisplay = &(d->valueVector);
    option->styleData = &(d->styleData);
//    option->subControls = QStyle::SC_All; // obsolete
//    option->activeSubControls = QStyle::SC_None; // obsolete
}

QyController::QyController(QWidget *parent)
    : QyAbstractController(*new QyControllerPrivate, parent)
{
    Q_D(QyController);
    d->init();
    qDebug() << "--ctor--  QyController-new";
}

QyController::~QyController()
{
}

void QyController::resizeEvent(QResizeEvent *e)
{
    qDebug() << " ** resizeEvent";
    Q_D(QyController);
    d->recalculateStyleData(this);
    QWidget::resizeEvent(e);
}


void QyController::paintEvent(QPaintEvent * e)
{
    QPainter p(this);

//     QStyleOption opt;
//     opt.init(this);
//     style()->drawPrimitive(QStyle::PE_Frame, &opt, &p, this);

    QyStyleOption option;
    initStyleOption(&option);
    drawQyMinimalStyle(&option, &p);
    qDebug() << " ** paintEvent: " << e ;
}

void QyController::mouseDoubleClickEvent(QMouseEvent * e)
{
    Q_D(QyController);
    d->controllerTransformer.reset();
    update();
    emit valueChanged( d->controllerTransformer.getValue(0));
    if( d->emitSliderValue ) {
        const auto sliderValue = d->controllerTransformer.getSliderValue(0);
        emit sliderPositionChanged( d->invertSliderValue ? QyBase::maximumSlider - sliderValue : sliderValue );
    }
}

void QyController::mousePressEvent(QMouseEvent * e)
{
    Q_D(QyController);
    d->lastPosition = - e->localPos().y() ;
    if( e->button() == Qt::MiddleButton ) {
        e->accept();
        auto kMods = QGuiApplication::keyboardModifiers();
        int keyModsInt = ( kMods & Qt::ShiftModifier ? 1 : 0 ) + ( kMods & Qt::ControlModifier ? 2 : 0 );
        emit userEvent( d->userEventValue, keyModsInt );
        return;
    }

//    if( e->button() != Qt::LeftButton ) {
//        e->ignore();
//        return;
//    }
//    e->accept();
}

void QyController::mouseReleaseEvent(QMouseEvent * e)
{
    Q_D(QyController);
    if (e->buttons() & (~e->button()) ||
       (e->button() != Qt::LeftButton)) {
        e->ignore();
        return;
    }
    e->accept();
}

void QyController::mouseMoveEvent(QMouseEvent * e)
{
    Q_D(QyController);
    if (!(e->buttons() & Qt::LeftButton)) {
        e->ignore();
        return;
    }
    e->accept();

    if( d->pressed ) {
        d->pressed = false;
        d->lastPosition = - e->localPos().y();
    } else if( d->valueFromPoint( - e->localPos().y() ) ) {
        update();
        emit valueChanged( d->controllerTransformer.getValue(0));
        if( d->emitSliderValue ) {
            const auto sliderValue = d->controllerTransformer.getSliderValue(0);
            emit sliderPositionChanged( d->invertSliderValue ? QyBase::maximumSlider - sliderValue : sliderValue );
        }
    }
}

void QyController::wheelEvent(QWheelEvent *e)
{
    Q_D(QyController);
    e->accept();
}

QSize QyController::minimumSizeHint() const
{
    qDebug() << " ** minimumSizeHint";
    return QSize(50, 50);
}

QSize QyController::sizeHint() const
{
    qDebug() << " ** sizeHint" ;
    return QSize(100, 100).expandedTo(QApplication::globalStrut());
}

bool QyController::event(QEvent *e)
{
    return QyAbstractController::event(e);
}

void QyController::enterEvent(QEvent *e)
{
    qDebug() << " ** enterEvent: " << e ;
//    QToolTip::showText( this->pos(), "test" );
    update();
}

void QyController::leaveEvent(QEvent *e)
{
    qDebug() << " ** leaveEvent: " << e ;
    update();
}
void QyController::keyPressEvent(QKeyEvent *ev)
{
    Q_D(QyAbstractController);
    int32_t stepValue = 0;
    if( ev->modifiers() & Qt::ShiftModifier ) {
        switch (ev->key()) {
        case Qt::Key_Down:
            stepValue = - d->valueHandler.getStepValue(QyBase::STEPS::STEP_SINGLE_PLUS);
            break;
        case Qt::Key_Left:
            stepValue = - d->valueHandler.getStepValue(QyBase::STEPS::STEP_TINY_PLUS);
            break;
        case Qt::Key_Up:
            stepValue = d->valueHandler.getStepValue(QyBase::STEPS::STEP_SINGLE_PLUS);
            break;
        case Qt::Key_Right:
            stepValue = d->valueHandler.getStepValue(QyBase::STEPS::STEP_TINY_PLUS);
            break;
        case Qt::Key_PageUp:
            d->controllerTransformer.stepGiant( d->valueHandler.getStepValue(QyBase::STEPS::STEP_PAGE));
            break;
        case Qt::Key_PageDown:
            d->controllerTransformer.stepGiant( -d->valueHandler.getStepValue(QyBase::STEPS::STEP_PAGE));
            break;
        case Qt::Key_Home:
            d->controllerTransformer.reset();
            break;
        case Qt::Key_End:
            // define a flag to enable
            break;
        default:
            ev->ignore();
            return;
        }
    } else {
        switch (ev->key()) {
        case Qt::Key_Down:
            stepValue = - d->valueHandler.getStepValue(QyBase::STEPS::STEP_SINGLE);
            break;
        case Qt::Key_Left:
            stepValue = - d->valueHandler.getStepValue(QyBase::STEPS::STEP_TINY);
            break;
        case Qt::Key_Up:
            stepValue = d->valueHandler.getStepValue(QyBase::STEPS::STEP_SINGLE);
            break;
        case Qt::Key_Right:
            stepValue = d->valueHandler.getStepValue(QyBase::STEPS::STEP_TINY);
            break;
        case Qt::Key_PageUp:
            stepValue = d->valueHandler.getStepValue(QyBase::STEPS::STEP_PAGE);
            break;
        case Qt::Key_PageDown:
            stepValue = - d->valueHandler.getStepValue(QyBase::STEPS::STEP_PAGE);
            break;
        case Qt::Key_Home:
            d->controllerTransformer.reset();
            break;
        case Qt::Key_End:
            // define a flag to enable
            break;
        default:
            ev->ignore();
            return;
        }
    }
    if( stepValue ) {
        if( ! d->controllerTransformer.addSliderValue( stepValue, 0 ) ) {
            return;
        }
    }
    update();
    emit valueChanged( d->controllerTransformer.getValue(0) );
    if( d->emitSliderValue ) {
        const auto sliderValue = d->controllerTransformer.getSliderValue(0);
        emit sliderPositionChanged( d->invertSliderValue ? QyBase::maximumSlider - sliderValue : sliderValue );
    }
}

QT_END_NAMESPACE

