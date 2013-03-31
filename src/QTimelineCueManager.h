/*
 *  QTimelineCueManager.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_CUE_MANAGER
#define QTIMELINE_CUE_MANAGER

#pragma once

#include "QTimelineWidget.h"
#include "QTimelineCue.h"

#define TIMELINE_CUELIST_HEIGHT     25


class QTimelineCueManager : public QTimelineWidget
{
    
    friend class QTimelineCue;
    
public:
    
    QTimelineCueManager();
    
    ~QTimelineCueManager()
    {
        clear();
    }
    
    void clear()
    {
        mCueList.clear();
        
        mCurrentCue.reset();
    }
    
    void render();
    
    bool mouseMove( ci::app::MouseEvent event );
    
    bool mouseDown( ci::app::MouseEvent event );
    
    bool mouseUp( ci::app::MouseEvent event );
    
    bool mouseDrag( ci::app::MouseEvent event );
    
    void addCue( std::string name, double startTime, double duration );
    
    void deleteCue( QTimelineCue *cue );
    
    std::vector<QTimelineCueRef> getCueList() { return mCueList; }
    
    QTimelineCueRef getCurrentCue() { return mCurrentCue; }
  
    void setNextCue(QTimelineCue* nextCue);
  
    bool playCue( int cueN = -1 );
    
    bool isTimeOnCue();
    
    double getCueStartTime() { return mCurrentCue->getStartTime(); }
    
    double getCueEndTime() { return mCurrentCue->getEndTime(); }
    
    ci::XmlTree getXmlNode();
    
    void loadXmlNode( ci::XmlTree node );
    
private:
    
    void menuEventHandler( QTimelineMenuItemRef item );
    
    void initMenu();
    
private:
    
    std::vector<QTimelineCueRef>    mCueList;
    QTimelineCueRef                 mCurrentCue;
    
};

#endif


