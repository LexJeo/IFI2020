#include "naobehavior.h"
#include "../rvdraw/rvdraw.h"
#include <cmath>
#include <vector>

vector<double> opponentFront;
int dajiao_kick_straight_ok,dajiao_kick_leftside_ok,dajiao_kick_rightside_ok;

bool cmp(pair<double,VecPosition> a,pair<double,VecPosition> b)
{
    return a.first < b.first;
}

void NaoBehavior::findOppo()
{
    double ballXposition = ball.getX();
    double ballYposition = ball.getY();
    opponentFront.clear();
    dajiao_kick_straight_ok = 1;
    for(int i = WO_OPPONENT1; i < WO_OPPONENT1+NUM_AGENTS; ++i) 
    {
        WorldObject* opponent = worldModel->getWorldObject( i );
        VecPosition oppoPosition = opponent->pos;
        double myAngle = worldModel->getMyAngDeg();
        SIM::Point2D oppoVel = opponent->absVel;
        SIM::Point2D ballVel = worldModel->getWorldObject(WO_BALL)->absVel;
        VecPosition localOppo = worldModel->g2l(oppoPosition);
        SIM::AngDeg localOppoAngle = atan2Deg(localOppo.getY(), localOppo.getX());
        double oppoXposition = (opponent->pos).getX();
        double oppoYposition = (opponent->pos).getY();
        if (opponent->validPosition) 
        {
            double distance2oppo = ball.getDistanceTo(oppoPosition);
            if(ballXposition - oppoXposition < 0 && distance2oppo < 2)
                opponentFront.push_back(atanDeg((oppoYposition - ballYposition)/(oppoXposition - ballXposition)));
            if(distance2oppo < 1.5)
            {
                dajiao_kick_straight_ok = 0;
            }
            if((abs(myAngle) > 90 && distance2oppo < 2 && abs(localOppoAngle) < 70))
            {
                dajiao_kick_straight_ok = 0;  //不能向前开大脚 
            }
            if(ballXposition > 5.0 && abs(myAngle) < 90 && distance2oppo > 1.5 && abs(localOppoAngle) < 45)
                dajiao_kick_straight_ok = 1;
            if((abs(ballVel.getX())<0.05 && abs(ballVel.getY())<0.1) && abs(oppoVel.getX())<0.1 && abs(oppoVel.getY())<0.1 && abs(myAngle) < 90)
            {
                dajiao_kick_straight_ok = 1;
            }
        } 
        else 
        {
            continue;
        }
    }
}

