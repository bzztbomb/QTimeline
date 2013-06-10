/*
 *  QTimelineMenu.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "QTimelineMenu.h"
#include "QTimeline.h"

void QTimelineMenuItem::init()
{
  mHeight = 0.0f;
  
  mFont   = QTimeline::getPtr()->getFontSmall();
}