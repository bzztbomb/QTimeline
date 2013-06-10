/*
 *  QTimelineWidget.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_WIDGET
#define QTIMELINE_WIDGET

#define TIMELINE_WIDGET_PADDING 1

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/Xml.h"

#include "QTimelineMenu.h"


typedef std::shared_ptr<class QTimelineWidget>          QTimelineWidgetRef;

class QTimelineWidget
{
    
public:
    
  QTimelineWidget( std::string name = "" );
  
    virtual ~QTimelineWidget()
    {
        if ( mMenu )
            delete mMenu;
    }
    
    virtual void setRect( ci::Rectf rect )
    {
        mRect               = rect;
        mRectPaddedHeight   = mRect.inflated( ci::Vec2f( 0, TIMELINE_WIDGET_PADDING ) );
    }
    
    virtual bool mouseMove( ci::app::MouseEvent event ) { return false; }
    virtual bool mouseDown( ci::app::MouseEvent event ) { return false; }
    virtual bool mouseUp( ci::app::MouseEvent event )   { return false; }
    virtual bool mouseDrag( ci::app::MouseEvent event ) { return false; }
    virtual bool keyDown( ci::app::KeyEvent event ) { return false; }
    
    virtual ci::XmlTree getXmlNode() { return ci::XmlTree(); }
    
    virtual void loadXmlNode( ci::XmlTree node ) {}
    
    virtual void updateLabel()
    {
        mLabel = mName;
        
        if ( mLabel == "" )
            return;
        
        int charsN = mLabel.size() * ( mRect.getWidth() ) / mNameStrSize.x;
        
        if ( charsN < mLabel.size() )
            mLabel.resize( charsN );
        
        mLabelStrSize = mFont->measureString( mLabel );
    }
    
protected:
    
    virtual void menuEventHandler( QTimelineMenuItemRef item ) {}
    
    virtual void initMenu() {}
    
    
public:
    
    bool contains( ci::Vec2f pos )
    {
        return mRect.contains( pos );
    }
    
    std::string getName() { return mName; }
    
	void        setName( std::string name )
    {
        mName           = name;
        mNameStrSize    = mFont->measureString( mName );

        updateLabel();
    }
    
    std::string getLabel() { return mLabel; }
	
    ci::Rectf   getRect() { return mRect; }
    
    ci::Rectf   getRectPaddedHeight() { return mRectPaddedHeight; }
    
    float       getHeight() { return mRect.getHeight(); }
    
    // each item has a primary color, these should be the only public methods that get or set the color
    // QTimelineWidget getter and setter should be private or protected.
    ci::ColorA getColor() { return getBgColor(); }
    void setColor( ci::ColorA col ) { return setBgColor( col ); }
        
	ci::ColorA  getBgColor() { return mBgColor; }
    void        setBgColor( ci::ColorA col )        { mBgColor      = col; }
    
    ci::ColorA  getBgOverColor() { return mBgOverColor; }
    void        setBgOverColor( ci::ColorA col )    { mBgOverColor  = col; }
    
	ci::ColorA  getFgColor() { return mFgColor; }
    void        setFgColor( ci::ColorA col )        { mFgColor      = col; }
    
    ci::ColorA  getTextColor() { return mTextColor; }
    void        setTextColor( ci::ColorA col )      { mTextColor    = col; }
	
    
protected:
    
    std::string             mName;
    ci::Vec2f               mNameStrSize;
    
    std::string             mLabel;
    ci::Vec2f               mLabelStrSize;
    
    ci::Rectf               mRect;
    ci::Rectf               mRectPaddedHeight;
    
    ci::ColorA              mBgColor;
    ci::ColorA              mBgOverColor;
    ci::ColorA              mFgColor;
    ci::ColorA              mTextColor;
    
    ci::gl::TextureFontRef	mFont;
    
    ci::Vec2f               mMouseDownPos;
    ci::Vec2f               mMousePrevPos;
    
    float                   mMouseDownStartTime;
    float                   mMouseDownEndTime;
    
    QTimelineMenu           *mMenu;
    
};


#endif