#include "naobehavior.h"
#include "../rvdraw/rvdraw.h"
#include "../worldmodel/worldmodel.h"
#include "beam_save.h"

#include<iostream>
#include<fstream>
#include<cstdio>
#include<cmath>
#include<string>
#include<algorithm>
#include<vector>
#include<climits>
#include<float.h>
#include<string.h>
#include<ctime>
#include <boost/cstdint.hpp>

typedef uint64_t uint64;

extern int agentBodyType;



/*
 * Real game beaming.
 * Filling params x y angle
 * 开场阵型
 */
void NaoBehavior::beam( double& beamX, double& beamY, double& beamAngle ) {
    if(worldModel->getUNum()==1){
        beamX = -14.8;
        beamY = 0;
        beamAngle = 0;
       }
    else if(worldModel->getUNum()==2){
        beamX = -11;
        beamY = 0.525;
        beamAngle = 0;
       }
    else if(worldModel->getUNum()==3){
        beamX = -10;
        beamY = -0.525;
        beamAngle = 0;
       }
    else if(worldModel->getUNum()==4){
        beamX = -6;
        beamY = 4;
        beamAngle = 0;
       }
    else if(worldModel->getUNum()==5){
        beamX = -6;
        beamY = -5;
        beamAngle = 0;
       }
    else if(worldModel->getUNum()==6){
        beamX = -2.3;
        beamY = 0;
        beamAngle = 0;
       }
    else if(worldModel->getUNum()==7){
        beamX = -3;
        beamY = -1;
        beamAngle = 0;
       }
    else if(worldModel->getUNum()==8){
        beamX = -2;
        beamY = 2;
        beamAngle = 0;
       }
    else if(worldModel->getUNum()==9){
        beamX = -2;
        beamY = -2;
        beamAngle = 0;
       }
    else if(worldModel->getUNum()==10){
        beamX = -1.5;
        beamY = 7.5;
        beamAngle = 0;
       }
    else if(worldModel->getUNum()==11){
        beamX = -1.5;
        beamY = -7.5;
        beamAngle = 0;
       }
}


