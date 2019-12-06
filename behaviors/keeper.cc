#include "naobehavior.h"
#include "../rvdraw/rvdraw.h"
#include <cmath>
#include <vector>
#include <math.h>
#include <iostream>
#include <algorithm>

using namespace std;

VecPosition kickTarget = VecPosition(HALF_FIELD_X,0,0); //初始踢球目标
VecPosition backTarget = VecPosition(-14.5,0); //goback目标
SIM::Point2D lastBall; //上周期球的位置
SIM::Point2D lastBallVel; //上周期球的速度
double lastBallPosY; 
double lastBallPosX;
double ProdictabilityOfBallX = 0; //预测球的X坐标
double ProdictabilityOfBallY = 0; //预测球的Y坐标
bool noDanger;
int closestplayer;

double minmax( double minx, double x, double maxx ) {
	return( min( max( minx, x ), maxx ) );
}   //判断x是否在(minx,maxx)范围内

bool NaoBehavior::isGoalKick()
{
	if((worldModel->getPlayMode() == PM_GOAL_KICK_LEFT && worldModel->getSide() == SIDE_LEFT) 
	|| (worldModel->getPlayMode() == PM_GOAL_KICK_RIGHT && worldModel->getSide() == SIDE_RIGHT))
		return true;
	else
		return false;
}	//判断是否为守门员开球状态

void NaoBehavior::ProdictabilityOfBall(SIM::Point2D ballVel) {
	double a_x,a_y,px,py;
	a_x = (ballVel.getX()-lastBallVel.getX())/0.02;
	a_y = (ballVel.getY()-lastBallVel.getY())/0.02;
	px = ballVel.getX()*0.02+0.0002*a_x;
	py = ballVel.getY()*0.02+0.0006*a_y;
	ProdictabilityOfBallX += px;
	ProdictabilityOfBallY += py;
	//cout << "rrrrr" << "(" << ProdictabilityOfBallX << "," << ProdictabilityOfBallY << ")" << endl;
}	//利用匀加速直线运动公式预测球下周期的x,y坐标值

SkillType NaoBehavior::selectKeeperSkills(VecPosition teammateball,double distance,double angle) {
	//cout << "keeper" << endl;
	VecPosition myPos = worldModel->getMyPosition();
	SIM::Point2D ballVel = worldModel->getWorldObject(WO_BALL)->absVel;//球速
    SIM::Point2D KlocalBall = worldModel->getWorldObject(WO_BALL)->loc;//球的坐标(守门员坐标系下的位置）
	double ballX = teammateball.getX();
	double ballY = teammateball.getY();	//通信获取的球的坐标
	double angleBall2OwnGoal = atan(ballY/(ballX+15));	
    closestplayer = findPlayerDistance2Ball().first;
    noDanger  = true;
	if(lastBall!=KlocalBall) {
		ProdictabilityOfBallX = ballX;
		ProdictabilityOfBallY = ballY;
	}
	else {
		lastBallVel = ballVel;
		ProdictabilityOfBall(ballVel);
	}	//refresh every cycle
	lastBall = KlocalBall;
    VecPosition proBall = VecPosition(ProdictabilityOfBallX,ProdictabilityOfBallY,0);
	if(((myPos.getX() < -15 && ballX > -14) || myPos.getX() > -(15-PENALTY_X) || myPos.getY() > PENALTY_Y || myPos.getY() < -PENALTY_Y) && closestplayer != 1) {
		//cout << "goback" << endl;
		if(std::abs(worldModel->getMyAngDeg()) > 150)
			return goToTargetRelative(VecPosition(0,0,0),-worldModel->getMyAngDeg()); //防止总是走向球网
		return goBack(myPos,angle);
	}	//handle outgoing issues
	else if(isGoalKick()) {
        	return goalKick(proBall,distance,angle,ballVel);
	}	//守门员开球
	else if(((ProdictabilityOfBallX < -10 && ballVel.getX()<-1)||
			(/*(me.getDistanceTo(proBall) < 3 || */(ProdictabilityOfBallX-lastBallPosX < -1 && lastBallPosX < -2) && std::abs(ballVel.getX() > 0.5))) 
			&& worldModel->getTime()-lastGetupRecoveryTime > 2.0 && worldModel->getPlayMode() == PM_PLAY_ON) {
		cout << ballVel << endl;
		return defend(ProdictabilityOfBallX,ProdictabilityOfBallY);
	}	//扑球
	else if(closestplayer == 1 && ((ProdictabilityOfBallX > -15 && ProdictabilityOfBallX < -13 && ProdictabilityOfBallY < 3 && ProdictabilityOfBallY > -3) 
		|| (!isPKGoal && minmax(-13,ProdictabilityOfBallX,-12)==ProdictabilityOfBallX && minmax(-4,ProdictabilityOfBallY,4)==ProdictabilityOfBallY))) {
		//cout << "yyyyyyyy" << proBall << endl;
		return walkToBall(proBall,distance,angle,ballVel);
	}	//只有在守门员为离球最近的人且球在禁区附近时，走向球
	else if(ProdictabilityOfBallX < 5) {
		return alignToBall(ProdictabilityOfBallX,ProdictabilityOfBallY,angleBall2OwnGoal,angle,ballVel);
	}	//对齐球
	else {
		return SKILL_STAND;
	}
}