SkillType NaoBehavior::onball()
{   
    double distance2oppo,angle2oppo;
    SIM::Point2D ballVel = worldModel->getWorldObject(WO_BALL)->absVel;//球速
    VecPosition localBall = worldModel->g2l(ball);
    SIM::AngDeg localBallAngle = atan2Deg(localBall.getY(), localBall.getX());
    double max_angle_left = 0;
    double max_angle_right = 0;
    double final_angle;
    double speed = 1.0;
    opponentFront.clear();
    double ballXposition = ball.getX();
    double ballYposition = ball.getY();
    double distance2goal,angle2goal;
    findOppo();
    //我方边线球
    if(((worldModel->getPlayMode() == PM_FREE_KICK_LEFT || worldModel->getPlayMode() == PM_KICK_IN_LEFT || worldModel->getPlayMode() == PM_CORNER_KICK_LEFT) && worldModel->getSide() == SIDE_LEFT) || ((worldModel->getPlayMode() == PM_FREE_KICK_RIGHT || worldModel->getPlayMode() == PM_KICK_IN_RIGHT || worldModel->getPlayMode() == PM_CORNER_KICK_RIGHT) && worldModel->getSide() == SIDE_RIGHT))
	{   
        //角球
        if(worldModel->getPlayMode() == PM_CORNER_KICK_LEFT || worldModel->getPlayMode() == PM_CORNER_KICK_RIGHT)
        {
            return ShortKick(VecPosition(12.0,0.0,0.0));
        }
        //边线球
        else
        {
            //后场
            if(ball.getX()<0.0)
            {
                if(ball.getY()>=8.0)
                {
                    return LongKick(VecPosition(10.5,0,0.0));
                }
                else if(ball.getY()<-8.0)
                {
                    return LongKick(VecPosition(10.5,0.0,0.0));
                }
                else
                {
                    return LongKick(VecPosition(15.0,0.0,0.0));
                }
            }
            //前场
            else
            {
                if(ball.getY()<-8.0)
                {
                    return LongKick(VecPosition(11.5,0.0,0.0)); 
                }
                else if(ball.getY()>8.0)
                {
                    return LongKick(VecPosition(11.0,0.0,0.0)); 
                }
                else
                {
                    return ShortKick(VecPosition(10.5,0,0));
                }
            }
        }
    }
    //对方边线球
    else if(((worldModel->getPlayMode() == PM_FREE_KICK_RIGHT || worldModel->getPlayMode() == PM_KICK_IN_RIGHT || worldModel->getPlayMode() == PM_CORNER_KICK_RIGHT) && worldModel->getSide() == SIDE_LEFT) || ((worldModel->getPlayMode() == PM_FREE_KICK_LEFT || worldModel->getPlayMode() == PM_KICK_IN_LEFT || worldModel->getPlayMode() == PM_CORNER_KICK_LEFT) && worldModel->getSide() == SIDE_RIGHT))
    {
        return SKILL_STAND;
    }
    else if((worldModel->getPlayMode() == PM_DIRECT_FREE_KICK_RIGHT && worldModel->getSide() == SIDE_RIGHT) || (worldModel->getPlayMode() == PM_DIRECT_FREE_KICK_LEFT && worldModel->getSide() == SIDE_LEFT))
    {
        getTargetDistanceAndAngle(VecPosition(15.0,0.0,0.0),distance2goal,angle2goal);
        if(distance2goal > 4)
            return LongKick(VecPosition(15.2,0.5,0));
        else
            return ShortKick(VecPosition(15,0,0));
    }
    //找到对方球员离球角度
    for(int i=0;i<(int)opponentFront.size();i++)
    {
        if(opponentFront[i]<0)
        {   
            max_angle_right = min(max_angle_right,opponentFront[i]);
        }
        else
        {
            max_angle_left = max(max_angle_left,opponentFront[i]);
        }
    }
    //前场策略
    if(ballXposition >= 0.0)
    {
        if(dajiao_kick_straight_ok == 1)
        {
            getTargetDistanceAndAngle(VecPosition(15.0,0.0,0.0),distance2goal,angle2goal);
            if(distance2goal <= 12.0 && distance2goal >= 4.0)
                return LongKick(VecPosition(15.0,0.3,0.0));
            else if(distance2goal < 4.0 && abs(ballYposition)>3)
                return ShortKick(VecPosition(16,0,0));
            else if(distance2goal < 4.0 && abs(ballYposition)<=3)
            {
                double distance,angle;
                getTargetDistanceAndAngle(VecPosition(15,0,0),distance,angle);
                if(abs(angle)>10)
                    return goToTargetRelative(localBall,angle);
                else
                    return goToTarget(ball);
            }
            else
            {
                for(int i = WO_TEAMMATE2; i < WO_TEAMMATE1+NUM_AGENTS; ++i) 
                {
                    WorldObject* teammate = worldModel->getWorldObject(i);
                    if(worldModel->getUNum() != i) // not myself
                    {
                        if(teammate->validPosition) 
                        {
                            if((teammate->pos).getX() - ballXposition > 3)
                            {
                                return LongKick(teammate->pos);
                            }
                        } 
                        else 
                        {
                            continue;
                        }
                    }
                }
                if(abs(worldModel->getMyAngDeg() - max_angle_right) < abs(worldModel->getMyAngDeg() - max_angle_left)) //转向小的一侧带
                {
                    final_angle = max_angle_right-10;
                    if(final_angle<-90)
                        final_angle = -90;
                    if(me.getDistanceTo(ball)>1)
                    {
                        double distance, angle;
                        getTargetDistanceAndAngle(ball, distance, angle);
                        if (abs(angle) > 10) 
                        {
                            return goToTargetRelative(localBall, angle);
                        } 
                        else 
                        {
                            return getWalk(0, 0,1.0, false);
                        }
                    }
                    else
                    {
                        double distance2, angle2;
                        getTargetDistanceAndAngle(ball, distance2, angle2);
                        if (abs(angle2) > 10) 
                        {
                            return goToTargetRelative(localBall, angle2);
                        } 
                        else 
                        {
                            if(abs(ballYposition) > 8 && ballXposition < 12)
                            {
                                final_angle = 0;
                            }
                            else if(ballXposition >=12 && ballYposition > 0)
                            {
                                final_angle = -90;
                            }
                            else if(ballXposition >=12 && ballYposition < 0)
                            {
                                final_angle = 90;
                            }
                            while(abs(worldModel->getMyAngDeg()-final_angle)>5)
                            {
                                return goToTargetRelative(localBall, final_angle-worldModel->getMyAngDeg());
                            }
                            return getWalk(0,0,1.0, false);
                        }
                    }
                }
                else
                {
                    final_angle = max_angle_left+10;
                    if(final_angle>90)
                        final_angle = 90;
                    if(me.getDistanceTo(ball)>1)
                    {
                        double distance, angle;
                        getTargetDistanceAndAngle(ball, distance, angle);
                        if(abs(angle) > 10) 
                        {
                            return goToTargetRelative(localBall, angle);
                        } 
                        else 
                        {
                            return getWalk(0, 0,1.0, false);
                        }
                    }
                    else
                    {
                        double distance2, angle2;
                        getTargetDistanceAndAngle(ball, distance2, angle2);
                        if (abs(angle2) > 10) 
                        {
                            return goToTargetRelative(localBall, angle2);
                        } 
                        else 
                        {
                            if(abs(ballYposition) > 8 && ballXposition < 12)
                            {
                                final_angle = 0;
                                speed = 0.8;
                            }
                            else if(ballXposition >=12 && ballYposition > 0)
                            {
                                final_angle = -90;
                                speed = 0.8;
                            }
                            else if(ballXposition >=12 && ballYposition < 0)
                            {
                                final_angle = 90;
                                speed = 0.8;
                            }
                            while(abs(worldModel->getMyAngDeg()-final_angle)>5)
                            {
                                return goToTargetRelative(localBall, final_angle-worldModel->getMyAngDeg(),speed);
                            }
                            return getWalk(0,0,1.0, false);
                        }
                    }
                }
            }
        }
        else  //无法开大脚
        {
            getTargetDistanceAndAngle(VecPosition(15.0,0.0,0.0), distance2goal, angle2goal);
            //离球太远，大多情况下如果采用带球转向的话，很有可能还没拿到球就被对方抢走球
            //而且动作会产生延迟导致撞上对手，被判犯规离场
            //因此采用直接走向球硬怼的策略，仅是去年通过大量实验得出的粗糙判断，并不是最优策略
            if(distance2goal < 4.0)
            {
                double distance, angle;
                getTargetDistanceAndAngle(VecPosition(15,0,0), distance, angle);
                if(abs(angle)>10)
                    return goToTargetRelative(localBall,angle);
                else
                    return goToTarget(ball);
            }
            //离球够近，足以完成带球转向动作，接下来的if-else判断向左转还是向右转
            if(abs(worldModel->getMyAngDeg() - max_angle_right) < abs(worldModel->getMyAngDeg() - max_angle_left)) //转向小的一侧带
            {
                final_angle = max_angle_right-10;
                if(final_angle<-90)
                    final_angle = -90;
                if(me.getDistanceTo(ball)>1)
                {
                    double distance, angle;
                    getTargetDistanceAndAngle(ball, distance, angle);
                    if (abs(angle) > 10) 
                    {
                        return goToTargetRelative(localBall, angle);
                    } 
                    else 
                    {
                        return getWalk(0, 0,1.0, false);
                    }
                }
                else
                {
                    double distance2, angle2;
                    getTargetDistanceAndAngle(ball, distance2, angle2);
                    if(abs(angle2) > 10) 
                    {
                        return goToTargetRelative(localBall, angle2);
                    } 
                    else 
                    {
                        if(abs(ballYposition) > 8 && ballXposition < 12)
                        {
                            final_angle = 0;
                            speed = 0.8;
                        }
                        else if(ballXposition >=12 && ballYposition > 0)
                        {
                            final_angle = -90;
                            speed = 0.8;
                        }
                        else if(ballXposition >=12 && ballYposition < 0)
                        {
                            final_angle = 90;
                            speed = 0.8;
                        }
                        while(abs(worldModel->getMyAngDeg()-final_angle)>5)
                        {
                            return goToTargetRelative(localBall, final_angle-worldModel->getMyAngDeg(),speed);
                        }
                        return getWalk(0,0,1.0, false);
                    }
                }
            }
            else
            {
                final_angle = max_angle_left+10;
                if(final_angle>90)
                    final_angle = 90;
                if(me.getDistanceTo(ball)>1)
                {
                    double distance, angle;
                    getTargetDistanceAndAngle(ball, distance, angle);
                    if(abs(angle) > 10) 
                    {
                        return goToTargetRelative(localBall, angle);
                    } 
                    else 
                    {
                        return getWalk(0, 0,1.0, false);
                    }
                }
                else
                {
                    double distance2, angle2;
                    getTargetDistanceAndAngle(ball, distance2, angle2);
                    if (abs(angle2) > 10) 
                    {
                        return goToTargetRelative(localBall, angle2);
                    } 
                    else 
                    {
                        if(abs(ballYposition) > 8 && ballXposition < 12)
                        {
                            final_angle = 0;
                            speed = 0.8;
                        }
                        else if(ballXposition >=12 && ballYposition > 0)
                        {
                            final_angle = -90;
                            speed = 0.8;
                        }
                        else if(ballXposition >=12 && ballYposition < 0)
                        {
                            final_angle = 90;
                            speed = 0.8;
                        }
                        while(abs(worldModel->getMyAngDeg()-final_angle)>5)
                        {
                            return goToTargetRelative(localBall, final_angle-worldModel->getMyAngDeg(),speed);
                        }
                        return getWalk(0,0,1.0, false);
                    }
                }
            }
        }
    }
    //后场策略
    else
    {
        //如果能开大脚，在后场就直接开大脚
        //这也不是最优的策略，因为判断是否能开大脚的条件很粗糙，并不鲁棒，只是去年采取的最保险的策略
        if(dajiao_kick_straight_ok == 1)
        {
            return LongKick(VecPosition(15,0,0));
        }
        else
        {
            //解围
            if(ball.getX()<-12 && abs(ball.getY()) < 5)
            {
                if(worldModel->getMyAngDeg() > -90 && worldModel->getMyAngDeg() < 90)
                    return LongKick(VecPosition(ball.getX()+10*cos(worldModel->getMyAngDeg()),ball.getY()+10*sin(worldModel->getMyAngDeg()),0));
            }
            //带球转向
            if(abs(worldModel->getMyAngDeg() - max_angle_right) < abs(worldModel->getMyAngDeg() - max_angle_left)) //转向小的一侧带
            {
                final_angle = max_angle_right-10;
                if(final_angle<-90)
                    final_angle = -90;
                if(me.getDistanceTo(ball)>1)
                {
                    double distance, angle;
                    getTargetDistanceAndAngle(ball, distance, angle);
                    if (abs(angle) > 10) 
                    {
                        return goToTargetRelative(localBall, angle);
                    } 
                    else 
                    {
                        return getWalk(0, 0,1.0, false);
                    }
                }
                else
                {
                    double distance2, angle2;
                    getTargetDistanceAndAngle(ball, distance2, angle2);
                    if (abs(angle2) > 10) 
                    {
                        return goToTargetRelative(localBall, angle2);
                    } 
                    else 
                    {
                        if(abs(ballYposition) > 8 && ballXposition < 12)
                        {
                            final_angle = 0;
                            speed = 0.8;
                        }
                        else if(ballXposition >=12 && ballYposition > 0)
                        {
                            final_angle = -90;
                            speed = 0.8;
                        }
                        else if(ballXposition >=12 && ballYposition < 0)
                        {
                            final_angle = 90;
                            speed = 0.8;
                        }
                        while(abs(worldModel->getMyAngDeg()-final_angle)>5)
                        {
                            return goToTargetRelative(localBall, final_angle-worldModel->getMyAngDeg(),speed);
                        }
                        return getWalk(0,0,1.0, false);
                    }
                }
            }
            else
            {
                final_angle = max_angle_left+10;
                if(final_angle>90)
                    final_angle = 90;
                if(me.getDistanceTo(ball)>1)
                {
                    double distance, angle;
                    getTargetDistanceAndAngle(ball, distance, angle);
                    if(abs(angle) > 10) 
                    {
                        return goToTargetRelative(localBall, angle);
                    } 
                    else 
                    {
                        return getWalk(0, 0,1.0, false);
                    }
                }
                else
                {
                    double distance2, angle2;
                    getTargetDistanceAndAngle(ball, distance2, angle2);
                    if (abs(angle2) > 10) 
                    {
                        return goToTargetRelative(localBall, angle2);
                    } 
                    else 
                    {
                        if(abs(ballYposition) > 8 && ballXposition < 12)
                        {
                            final_angle = 0;
                            speed = 0.8;
                        }
                        else if(ballXposition >=12 && ballYposition > 0)
                        {
                            final_angle = -90;
                            speed = 0.8;
                        }
                        else if(ballXposition >=12 && ballYposition < 0)
                        {
                            final_angle = 90;
                            speed = 0.8;
                        }
                        while(abs(worldModel->getMyAngDeg()-final_angle)>5)
                        {
                            return goToTargetRelative(localBall, final_angle-worldModel->getMyAngDeg(),speed);
                        }
                        return getWalk(0,0,1.0, false);
                    }
                }
            }
        }
    }   
    return LongKick(VecPosition(15,0,0));
}