SkillType NaoBehavior::selectSkill()
{
    worldModel->getRVSender()->clear(); // erases drawings from previous cycle
    worldModel->getRVSender()->clearStaticDrawings();
    int Nearest2ball = findPlayerDistance2Ball().first;
    int SecondNearest2ball = findPlayerDistance2Ball().second;
    double distance,angle;
    VecPosition teammateball;
    WorldObject *pBall = worldModel->getWorldObject(WO_BALL);
    //通信得来的球坐标
    if(pBall->haveSighting==true)
    {
        teammateball=pBall->sighting;
    }
    else
    {
        teammateball=pBall->pos;
    }
    getTargetDistanceAndAngle(teammateball,distance,angle);
    //画出球的位置
    worldModel->getRVSender()->drawPoint("teammateball",teammateball.getX(),teammateball.getY(),20.0f, RVSender::PINK);

    //判断己方禁区人数，确定球员状态
    if(teammateGoalCount()==true && worldModel->getUNum() > 1 && teammateball.getX()<-13.2 && abs(teammateball.getY())<3&&(me.getX()<-14&&abs(me.getY())<1.3))
    {
        VecPosition mepos=VecPosition(me.getX()+2,me.getY(),0);
        double distance, angle;
        getTargetDistanceAndAngle(ball, distance, angle);
        return goToTargetRelative(mepos,angle);
    }

    //开球模式
    if(worldModel->getPlayMode() == PM_KICK_OFF_LEFT || worldModel->getPlayMode() == PM_KICK_OFF_RIGHT)
    {
        return kickOff();
    }
    //对方球门球
    else if(((worldModel->getPlayMode() == PM_GOAL_KICK_RIGHT && worldModel->getSide() == SIDE_LEFT) || (worldModel->getPlayMode() == PM_GOAL_KICK_LEFT && worldModel->getSide() == SIDE_RIGHT)))
    {
        if(Nearest2ball == worldModel->getUNum() && worldModel->getUNum() > 1)
        {
            return oppoGoalStrategy();
        }
        else
        {
            return goToTarget((OppoPenaltyAttackMode(ball))[worldModel->getUNum()]);
        }
    }
    //PLAY_ON mode
    else
    {
        if(Nearest2ball == worldModel->getUNum() && worldModel->getUNum() > 1)
        {
            if(worldModel->getPlayMode() == PM_GOAL_KICK_RIGHT || worldModel->getPlayMode() == PM_GOAL_KICK_LEFT)
            {
                return SKILL_STAND;
            }
            return onball();
        }
        //支持球员
        else if(SecondNearest2ball == worldModel->getUNum() && worldModel->getUNum() > 1 && onBallChange(Nearest2ball) == true)
        {
            return supporter();
        }
        //守门员
        else if(worldModel->getUNum() == 1)
        {
            isPKGoal = false;
            if(worldModel->getPlayMode() == PM_CORNER_KICK_LEFT || worldModel->getPlayMode() == PM_CORNER_KICK_RIGHT)
            {
                return goToTargetRelative(worldModel->g2l(VecPosition(-14.5,0,0)),angle);
            }
            else
            {
                return NaoBehavior::selectKeeperSkills(teammateball,distance,angle);
            }
        }
        //other players
        else
        {
            double X = teammateball.getX();  //update ball's position
            double Y = teammateball.getY();
            if(X>0.0)
            {
                current_beam = attackMode(teammateball);
            }
            else if(X<0.0 && X>-11)
            {
                current_beam = defendMode(teammateball);
            }
            else
            {
                if(abs(Y)<3.0)
                {
                    current_beam = penaltyDefendMode(teammateball);
                }
                else
                {
                    current_beam = cornerDefendModeDeam(teammateball);
                }
            }
            last_beam.clear();  //delete the data before
            for(int i = WO_TEAMMATE2; i < WO_TEAMMATE1+NUM_AGENTS; ++i)
            {
                if(i != Nearest2ball)
                {
                    last_beam.push_back(i);
                    if(last_beam.size() == 9)
                    {
                        break;
                    }
                }
            }
            for(int i=0;i<(int)last_beam.size();i++)
            {
                if(last_beam[i] == worldModel->getUNum())
                {
                    VecPosition target;   
                    //对方边线球
                    if(((worldModel->getPlayMode() == PM_DIRECT_FREE_KICK_RIGHT || worldModel->getPlayMode() == PM_FREE_KICK_RIGHT || worldModel->getPlayMode() == PM_KICK_IN_RIGHT || worldModel->getPlayMode() == PM_CORNER_KICK_RIGHT) && worldModel->getSide() == SIDE_LEFT) || ((worldModel->getPlayMode() == PM_FREE_KICK_LEFT || worldModel->getPlayMode() == PM_KICK_IN_LEFT || worldModel->getPlayMode() == PM_CORNER_KICK_LEFT || worldModel->getPlayMode() == PM_DIRECT_FREE_KICK_LEFT) && worldModel->getSide() == SIDE_RIGHT))
                    {
                        target = collisionAvoidance(true/*Avoid teammate*/,true/*Avoid opponent*/,true/*Avoid ball*/,2.0,2.0,current_beam[i], true /*fKeepDistance*/);
                    }
                    else
                    {
                        target = collisionAvoidance(true/*Avoid teammate*/,false/*Avoid opponent*/,false/*Avoid ball*/,1.0,.5,current_beam[i], true /*fKeepDistance*/);
                    }
                    if(me.getDistanceTo(target) > 0.25)
                    {
                        // Far away from the ball so walk toward target offset from the ball
                        return goToTarget(target);
                    }
                    //面向球
                    double rot,dis;
                    getTargetDistanceAndAngle(ball,dis,rot);
                    return goToTargetRelative(target,rot);
                }
            }
        }
        return goToTarget(ball);
    }
}