SkillType NaoBehavior::goBack(VecPosition myPos,double deg) {
	double angle = -atanDeg(myPos.getY()/(myPos.getX()+15));
	if(angle > -90 && angle < 0) {
		backTarget = VecPosition(-1.5,0).rotateAboutZ(angle-10) + VecPosition(-15,0);
	}
	else if(angle >= 0 && angle < 90) {
		backTarget = VecPosition(-1.5,0).rotateAboutZ(angle+10) + VecPosition(-15,0);
	}
	else {
		backTarget = VecPosition(-14.5,0);
	}
	if(myPos.getX() > -15.6 && minmax(-1.1,myPos.getY(),1.1)==myPos.getY()) {
		backTarget = VecPosition(-14.5,0);
	}
	return goToTargetRelative(worldModel->g2l(backTarget),deg);
}

SkillType NaoBehavior::walkToBall(VecPosition mball,double distance,double angle,SIM::Point2D ballVel) {
	//cout << "walkToBall" << endl;
    if(me.getX() > mball.getX()) {
	    //cout << "zhuanshen" << endl;
        VecPosition localCenter = worldModel->g2l(mball);
        SIM::AngDeg localCenterAngle = atan2Deg(localCenter.getY(), localCenter.getX());
        VecPosition target = mball + VecPosition(0.4,0,0).rotateAboutZ(-worldModel->getMyAngDeg());
        target = collisionAvoidance(true, false, true, 1, .25, target, true);
        return goToTargetRelative(worldModel->g2l(target),localCenterAngle);
    }	//球在身后时，转身绕球走
	else if(me.getDistanceTo(mball) < 0.5) {
        if(noDanger)
        	return LongKick(kickTarget);	//大脚
        else
        	return kickBall(KICK_DRIBBLE,kickTarget);	//带球
	}
	else {
		kickTarget = findGreatKickTarget();
        //cout << "shoumen:" << kickTarget << endl;
		return goToTargetRelative(worldModel->g2l(mball),angle); //face ball
	}
}

SkillType NaoBehavior::goalKick(VecPosition mball, double distance, double angle, SIM::Point2D ballVel){
    SIM::AngDeg Ang = worldModel->getMyAngDeg();
    kickTarget = findGreatKickTarget();
    cout << kickTarget << endl;
    if(me.getDistanceTo(mball) < 0.5) {
        if(std::abs(Ang) > 60) {
            //cout << "我要踢啦！！！！！" << endl;
            kickTarget = VecPosition(me.getX()+5*cosDeg(Ang),me.getY()+5*sinDeg(Ang),0);
            return ShortKick(kickTarget);
        }
        else
            return LongKick(kickTarget);	//为了减少转身角度，防止摔倒，提高速度
    }
    else {
        return goToTargetRelative(worldModel->g2l(mball),angle);
    }
}

SkillType NaoBehavior::alignToBall(double x,double y,double angleBall2OwnGoal,double angle,SIM::Point2D ballVel) {
	VecPosition target;
    if((ballVel.getX() < -1 || std::abs(ballVel.getY()) > 1) && ProdictabilityOfBallX < -12) {
		//cout << "dddddd" << endl;
		return defend(ProdictabilityOfBallX,ProdictabilityOfBallY);
	}
	if(std::abs(angleBall2OwnGoal) < 1.4) {
		if(x > -7.5) {
			target = VecPosition(-15+cos(angleBall2OwnGoal),sin(angleBall2OwnGoal),0);
		}
		else {
			target = VecPosition(-15+0.5*cos(angleBall2OwnGoal),0.5*sin(angleBall2OwnGoal),0);
		}
	}
	else {
		target = VecPosition(-14.5,angleBall2OwnGoal/3,0);	//球靠近底线时守门员站位
	}
	//cout << "alignToBall" << endl;
	target = collisionAvoidance(true, true, true, .5, .25, target,false);
	return goToTargetRelative(worldModel->g2l(target),angle);//face ball
}

