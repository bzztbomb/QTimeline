/*
 *  QTimelineWidget.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "QTimelineWidget.h"
#include "QTimeline.h"

QTimelineWidget::QTimelineWidget( std::string name)
{
  mFont           = QTimeline::getPtr()->getFontSmall();
  mMenu           = new QTimelineMenu();
  
  setName( name );
}