//开场站位
SkillType NaoBehavior::kickOff()
{
    //我方开球
    if((worldModel->getSide() == SIDE_LEFT && worldModel->getPlayMode() == PM_KICK_OFF_LEFT) || (worldModel->getSide() == SIDE_RIGHT && worldModel->getPlayMode() == PM_KICK_OFF_RIGHT))
    {
        if(worldModel->getUNum()==1)
        {
            return SKILL_STAND;        //1号守门员
        }
        else if(worldModel->getUNum()==2)
        {
            return SKILL_STAND;    //2号左后卫
        }
        else if(worldModel->getUNum()==3)
        {
            return SKILL_STAND;    //3号右后卫
        }
        else if(worldModel->getUNum()==4)
        {
            return goToTarget(VecPosition(-5.5,8.0,0.0));      //4号左前卫
        }
        else if(worldModel->getUNum()==5)
        {
            return goToTarget(VecPosition(-6.0,0.0,0.0));   //5号右前卫
        }
        else if(worldModel->getUNum()==7)
        {
            return goToTarget(VecPosition(-2.0,2.5,0.0));
        }
        else if(worldModel->getUNum()==8)
        {
            return goToTarget(VecPosition(-0.5,5.0,0.0));
        }
        else if(worldModel->getUNum()==9)
        {
            return goToTarget(VecPosition(-2.5,0.0,0.0));
        }
        else if(worldModel->getUNum()==10)
        {
            return goToTarget(VecPosition(-0.5,8.0,0.0));
        }
        else if(worldModel->getUNum()==11)
        {
            return goToTarget(VecPosition(-0.5,-2.5,0.0));
        }
        else
        {
            return LongKick(VecPosition(15,7,0.0));
        }
    }
    //对方开球
    else if((worldModel->getSide() == SIDE_LEFT && worldModel->getPlayMode() == PM_KICK_OFF_RIGHT) || (worldModel->getSide() == SIDE_RIGHT && worldModel->getPlayMode() == PM_KICK_OFF_LEFT))
    {
        if(worldModel->getUNum()==10)
        {
            return SKILL_STAND;
        }
        else if(worldModel->getUNum()==11)
        {
            return SKILL_STAND;
        }
        else if(worldModel->getUNum()==8)
        {
            return goToTarget(VecPosition(-1.5,2.0,0.0));
        }
        else if(worldModel->getUNum()==9)
        {
            return goToTarget(VecPosition(-1.5,-2.3,0.0));
        }
        else if(worldModel->getUNum()==6)
        {
            return goToTarget(VecPosition(-2.5,0.4,0.0));
        }
        else if(worldModel->getUNum()==7)
        {
            return goToTarget(VecPosition(-2.5,-0.4,0.0));
        }
        else if(worldModel->getUNum()==4)
        {
            return goToTarget(VecPosition(-7.0,3.0,0.0));
        }
        else if(worldModel->getUNum()==5)
        {
            return goToTarget(VecPosition(-7.0,-3.0,0.0));
        }
        else
        {
            return SKILL_STAND;
        }
    }
    return SKILL_STAND;
}

//前场阵型

vector<VecPosition> NaoBehavior::attackMode(VecPosition tball)
{
    deam_position.clear();
    float x = tball.getX();
    float y = tball.getY();
    double BallToMyGoal = atan(y/(x+15.0));
    int Nearest2ball = findPlayerDistance2Ball().first;
    //defend
    deam_position.push_back(VecPosition(3.0*cos(BallToMyGoal)-15.0,3.0*sin(BallToMyGoal),0));
    deam_position.push_back(VecPosition(5.0*cos(BallToMyGoal)-15.0,6.0*sin(BallToMyGoal),0));
    deam_position.push_back(VecPosition(-5.5,y,0));
    deam_position.push_back(VecPosition(7.0*cos(BallToMyGoal)-15.0,9.0*sin(BallToMyGoal),0));

    VecPosition pos = VecPosition(-2.1,0.0,0.0);

    //attack
    if(y>5.0)
    {
        deam_position.push_back(pos.rotateAboutZ(-80.0)+VecPosition(x,y,0));
        deam_position.push_back(pos.rotateAboutZ(-40.0)+VecPosition(x,y,0));
        deam_position.push_back(pos+VecPosition(x,y,0));
        deam_position.push_back(VecPosition(-2.5,y,0));
    }
    else if(y<-5.0)
    {
        deam_position.push_back(pos.rotateAboutZ(80.0)+VecPosition(x,y,0));
        deam_position.push_back(pos.rotateAboutZ(40.0)+VecPosition(x,y,0));
        deam_position.push_back(pos+VecPosition(x,y,0));
        deam_position.push_back(VecPosition(-2.5,y,0));
    }
    else
    {
        deam_position.push_back(pos.rotateAboutZ(-65.0)+VecPosition(x,y,0));
        deam_position.push_back(pos+VecPosition(x,y,0));
        deam_position.push_back(pos.rotateAboutZ(65)+VecPosition(x,y,0));
        deam_position.push_back(VecPosition(-2.5,y,0));
    }
    deam_position.push_back(VecPosition(10.5,0.0,0.0));
    for(int i=0;i<=(int)deam_position.size();i++)
    {
        if(deam_position[i].getY() >= 0.0)
        {
            if(deam_position[i].getX()  >= 15.0 && deam_position[i].getY() <10.0)
            {
                deam_position[i].setX(14.0);
            }
            else if(deam_position[i].getX()  < 15.0 && deam_position[i].getY() >=10.0)
            {
                deam_position[i].setY(9.5);
            }
            else if(deam_position[i].getX()  >= 15.0 && deam_position[i].getY() >=10.0)
            {
                deam_position[i].setX(14.0);
                deam_position[i].setY(9.5);                           //检测是否越界
            }
        }
        else
        {
            if(deam_position[i].getX()  >= 15.0 && deam_position[i].getY() <=10.0)
            {
                deam_position[i].setX(14.0);
            }
            else if(deam_position[i].getX()  < 15.0 && deam_position[i].getY() <=-10.0)
            {
                deam_position[i].setY(-9.5);
            }
            else if(deam_position[i].getX()  >= 15.0 && deam_position[i].getY() <=-10.0)
            {
                deam_position[i].setX(14.0);
                deam_position[i].setY(-9.5);
            }
        }
    }
    //drawing beam
    worldModel->getRVSender()->clearStaticDrawings();
    if(worldModel->getUNum() == Nearest2ball)  //离球最近的人定位的阵型
    {
        worldModel->getRVSender()->clear();
        worldModel->getRVSender()->clearStaticDrawings();
        for(int j=0;j<9;j++)
        {
            worldModel->getRVSender()->drawPoint(deam_position[j].getX(),deam_position[j].getY(),6.0f,RVSender::YELLOW);
        }
    }
    return deam_position;
}