SkillType NaoBehavior::defend(double x,double y) {
    //cout << "defend" << endl;
    if(y-me.getY() < -0.3 && worldModel->getMyAngDeg() > -70) {
		//cout << "right" << endl;
		return SKILL_LEFT_BLOCK;
	}
    else if(y-me.getY() > 0.3 && worldModel->getMyAngDeg() < 70) {
		//cout << "left" << endl;
		return SKILL_RIGHT_BLOCK;
	}
	else {
		//cout << "middle" << endl;
		return SKILL_MIDDLE_BLOCK;
	}
}

VecPosition NaoBehavior::findGreatKickTarget() {
	//cout << "!!!!!!!find" << endl;
    //worldModel->getRVSender()->clearStaticDrawings();
    SIM::AngDeg Ang = worldModel->getMyAngDeg();
    if(std::abs(Ang) < 80)
        kickTarget = VecPosition(HALF_FIELD_X,0,0).rotateAboutZZ(Ang)-ball;
    else
        kickTarget = VecPosition(15,0,0);	//保证转身角度较小
	double angle = 0;
	double tempDis,tempAng;
	VecPosition OppPosition;
	for(int i=WO_OPPONENT1;i<WO_OPPONENT1+NUM_AGENTS;++i) {
		WorldObject* Opp = worldModel->getWorldObject(i);
		if(Opp->validPosition == true) {
			OppPosition = Opp->pos;
			OppPosition.setZ(0);
			getTargetDistanceAndAngle2Ball(OppPosition,tempDis,tempAng);
			if(tempDis < 1.5 && std::abs(tempAng-Ang) < 5) {
            	noDanger = false;
            	kickTarget = collisionAvoidance(true, true, false, .5, .5, kickTarget,false);
          		return kickTarget;
            }
			else if(tempDis > 3.0) {
				continue;
			}
			else {
				//worldModel->getRVSender()->drawPoint(OppPosition.getX()-1,OppPosition.getY(),10.0f,RVSender::BLUE);
				//cout << "Ang:" << tempAng << endl;
				if(minmax(tempAng-5,angle,tempAng+5) == angle) {
					if(tempAng > 0) 
						//KickTarget = KickTarget.rotateAboutZ(10.0);//顺时针旋转10度
						angle += 10.0;
					else
						//KickTarget = KickTarget.rotateAboutZ(-10.0);//逆时针旋转10度
						angle -= 10.0;
				}
			}
			//cout << "TempAng" << angle << endl;
		}
	}
    for(int i=2;i<12;++i) {
        WorldObject* Team = worldModel->getWorldObject(i);
        if(Team->validPosition == true) {
            VecPosition TeamPos = Team->pos;
            TeamPos.setZ(0);
			getTargetDistanceAndAngle2Ball(OppPosition,tempDis,tempAng);
            if(tempDis < 3.0 && std::abs(tempAng-Ang) < 5) {
            	if(tempAng > 0) 
					//KickTarget = KickTarget.rotateAboutZ(10.0);//顺时针旋转10度
					angle += 10.0;
				else
					//KickTarget = KickTarget.rotateAboutZ(-10.0);//逆时针旋转10度
					angle -= 10.0;
            }
            else {
                continue;
            }
        }
    }
	kickTarget = kickTarget.rotateAboutZZ(angle);
	return kickTarget;
}	//规避3米内敌方、己方球员

void NaoBehavior::getTargetDistanceAndAngle2Ball(const VecPosition &target, double &distance, double &angle) {
    VecPosition targetDirection = VecPosition(target) - ball;
    targetDirection.setZ(0);
    // distance
    distance = targetDirection.getMagnitude();//计算二维矢量的幅值
    // angle
    targetDirection.normalize();
    angle = VecPosition(0, 0, 0).getAngleBetweenPoints(myXDirection, targetDirection);
    if (isnan(angle)) {
        //cout << "BAD angle!\n";
        angle = 0;
    }
    if(myYDirection.dotProduct(targetDirection) < 0) {
        angle = -angle;
    }
}	//计算目标位置距球的距离和角度
