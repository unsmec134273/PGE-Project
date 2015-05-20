/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2015 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "simple_animator.h"

//#include "logger.h"
#include <QtDebug>

SimpleAnimator::SimpleAnimator()
{
    construct(false, 1, 64, 0, -1, false, false);
}

SimpleAnimator::SimpleAnimator(const SimpleAnimator &animator)
{
    construct(animator.animated,
              animator.framesQ,
              animator.speed,
              animator.frameFirst,
              animator.frameLast,
              animator.reverce,
              animator.bidirectional);
    this->manual_ticks = animator.manual_ticks;
    this->onceMode = animator.onceMode;
    this->animationFinished = animator.animationFinished;
}

SimpleAnimator::SimpleAnimator(bool enables, int framesq, int fspeed, int First, int Last, bool rev, bool bid)
{
    construct(enables, framesq, fspeed, First, Last, rev, bid);
}

void SimpleAnimator::construct(bool enables, int framesq, int fspeed, int First, int Last, bool rev, bool bid)
{
    animated = enables;
    frameFirst = First;
    frameLast = Last;
    CurrentFrame = 0;

    frame_sequance_enabled=false;

    manual_ticks = fspeed;

    onceMode=false;
    animationFinished=false;

    isEnabled=false;

    pos1 = 0.0;
    pos2 = 1.0f;

    bidirectional = bid;
    reverce = rev;

    speed=fspeed;
    framesQ = framesq;

    setFrame(frameFirst);
}

void SimpleAnimator::setFrameSequance(QList<int> sequance)
{
    frame_sequance=sequance;
    frame_sequance_enabled=true;
    frame_sequance_cur=0;
    if(!frame_sequance.isEmpty())
    {
        CurrentFrame = frame_sequance[frame_sequance_cur];
        pos1 = CurrentFrame/framesQ;
        pos2 = CurrentFrame/framesQ + 1.0/framesQ;
        manual_ticks = speed;
    }
}

SimpleAnimator::~SimpleAnimator()
{
    this->stop();
}

bool SimpleAnimator::operator!=(const SimpleAnimator &animator) const
{
    return !(*this == animator);
}

bool SimpleAnimator::operator==(const SimpleAnimator &animator) const
{
    if(animator.speed != speed) return false;
    if(animator.animated != animated) return false;
    if(animator.framesQ != framesQ) return false;
    if(animator.frameFirst != frameFirst) return false;
    if(animator.frameLast != frameLast) return false;
    if(animator.reverce != reverce) return false;
    if(animator.bidirectional != bidirectional) return false;
    if(animator.manual_ticks != manual_ticks) return false;
    if(animator.animationFinished != animationFinished) return false;
    if(animator.onceMode != onceMode) return false;
    return true;
}

SimpleAnimator &SimpleAnimator::operator=(const SimpleAnimator &animator)
{
    this->construct(animator.animated,
              animator.framesQ,
              animator.speed,
              animator.frameFirst,
              animator.frameLast,
              animator.reverce,
              animator.bidirectional);
    return *this;
}

//Returns images

AniPos SimpleAnimator::image(double frame)
{
    if((frame<0)||(frame>=framesQ))
        return AniPos(pos1, pos2);
    else
        return AniPos(frame/framesQ, frame/framesQ + 1.0/framesQ);
}

//Animation process
void SimpleAnimator::nextFrame()
{
    if(frame_sequance_enabled)
    {
        frame_sequance_cur++;
        if(frame_sequance_cur<0)
            frame_sequance_cur=0;
        if(frame_sequance_cur>=frame_sequance.size())
            frame_sequance_cur=0;
        if(!frame_sequance.isEmpty())
            CurrentFrame = frame_sequance[frame_sequance_cur];
        goto makeFrame;
    }

    if(reverce)
    { // Reverce animation
        CurrentFrame--;
        if(CurrentFrame<frameFirst)
        {
            if(bidirectional)
            {
                reverce=!reverce; // change direction on first frame
                CurrentFrame+=2;
                if(onceMode) animationFinished=true;
            }
            else
            {
                 // Return to last frame;
                if(frameLast<0)
                    CurrentFrame=framesQ-1;
                else
                    CurrentFrame=frameLast;
                if(onceMode) animationFinished=true;
            }
        }

    }
    else
    { // Direct animation
        CurrentFrame++;
        if(((CurrentFrame>=framesQ)&&(frameLast<0))||
           ((CurrentFrame>frameLast)&&(frameLast>=0)))
        {
            if(bidirectional)
            {
                reverce=!reverce; // change direction on last frame
                CurrentFrame-=2;
            }
            else
            {
                CurrentFrame=frameFirst; // Return to first frame;
                if(onceMode) animationFinished=true;
            }
        }
    }

makeFrame:
    pos1 = CurrentFrame/framesQ;
    pos2 = CurrentFrame/framesQ + 1.0/framesQ;

    if(isEnabled)
        timer_id = SDL_AddTimer(speed, &SimpleAnimator::TickAnimation, this);
}


void SimpleAnimator::setFrame(int y)
{
    if(y>=framesQ) y= frameFirst;
    if(y<frameFirst) y = (frameLast<0)? framesQ-1 : frameLast;
    CurrentFrame = y;

    pos1 = CurrentFrame/framesQ;
    pos2 = CurrentFrame/framesQ + 1.0/framesQ;
}

void SimpleAnimator::setFrames(int first, int last)
{
    if((frameFirst == first) && (frameLast == last)) return;
        frameFirst = first;
        frameLast = last;
        setFrame(frameFirst);
}

void SimpleAnimator::start()
{
    if(!animated) return;
    if(isEnabled) return;

    if((frameLast>0)&&((frameLast-frameFirst)<=1)) return; //Don't start singleFrame animation
    isEnabled=true;
    timer_id = SDL_AddTimer(speed, &SimpleAnimator::TickAnimation, this);
}

void SimpleAnimator::stop()
{
    if(!animated) return;
    if(!isEnabled) return;
    isEnabled=false;
    SDL_RemoveTimer(timer_id);
    setFrame(frameFirst);
}

unsigned int SimpleAnimator::TickAnimation(unsigned int x, void *p)
{
    Q_UNUSED(x);
    SimpleAnimator *self = reinterpret_cast<SimpleAnimator *>(p);
    self->nextFrame();
    return 0;
}

void SimpleAnimator::setOnceMode(bool once)
{
    onceMode=once;
    animationFinished=false;
    if(once)
        setFrame(frameFirst);
}

//Ability to tick animation manually!
void SimpleAnimator::manualTick(int ticks)
{
    if(speed<1) return; //Idling animation

    manual_ticks-=abs(ticks);
        while(manual_ticks<=0)
        {
            nextFrame();
            manual_ticks+=speed;
        }
}

bool SimpleAnimator::isFinished()
{
    return animationFinished;
}