//后场防守阵型

vector<VecPosition> NaoBehavior::defendMode(VecPosition tball)
{
    deam_position.clear();
    double x = tball.getX();//球横坐标
    double y = tball.getY();//球纵坐标
    double BallToMyGoal = atan(y/(x+15.0));//球与己方球门中点连线与x轴的夹角
    double distance = abs(tball.getDistanceTo(VecPosition(-15.0,0.0,0.0))-(1.8/cos(BallToMyGoal)));
    int Nearest2ball = findPlayerDistance2Ball().first;

    deam_position.push_back(VecPosition(distance*0.25*cos(BallToMyGoal)-13.2,distance*0.25*sin(BallToMyGoal)+1.8*tan(BallToMyGoal)));
    deam_position.push_back(VecPosition(distance*0.50*cos(BallToMyGoal)-13.2,distance*0.50*sin(BallToMyGoal)+1.8*tan(BallToMyGoal)));
    
    deam_position.push_back(VecPosition((distance-0.3)*cos(BallToMyGoal)-14.5,(distance-0.3)*sin(BallToMyGoal)+1.8*tan(BallToMyGoal),0));
	deam_position.push_back(VecPosition((distance-0.3)*cos(BallToMyGoal)-14,(distance-0.3)*sin(BallToMyGoal)+1.8*tan(BallToMyGoal)+0.8,0));
    deam_position.push_back(VecPosition((distance-0.3)*cos(BallToMyGoal)-14,(distance-0.3)*sin(BallToMyGoal)+1.8*tan(BallToMyGoal)-0.8,0));
     
    deam_position.push_back(VecPosition(x+6,y+1.0,0));
    deam_position.push_back(VecPosition(x+10,y+2.0,0));
    deam_position.push_back(VecPosition(x+10,y-2.0,0));
    deam_position.push_back(VecPosition(10.5,0,0));


    for(Pos=deam_position.begin();Pos!=deam_position.end();Pos++)
    {
        if((*Pos).getX() <= -13.2)
        {
            (*Pos).setX(-13.0);
        }
        else if((*Pos).getY() > 10.0)
        {
            (*Pos).setY((*Pos).getY()-5.0);
        }
        else if((*Pos).getY() < -10.0)
        {
            (*Pos).setY((*Pos).getY()+5.0);                        
        }
    }
    //drawing beam
    worldModel->getRVSender()->clearStaticDrawings();
    if(worldModel->getUNum() == Nearest2ball)  //离球最近的人定位的阵型
    {
        worldModel->getRVSender()->clear();
        worldModel->getRVSender()->clearStaticDrawings();
        for(int j=0;j<9;j++)
        {
            worldModel->getRVSender()->drawPoint(deam_position[j].getX(),deam_position[j].getY(),6.0f,RVSender::GREEN);
        }
    }
    return deam_position;
}
//己方球门球阵型
vector<VecPosition> NaoBehavior::penaltyDefendMode(VecPosition tball)
{
    double X = tball.getX();
    double Y = tball.getY();
    double distanceright = sqrt((X+13.2)*(X+13.2)+(Y-3.0)*(Y-3.0));
    double distanceleft = sqrt((X+13.2)*(X+13.2)+(Y+3.0)*(Y+3.0));
    int Nearest2ball = findPlayerDistance2Ball().first;
    deam_position.clear();
    if(Y>=0)
    {
        double angle1 = atan((Y+0.25)/(X+15.0));
        if(X <= -13.0)
        {
            if(distanceright > 0.8)
            {
                deam_position.push_back(VecPosition(X+(distanceright+0.5)*sin(angle1*0.3),Y+(distanceright+0.5)*cos(angle1*0.3),0.0));
                deam_position.push_back(VecPosition(X+(-X-13.2+1.1)*sin(angle1*0.5+0.89),Y+(-X-13.2+1.1)*cos(angle1*0.5+0.89),0.0));
                deam_position.push_back(VecPosition(X+(-X-13.2+1.5)*sin(0.7),Y-(-X-13.2+1.5)*cos(0.7),0.0));
                deam_position.push_back(VecPosition(X+(distanceleft+0.2)*sin(angle1*0.25),Y-(distanceleft+0.2)*cos(angle1*0.25),0.0));
            }
            else
            {
                deam_position.push_back(VecPosition(X+1.4*sin(0.5),Y+1.3*cos(0.5),0.0));
                deam_position.push_back(VecPosition(X+1.2*sin(1.6),Y+1.1*cos(1.6),0.0));
                deam_position.push_back(VecPosition(X+1.4*sin(0.6),Y-1.3*cos(0.6),0.0));
                deam_position.push_back(VecPosition(X+(distanceleft+0.2)*sin(angle1*0.2),Y-(distanceleft+0.2)*cos(angle1*0.2),0.0));
            }
            deam_position.push_back(VecPosition(-13,2,0.0));
        }
        else
        {
            deam_position.push_back(VecPosition(X-(distanceright+0.7)*sin(0.5),Y+(distanceright+0.7)*cos(0.5),0.0));
            deam_position.push_back(VecPosition(X,Y-0.5*(distanceleft+0.4),0.0));
            deam_position.push_back(VecPosition(X+1.3*sin(0.5),Y+1.3*cos(0.5),0.0));
            deam_position.push_back(VecPosition(X+1.3*sin(0.7),Y-1.3*cos(0.7),0.0));
            deam_position.push_back(VecPosition(-13,2,0.0));
        }
    }
    if(Y<0)
    {
        double angle2 = atan((-Y+0.25)/(X+15.0));
        if(X <= -13.0)
        {
            if(distanceleft > 0.8)
            {
                deam_position.push_back(VecPosition(X+(distanceleft+0.5)*sin(angle2*0.3),Y-(distanceleft+0.5)*cos(angle2*0.3),0.0));
                deam_position.push_back(VecPosition(X+(-X-13.2+1.1)*sin(angle2*0.5+0.3),Y-(-X-13.2+1.1)*cos(angle2*0.5+0.3),0.0));
                deam_position.push_back(VecPosition(X+(-X-13.2+1.5)*sin(0.7),Y+(-X-13.2+1.5)*cos(0.7),0.0));
                deam_position.push_back(VecPosition(X+(distanceright+0.2)*sin(angle2*0.25),Y+(distanceright+0.2)*cos(angle2*0.25),0.0));}
            else
            {
                deam_position.push_back(VecPosition(X+1.4*sin(0.5),Y-1.3*cos(0.5),0.0));
                deam_position.push_back(VecPosition(X+1.2*sin(1.6),Y-1.1*cos(1.6),0.0));
                deam_position.push_back(VecPosition(X+1.4*sin(0.6),Y+1.3*cos(0.6),0.0));
                deam_position.push_back(VecPosition(X+(distanceright+0.2)*sin(angle2*0.2),Y+(distanceright+0.2)*cos(angle2*0.2),0.0));
            }
            deam_position.push_back(VecPosition(-13,-2,0.0));
        }
        else
        {
            deam_position.push_back(VecPosition(X-(distanceleft+0.5)*sin(0.5),Y-(distanceleft+0.5)*cos(0.5),0.0));
            deam_position.push_back(VecPosition(X,Y+0.5*(distanceright+0.4),0.0));
            deam_position.push_back(VecPosition(X+1.3*sin(0.7),Y+1.3*cos(0.7),0.0));
            deam_position.push_back(VecPosition(X+1.3*sin(0.5),Y-1.3*cos(0.5),0.0));
            deam_position.push_back(VecPosition(-13,-2,0.0));
        }
    }
    deam_position.push_back(VecPosition(X+6,Y+1.0,0));
    deam_position.push_back(VecPosition(X+10,Y+2.0,0));
    deam_position.push_back(VecPosition(X+10,Y-2.0,0));
    deam_position.push_back(VecPosition(10.5,0,0));
    //draw beam
    worldModel->getRVSender()->clearStaticDrawings();
    if(worldModel->getUNum() == Nearest2ball)  //离球最近的人定位的阵型
    {
        worldModel->getRVSender()->clear();
        worldModel->getRVSender()->clearStaticDrawings();
        for(int j=0;j<9;j++)
        {
            worldModel->getRVSender()->drawPoint(deam_position[j].getX(),deam_position[j].getY(),6.0f,RVSender::GREEN);
        }
    }
    return deam_position;
}

