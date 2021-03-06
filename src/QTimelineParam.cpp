/*
 *  QTimelineParam.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#include "cinder/Utilities.h"

#include "QTimeline.h"
#include "QTimelineParam.h"

using namespace ci;
using namespace ci::app;
using namespace std;


bool sortKeyframesHelper(QTimelineKeyframeRef a, QTimelineKeyframeRef b) { return ( a->getTime() < b->getTime() ); }


QTimelineParam::QTimelineParam( QTimelineModuleItem *module, const std::string &name, float *var, float minVal, float maxVal, double startTime )
: mParentModule(module), mVar(var), mMax(maxVal), mMin(minVal), QTimelineWidget(name)
{
    mBgColor                = QTimeline::mParamsBgCol;
    mBgOverColor            = QTimeline::mParamsBgOverCol;
    mTextColor              = QTimeline::mParamsTextCol;
    
    mKeyframesBgCol         = QTimeline::mKeyframesBgCol;
    mKeyframesBgOverCol     = QTimeline::mKeyframesBgOverCol;
    mKeyframesBgSelectedCol = QTimeline::mKeyframesBgSelectedCol;
    mKeyframesGraphCol      = QTimeline::mKeyframesGraphCol;
    
    mIsOnSelection  = false;
    mMousePos       = Vec2f::zero();
    mMouseDownPos   = Vec2f::zero();
    
    mDefaultEasing  = EaseNone();
    
    initMenu();
}


QTimelineParam::~QTimelineParam()
{
    for( size_t k=0; k < mKeyframes.size(); k++ )
        mKeyframes[k].reset();
    
    if ( mMenu )
        mParentModule->mParentTrack->mQTimeline->closeMenu( mMenu );
}


void QTimelineParam::update( double relativeTime )
{
    if ( mKeyframes.size() == 0 )
        return;
    
    if ( mKeyframes.size() == 1 )
    {
        *mVar = mKeyframes[0]->getValue();
        return;
    }
    
    double time = relativeTime * mParentModule->getDuration() + mParentModule->getStartTime();
    
    Vec2f                   keyframePos, prevKeyframePos, keyframeSize;
    QTimelineKeyframeRef    prevKeyframe;
    QTimelineKeyframeRef    keyframe;
    float                   deltaT, fnValue;
    
    // update keyframes
    for( size_t k=0; k < mKeyframes.size()-1; k++ )
    {
        if ( time > mKeyframes[k]->getTime() - 0.1f && time <= mKeyframes[k+1]->getTime() + 0.1f )
        {
            prevKeyframe    = mKeyframes[k];
            keyframe        = mKeyframes[k+1];
            break;
        }
    }
    
    // no pair, return!
    if ( !keyframe )
    {
        if ( time <= mKeyframes.front()->getTime() )
            *mVar = mKeyframes.front()->getValue();
        
        else if ( time >= mKeyframes.back()->getTime() )
            *mVar = mKeyframes.back()->getValue();
        
        return;
    }
    
    deltaT  = ( time - prevKeyframe->getTime() ) / ( keyframe->getTime() - prevKeyframe->getTime() );
    fnValue = math<float>::clamp( keyframe->mFn( deltaT ), 0.0f, 1.0f );
    *mVar   = prevKeyframe->getValue() + fnValue * ( keyframe->getValue() - prevKeyframe->getValue() );
}


Rectf QTimelineParam::render( Rectf prevElementRect )
{
    Rectf paramRect( prevElementRect.getLowerLeft(), prevElementRect.getLowerRight() + Vec2f(0, TIMELINE_PARAM_HEIGHT) );
    setRect( paramRect );
    
    gl::color( mBgColor );
    gl::drawSolidRect( paramRect );
    gl::color( mTextColor );
    mFont->drawString( getName(), paramRect.getCenter() + mNameStrSize * Vec2f( -0.5f, 0.3f ) );
    gl::color( mTextColor );
    
    if ( mIsOnSelection )
    {
        Rectf r( math<float>::clamp( mMouseDownPos.x, mRect.x1, mRect.x2 ), mRect.y1,
                math<float>::clamp( mMousePos.x, mRect.x1, mRect.x2 ),     mRect.y2 );
        
        gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.1f ) );
        gl::drawSolidRect( r );
    }
    
    gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.08f ) );
    glBegin( GL_LINES );
    gl::vertex( mRect.getUpperLeft() );
    gl::vertex( mRect.getLowerLeft() );
    gl::vertex( mRect.getUpperRight() );
    gl::vertex( mRect.getLowerRight() );
    glEnd();
    
    return mRectPaddedHeight;
}


void QTimelineParam::renderKeyframes()
{
    Vec2f                   keyframePos, prevKeyframePos, keyframeWindowSize, plot;
    QTimelineKeyframeRef    keyframe, prevKeyframe;
    double                  startTime   = mParentModule->getStartTime();
    double                  endTime     = mParentModule->getEndTime();
    
    Vec2f timeWindow    = mParentModule->mParentTrack->mQTimeline->getTimeWindow();
    float timeWidth     = mParentModule->mParentTrack->mQTimeline->getTimeBarWidth();
    float oneSecInPx    = timeWidth / ( timeWindow.y - timeWindow.x );
    Rectf actualRect(   ( mParentModule->getStartTime() - timeWindow.x ) * oneSecInPx,
                        mRect.y1,
                        ( mParentModule->getStartTime() + mParentModule->getDuration() - timeWindow.x ) * oneSecInPx,
                        mRect.y2 );
    
    gl::color( mKeyframesGraphCol );
    
    glBegin( GL_LINE_STRIP );
    
    for( size_t k=0; k < mKeyframes.size(); k++ )
    {
        keyframe    = mKeyframes[k];
        keyframePos = getKeyframePos( keyframe );
        //        keyframePos = keyframe->getAbsolutePosition( actualRect, mMax, mMin, startTime, endTime );

        if ( k == 0 && keyframePos.x > mRect.x1 )
        {
            gl::vertex( Vec2f( actualRect.x1, keyframePos.y ) );
            gl::vertex( Vec2f( math<float>::clamp( keyframePos.x, actualRect.x1, actualRect.x2 ), keyframePos.y ) );
        }
        
        else if ( k > 0 )
        {
            prevKeyframe    = mKeyframes[k-1];
            prevKeyframePos = prevKeyframe->getAbsolutePosition( actualRect, mMax, mMin, startTime, endTime );
            
            keyframeWindowSize.x  = keyframePos.x - prevKeyframePos.x;
            keyframeWindowSize.y  = keyframePos.y - prevKeyframePos.y;
            
            for( int relPos = 0; relPos < keyframeWindowSize.x; relPos ++ )
            {
                plot.x  = relPos;
                plot.y  = keyframe->mFn( relPos / keyframeWindowSize.x ) * keyframeWindowSize.y;
                plot    += prevKeyframePos;
                
                if ( mRect.contains( plot ) )
                    gl::vertex( plot );
            }
        }
        
        if ( k == mKeyframes.size() - 1 && keyframePos.x < mRect.x2 )
        {
            gl::vertex( Vec2f( math<float>::clamp( keyframePos.x, mRect.x1, mRect.x2 ), keyframePos.y ) );
            gl::vertex( Vec2f( mRect.x2, keyframePos.y ) );
        }
    }
    
    glEnd();
    
    
    // render keyframes, this function can be optimized, ie don't iterate through all the keyframes(again) and don't calculate the keyframe pos
    for( size_t k=0; k < mKeyframes.size(); k++ )
    {
        keyframe    = mKeyframes[k];
        keyframePos = keyframe->getAbsolutePosition( actualRect, mMax, mMin, startTime, endTime );
        
        if ( !mRect.contains( keyframePos ) )
            continue;
        
        if ( mMouseOnKeyframe == keyframe )
        {
            gl::color( mTextColor );
            mFont->drawString( keyframe->getFormattedStr(), keyframePos + Vec2f(5,-5) );
            gl::color( mKeyframesBgOverCol );
        }
        else
            gl::color( mKeyframesBgCol );
        
        gl::drawSolidCircle( keyframePos, TIMELINE_KEYFRAME_SIZE );
        
        gl::color( mTextColor );
        gl::drawStrokedCircle( keyframePos, TIMELINE_KEYFRAME_SIZE );
    }
    
    
    gl::color( mKeyframesBgSelectedCol );
    for( size_t k=0; k < mKeyframesSelection.size(); k++ )
    {
        keyframePos = getKeyframePos( mKeyframesSelection[k] );
        if ( mRect.contains( keyframePos ) )
            gl::drawSolidCircle( keyframePos, TIMELINE_KEYFRAME_SIZE );
    }
}


void QTimelineParam::addKeyframe( double time, float value )
{
    value = math<float>::clamp( value, mMin, mMax );
    mKeyframes.push_back( QTimelineKeyframeRef( new QTimelineKeyframe( time, value, mDefaultEasing ) ) );
    
    sort( mKeyframes.begin(), mKeyframes.end(), sortKeyframesHelper );
}


void QTimelineParam::removeKeyframe( QTimelineKeyframeRef ref )
{
    for( size_t k=0; k < mKeyframes.size(); k++ )
    {
        if ( mKeyframes[k] == ref )
        {
            mKeyframes.erase( mKeyframes.begin()+k );
            return;
        }
    }
    
    sort( mKeyframes.begin(), mKeyframes.end(), sortKeyframesHelper );
}


bool QTimelineParam::mouseMove( MouseEvent event )
{
    mMouseOnKeyframe.reset();
    
    mMousePos           = event.getPos();
    
    if ( !mParentModule->isTrackOpen() || mIsOnSelection )
        return false;
    
    if ( contains( mMousePos ) )
    {
        // find the first keyframe, perhaps it should find the closest
        for( size_t k=0; k < mKeyframes.size(); k++ )
        {
            //            if ( mKeyframes[k]->getAbsolutePosition( mRect, mMax, mMin, startTime, endTime ).distance( mMousePos ) <= TIMELINE_KEYFRAME_SIZE )
            
            if ( getKeyframePos( mKeyframes[k] ).distance( mMousePos ) <= TIMELINE_KEYFRAME_SIZE * 1.5f )
            {
                mMouseOnKeyframe = mKeyframes[k];
                break;
            }
        }
        return true;
    }
    return false;
}


bool QTimelineParam::mouseUp( MouseEvent event )
{
    if ( mIsOnSelection )
    {
        mIsOnSelection  = false;
        findKeyframesInSelection();
        return false;
    }
    
    sort(mKeyframes.begin(), mKeyframes.end(), sortKeyframesHelper);
    
    return false;
}


bool QTimelineParam::mouseDown( MouseEvent event )
{
    if ( event.isRightDown() )
    {
        
        if ( mMouseOnKeyframe )                        // move or delete keyframe
        {
            removeKeyframe( mMouseOnKeyframe );
            mouseMove( event );
        }
        
        else if ( !mKeyframesSelection.empty() )
            mParentModule->mParentTrack->mQTimeline->openMenu( mMenu, event.getPos() );
        
        else
            mParentModule->mParentTrack->mQTimeline->closeMenu( mMenu );
    }
    
    else
    {
        mKeyframesSelection.clear();
        mMenu->close();
        mParentModule->mParentTrack->mQTimeline->closeMenu( mMenu );
        
        if ( event.isShiftDown() )
        {
            mIsOnSelection  = true;
            mMouseDownPos   = event.getPos();
        }
        
        else if ( !mMouseOnKeyframe )                        // move or delete keyframe
        {
            // create new keyframe
            float   time    = mParentModule->mParentTrack->mQTimeline->getTimeFromPos( event.getPos().x );
            time            = math<float>::clamp( time, mParentModule->getStartTime(), mParentModule->getEndTime() );
            float value     = getPosValue( event.getPos().y );

            addKeyframe( time, value );
            
            mouseMove( event );
        }
    }
    
    return false;
}


bool QTimelineParam::mouseDrag( MouseEvent event )
{
    mMousePos = event.getPos();
    
    if ( mMouseOnKeyframe && !mIsOnSelection )
    {
        float   time    = mParentModule->mParentTrack->mQTimeline->getTimeFromPos( mMousePos.x );
        time            = math<float>::clamp( time, mParentModule->getStartTime(), mParentModule->getEndTime() );
        float value     = getPosValue( mMousePos.y );
        
        mMouseOnKeyframe->set( time, value );
        
        sort( mKeyframes.begin(), mKeyframes.end(), sortKeyframesHelper );
        
        
        mParentModule->mParentTrack->mQTimeline->updateCurrentTime();
    }
    
    return false;
}


void QTimelineParam::updateKeyframesPos( float deltaT )
{
    for( size_t k=0; k < mKeyframes.size(); k++ )
        mKeyframes[k]->mTime += deltaT;
}


void QTimelineParam::findKeyframesInSelection()
{
    float startTime = mParentModule->mParentTrack->mQTimeline->getTimeFromPos( mMouseDownPos.x );
    double endTime  = mParentModule->mParentTrack->mQTimeline->getTimeFromPos( mMousePos.x );
    
    if ( startTime > endTime )
    {
        float t     = startTime;
        startTime   = endTime;
        endTime     = t;
    }
    
    mKeyframesSelection.clear();
    
    for( size_t k=0; k < mKeyframes.size(); k++ )
    {
        if ( mKeyframes[k]->getTime() >= startTime && mKeyframes[k]->getTime() <= endTime )
        {
            mKeyframesSelection.push_back( mKeyframes[k] );
        }
    }
    
    
}


void QTimelineParam::menuEventHandler( QTimelineMenuItem* item )
{
    if ( mKeyframesSelection.empty() )
        return;
    
    std::function<float (float)> fn;
    
    if( item->getName() == "EaseInQuad" )
        fn = EaseInQuad();
    
    else if( item->getName() == "EaseOutQuad" )
        fn = EaseOutQuad();
    
    else if( item->getName() == "EaseInOutQuad" )
        fn = EaseInOutQuad();
    
    else if( item->getName() == "EaseOutInQuad" )
        fn = EaseOutInQuad();
    
    else if( item->getName() == "EaseStep" )
        fn = EaseStep();
    
    else if( item->getName() == "EaseNone" )
        fn = EaseNone();
    
    for( size_t k=0; k < mKeyframesSelection.size(); k++ )
        mKeyframesSelection[k]->setEasing(fn);
    
    mDefaultEasing = fn;
}


Vec2f QTimelineParam::getRelPosNorm( Vec2f pos )
{
    Vec2f timeWindow    = mParentModule->mParentTrack->mQTimeline->getTimeWindow();
    float timeWidth     = mParentModule->mParentTrack->mQTimeline->getTimeBarWidth();
    float oneSecInPx    = timeWidth / ( timeWindow.y - timeWindow.x );
    Rectf actualRect(   ( mParentModule->getStartTime() - timeWindow.x ) * oneSecInPx,
                     mRect.y1,
                     ( mParentModule->getStartTime() + mParentModule->getDuration() - timeWindow.x ) * oneSecInPx,
                     mRect.y2 );
    
    return ( pos - actualRect.getUpperLeft() ) / actualRect.getSize();
}



float QTimelineParam::getPosValue( float yPos )
{
    float valNorm = math<float>::clamp( 1.0f - ( yPos - mRect.y1 ) / mRect.getHeight(), 0.0f, 1.0f );
    return getMin() + valNorm * ( getMax() - getMin() );
}


Vec2f QTimelineParam::getKeyframePos( QTimelineKeyframeRef ref )
{
    ci::Vec2f pos;
    pos.x = mParentModule->mParentTrack->mQTimeline->getPosFromTime( ref->getTime() );
    pos.y   = mRect.y2 - mRect.getHeight() * ( ref->getValue() - getMin() ) / ( getMax() - getMin() );
    return pos;
}


void QTimelineParam::initMenu()
{
    mMenu->init( "PARAM MENU" );
    
    mMenu->addItem( "EaseInQuad", "", this, &QTimelineParam::menuEventHandler );
    mMenu->addItem( "EaseOutQuad", "", this, &QTimelineParam::menuEventHandler );
    mMenu->addItem( "EaseInOutQuad", "", this, &QTimelineParam::menuEventHandler );
    mMenu->addItem( "EaseOutInQuad", "", this, &QTimelineParam::menuEventHandler );
    mMenu->addItem( "EaseStep", "", this, &QTimelineParam::menuEventHandler );
    mMenu->addItem( "EaseNone", "", this, &QTimelineParam::menuEventHandler );
}