//对方球门球
vector<VecPosition> NaoBehavior::OppoPenaltyAttackMode(VecPosition tball)
{
    deam_position.clear();
    float x = tball.getX();
    float y = tball.getY();
    double BallToMyGoal = atan(y/(x+15.0));
    int Nearest2ball = findPlayerDistance2Ball().first;
    //defend
    deam_position.push_back(VecPosition(3.0*cos(BallToMyGoal)-15.0,3.0*sin(BallToMyGoal),0));
    deam_position.push_back(VecPosition(5.0*cos(BallToMyGoal)-15.0,6.0*sin(BallToMyGoal),0));
    deam_position.push_back(VecPosition(-5.5,y,0));
    deam_position.push_back(VecPosition(7.0*cos(BallToMyGoal)-15.0,9.0*sin(BallToMyGoal),0));
    deam_position.push_back(VecPosition(-2.5,y,0));
    //attack
    deam_position.push_back(VecPosition(12.5,0.0,0.0));
    deam_position.push_back(VecPosition(12.5,1.5,0.0));
    deam_position.push_back(VecPosition(12.5,-1.5,0.0));
    deam_position.push_back(VecPosition(12.7,3.84,0.0));
    deam_position.push_back(VecPosition(10.5,0.0,0.0));
    //draw beam
    worldModel->getRVSender()->clearStaticDrawings();
    if(worldModel->getUNum() == Nearest2ball)  //离球最近的人定位的阵型
    {
        worldModel->getRVSender()->clear();
        worldModel->getRVSender()->clearStaticDrawings();
        for(int j=0;j<9;j++)
        {
            worldModel->getRVSender()->drawPoint(deam_position[j].getX(),deam_position[j].getY(),6.0f,RVSender::GREEN);
        }
    }
    return deam_position;
}

//角球阵型
vector<VecPosition> NaoBehavior::cornerDefendModeDeam(VecPosition tball)
{
    deam_position.clear();
    double x = tball.getX();//球横坐标
    double y = tball.getY();//球纵坐标
    double BallToMyGoal = atan(y/(x+15.0));//球与己方球门中点连线与x轴的夹角
    double distance = abs(tball.getDistanceTo(VecPosition(-15.0,0.0,0.0))-3.5);
    int Nearest2ball = findPlayerDistance2Ball().first;

    if(abs(y)>5.6)
    {
        deam_position.push_back(VecPosition((distance*0.25+3.5)*cos(BallToMyGoal)-15.0,(distance*0.25+3.5)*sin(BallToMyGoal)));
        deam_position.push_back(VecPosition((distance*0.50+3.5)*cos(BallToMyGoal)-15.0,(distance*0.50+3.5)*sin(BallToMyGoal)));
        deam_position.push_back(VecPosition((distance*0.85+3.5)*cos(BallToMyGoal)-1.5*sin(BallToMyGoal)-15.0,(distance*0.85+3.5)*sin(BallToMyGoal)+1.5*cos(BallToMyGoal)));
        deam_position.push_back(VecPosition((distance*0.75+3.5)*cos(BallToMyGoal)-15.0,(distance*0.75+3.5)*sin(BallToMyGoal)));
        deam_position.push_back(VecPosition((distance*0.85+3.5)*cos(BallToMyGoal)+1.5*sin(BallToMyGoal)-15.0,(distance*0.85+3.5)*sin(BallToMyGoal)-1.5*cos(BallToMyGoal)));
    }
    else
    {
        if(y>0)
        {
            deam_position.push_back(VecPosition(0.0,-2.2).rotateAboutZ(-10)+VecPosition(-15.0,5.5));
            deam_position.push_back(VecPosition(0,-2.2).rotateAboutZ(-40)+VecPosition(-15.0,5.5));
            deam_position.push_back(VecPosition(0,-2.2).rotateAboutZ(-60)+VecPosition(-15.0,5.5));
            deam_position.push_back(VecPosition(0,-2.2).rotateAboutZ(-80)+VecPosition(-15.0,5.5));
            deam_position.push_back(VecPosition(x+3,-1.3,0.0));
        }
        else
        {
            deam_position.push_back(VecPosition(0.0,2.2).rotateAboutZ(10)+VecPosition(-15.0,-5.5));
            deam_position.push_back(VecPosition(0,2.2).rotateAboutZ(40)+VecPosition(-15.0,-5.5));
            deam_position.push_back(VecPosition(0,2.2).rotateAboutZ(60)+VecPosition(-15.0,-5.5));
            deam_position.push_back(VecPosition(0,2.2).rotateAboutZ(80)+VecPosition(-15.0,-5.5));
            deam_position.push_back(VecPosition(x+3,1.3,0.0));
        }  
    }  
    deam_position.push_back(VecPosition(x+6,y+1.0,0));
    deam_position.push_back(VecPosition(x+10,y+2.0,0));
    deam_position.push_back(VecPosition(x+10,y-2.0,0));
    deam_position.push_back(VecPosition(10.5,0,0));

    for(Pos=deam_position.begin();Pos!=deam_position.end();Pos++)
    {
        if((*Pos).getX() <= -15.0)
        {
            (*Pos).setX(-14.6);
        }
        else if((*Pos).getY() > 10.0)
        {
            (*Pos).setY((*Pos).getY()-5.0);
        }
        else if((*Pos).getY() < -10.0)
        {
            (*Pos).setY((*Pos).getY()+5.0);                           //检测是否越界
        }
    }
    //draw beam
    worldModel->getRVSender()->clearStaticDrawings();
    if(worldModel->getUNum() == Nearest2ball)  //离球最近的人定位的阵型
    {
        worldModel->getRVSender()->clear();
        worldModel->getRVSender()->clearStaticDrawings();
        for(int j=0;j<9;j++)
        {
            worldModel->getRVSender()->drawPoint(deam_position[j].getX(),deam_position[j].getY(),6.0f,RVSender::GREEN);
        }
    }
    return deam_position;
}

//威胁球员标记
vector<VecPosition> NaoBehavior::findHighThreatOpponent()
{
    int clearnum=0;
    vector<VecPosition> highThreatOpponent;
    highThreatOpponent.clear();
    for(int i=1;i<12;++i)
    {
        bool stand=worldModel->isFallen();
        if(i==(worldModel->getUNum()))
        {
            int max=0;
            for(int n=WO_OPPONENT1;n<WO_OPPONENT1+NUM_AGENTS;++n)
            {
                WorldObject*oppo=worldModel->getWorldObject(n);
                if((oppo->currentlySeen=true)&&(oppo->currentlySeenOrien=true)&&(oppo->validPosition=true))
                {
                    max=max+1;
                }
            }
            WorldObject*ballvision=worldModel->getWorldObject(WO_BALL);
            if((max>3)&&(stand=true)&&(ballvision->currentlySeen=true)&&(ballvision->currentlySeenOrien=true)&&(ballvision->validPosition=true))
            {
                clearnum=i;
            }
        }
    }
    //找出离球最近的球员不标定
    int oppoclosestoball=-1;
    double oppoclosestdistoball=10000;
    for(int i=12;i<23;++i)
    {
	    VecPosition oppo=worldModel->getOpponent(i);
	    double temp=oppo.getDistanceTo(ball);
        if(temp<oppoclosestdistoball)
        {
            oppoclosestdistoball=temp;
            oppoclosestoball=i;
        }
    }
    VecPosition goalCenter=VecPosition(-15,0,0);
    if(clearnum == worldModel->getUNum())
    {
        for(int i=12;i<23; ++i)
        {
            VecPosition oppoPos= worldModel ->getOpponent(i);
            WorldObject*oppo=worldModel->getWorldObject(i);
            if(-15<oppoPos.getX()&&15>oppoPos.getX()&&oppoPos.getY()<12&&oppoPos.getY()>-12)
            {
                if(-15<oppoPos.getX()&&0>oppoPos.getX())
                {
                    if(i!=oppoclosestoball)
                    {
                        double markX= (((oppoPos.getDistanceTo(goalCenter)-1)/(oppoPos.getDistanceTo(goalCenter)))*(oppoPos.getX()+15)-15);
                        double markY= (((oppoPos.getDistanceTo(goalCenter)-1)/(oppoPos.getDistanceTo(goalCenter)))*oppoPos.getY());
                        oppoPos.setX(markX);
                        oppoPos.setY(markY);
                    }
                }
                else
                {
                    if(ball.getX()>-13)
                    {
                        double markX=ball.getX()-1/(ball.getDistanceTo(goalCenter))*(ball.getX()+15);
                        double markY=ball.getY()+1/(ball.getDistanceTo(goalCenter))*(-ball.getY());
                        oppoPos.setX(markX);
                        oppoPos.setY(markY);
                        highThreatOpponent.push_back(oppoPos);
                        double markX1=oppoPos.getX()+(ball.getDistanceTo(oppoPos)+0.5)/ball.getDistanceTo(oppoPos)*(oppoPos.getX()-ball.getX());
                        double markY1=oppoPos.getY()-(ball.getDistanceTo(oppoPos)+0.5)/ball.getDistanceTo(oppoPos)*(ball.getY()-oppoPos.getY());
                        oppoPos.setX(markX1);
                        oppoPos.setY(markY1);
                        highThreatOpponent.push_back(oppoPos);
                    }
                }
            }
        }
    }
    return highThreatOpponent;
}

//威胁球员分配
/*vector<VecPosition> NaoBehavior::highThreatOpponentDelivery(int n,vector<VecPosition> oppo,vector<VecPosition> current_beam)
{
    vector<Edge1> edge;
    vector<Answer>answer;
    Test t;
    answer.clear();
    for(int i=0;i<n;i++)
    {
        t.starts.push_back(make_pair(oppo[i].getX(),oppo[i].getY()));
    }
    for(int i=0;i<9;i++)
    {
        t.targets.push_back(make_pair(current_beam[i].getX(),current_beam[i].getY()));
    }
    for(int i=0;i<n;i++)
        for(int j=0;j<9;j++)
            edge.push_back(make_pair(getdist(t.starts[i],t.targets[j]),(i*100+j)));
    double x;
    int y;
    for(int i=0;i<(n*9);i++)             //排序 
    {
        for(int j=i+1;j<(n*9);j++)
        {
            if(edge[i].first>edge[j].first)
            {
                x=edge[i].first;edge[i].first=edge[j].first;edge[j].first=x;
                y=edge[i].second;edge[i].second=edge[j].second;edge[j].second=y;
            }
        }
    }
    int k=0;
    int l=0;
    for(int i=0;i<(n*9);i++)
    {
        if(i==0)
        {
            current_beam[edge[i].second%100] = oppo[edge[i].second/100];
            answer.push_back(make_pair(edge[i].second/100,edge[i].second%100));
            k+=1;
        }
        else
        {
            for(l=0;l<k;l++)
            {
                int t,v;
                t=edge[i].second/100;
                v=edge[i].second%100;
                if((answer[l].first==t)|(answer[l].second==v))break;
            }
            if(l>=k)
            {
                current_beam[edge[i].second%100] = oppo[edge[i].second/100];
                answer.push_back(make_pair(edge[i].second/100,edge[i].second%100));
                k+=1;
            }
            if(k==n)break;
        }
    }
    t.starts.clear();
    t.targets.clear();
    return current_beam;
}*/

bool NaoBehavior::teammateGoalCount()
{
	int counts=0;
	for(int i=1;i<12;++i)
	{
		VecPosition temp=worldModel->getTeammate(i);
		if(temp.getX()<-13.2 && temp.getY()<1.05 && temp.getY()>-1.05)
		{
			++counts;
		}
	}
	VecPosition myPos=worldModel->getMyPosition();
	if(counts>1&&myPos.getX()<-12.5&&myPos.getY()<3.5&&myPos.getY()>-3.5)
	{
		return true;
	}
	else 
	{
		return false;  
	}
}    










