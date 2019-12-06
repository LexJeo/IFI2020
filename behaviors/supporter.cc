#include "naobehavior.h"


bool NaoBehavior::onBallChange(int onBall)
{
    if(worldModel->getFallenTeammate(onBall-1) == true) //onball摔倒
    {  
        return true;
    }
    else
    {
        return false;
    }	
}

bool cmp(pair<double,int> a,pair<double,int> b)
{
    return a.first < b.first;
}


pair<int,int> NaoBehavior::findPlayerDistance2Ball()
{
    VecPosition temp;
    int secondPlayer;
    double dis;
	pair<double,int> allPlayerDis2Ball;
    vector<pair<double,int> > distance;
	pair<int,int> firstTwoPlayer;
    for(int i = WO_TEAMMATE1; i < WO_TEAMMATE1+NUM_AGENTS; ++i) 
    {
        int playerNum = i - WO_TEAMMATE1 + 1;
        if (worldModel->getUNum() == playerNum) 
        {
            //当前机器人进程
            temp = worldModel->getMyPosition();
			temp.setZ(0);
            dis = temp.getDistanceTo(ball);
	        distance.push_back(make_pair(dis,i));
        } 
        else 
        {
            WorldObject* teammate = worldModel->getWorldObject( i );
            if (teammate->validPosition) 
            {
                temp = teammate->pos;
				temp.setZ(0);
				dis = temp.getDistanceTo(ball);
	            distance.push_back(make_pair(dis,i));
            } 
            else 
            {
                distance.push_back(make_pair(20,i));
            }
        }        
    }
    sort(distance.begin(),distance.end(),cmp);
	firstTwoPlayer = make_pair(distance[0].second,distance[1].second);
	return firstTwoPlayer;
}

int NaoBehavior::findNearest2ballDefend()
{
    double minDis=100;
    int tempResult = 1;
    
    for(int i=2;i<12;++i)
    {
        if (worldModel->getUNum() == i) 
        {
            //当前机器人进程
            VecPosition temp = worldModel->getMyPosition();
			temp.setZ(0);
            if(abs(temp.getY() - ball.getY())<2)
            {
                double dis = temp.getDistanceTo(ball);
                if(dis < minDis)
                {
                    minDis=dis;
                    tempResult = i;
                }
            }
        }
        else
        {
            WorldObject* teammate = worldModel->getWorldObject( i );
            VecPosition matePos = teammate->pos;
            if(abs(matePos.getY() - ball.getY()) < 2)
            {
                double dis=ball.getDistanceTo(matePos);
                if(dis < minDis)
                {
                    minDis=dis;
                    tempResult = i;
                    //cout << "robot!!!!!!!" << i << endl;
                }
            }   
        }
    }
    //cout << "the nearest defender is " << tempResult << "!!!!!!" << endl;
    return tempResult;
}

int NaoBehavior::findOppoOnball()
{
    double minDis=100;
    int tempResult=1;
    for(int i=12;i<23;++i)
    {
        WorldObject* oppo = worldModel->getWorldObject( i );
        VecPosition oppoPos = oppo->pos;
        double dis=ball.getDistanceTo(oppoPos);
        if(dis<minDis)
        {
            minDis = dis;
            tempResult = i;
        }
    }
    return tempResult; 
}

SkillType NaoBehavior::oppoGoalStrategy()
{  
    VecPosition oppo = (worldModel->getWorldObject(findOppoOnball()))->pos;
    worldModel->getRVSender()->drawPoint("oppoOnBall",oppo.getX(),oppo.getY(),20.0f, RVSender::BLUE);
    double targetY=oppo.getY();
    if(oppo.getX()>13.2)
    {
        VecPosition target=VecPosition(12.9,targetY,0);
        return goToTarget(target);
    }
    else
    {
        return goToTarget(VecPosition(oppo.getX()-0.5,targetY,0));
    }   
}

SkillType NaoBehavior::supporter(){
    //if(onBallChange()==true){
    if(findPlayerDistance2Ball().first==1){
        return LongKick(VecPosition(15,0,0));
    }else{
        VecPosition offBall=VecPosition(ball.getX()-0.3,ball.getY(),0);
        VecPosition target = collisionAvoidance(true/*Avoid teammate*/,true/*Avoid opponent*/,false/*Avoid ball*/,1.0,.5,offBall, true /*fKeepDistance*/);
        return goToTarget(target);
    }        
    //}
}
/*int NaoBehavior::findPlayerSecond2Ball()
{
    // Find closest player to ball
    int playerClosestToBall = -1;
    int secondNum=-2;
    double closestDistanceToBall = 10000;
    double secondDistance=20000;
    pair<int,VecPosition> closest2Ball_info;
    VecPosition temp;
    VecPosition final_closest_player;
    
    //cout << "findclosest" << endl;
    for(int i = WO_TEAMMATE1; i < WO_TEAMMATE1+NUM_AGENTS; ++i) {
        int playerNum = i - WO_TEAMMATE1 + 1;
	//cout << worldModel->getUNum();
        if (worldModel->getUNum() == playerNum) {
            // This is us
            temp = worldModel->getMyPosition();
        } else {
            WorldObject* teammate = worldModel->getWorldObject( i );
            if (teammate->validPosition) {
                temp = teammate->pos;
            } else {
                continue;
            }
        }
        temp.setZ(0);

	
        double distanceToBall = temp.getDistanceTo(ball);
	if(i< WO_TEAMMATE1+2){
	  if(i==WO_TEAMMATE1){
	    closestDistanceToBall=distanceToBall;
	  }
	  if(i==WO_TEAMMATE1+1){
	    secondDistance=distanceToBall;
	  
	  if(secondDistance<closestDistanceToBall){
	    double middle=closestDistanceToBall;
	    closestDistanceToBall=secondDistance;
	    secondDistance=middle;
	  }
	  playerClosestToBall=1;
	  secondNum=2;
	  }
	}
	if(i>WO_TEAMMATE1+1){
        if (distanceToBall < closestDistanceToBall) {
	    secondDistance=closestDistanceToBall;
            playerClosestToBall = playerNum;
            closestDistanceToBall = distanceToBall;
	    final_closest_player = temp;
        }else if(distanceToBall<secondDistance){
	  secondDistance=distanceToBall;
	  secondNum=playerNum;
	}
	}
    }
    //closest2Ball_info = make_pair(playerClosestToBall,final_closest_player);
    VecPosition tmate=worldModel->getTeammate(secondNum);
    worldModel->getRVSender()->clear(); 
    worldModel->getRVSender()->drawPoint("secondplayer",tmate.getX(),tmate.getY(),20.0f, RVSender::ORANGE );
    return secondNum;
}*/

/*SkillType NaoBehavior::supporter(){
  if(onBallChange()==true){
    VecPosition fallentest= findPlayerClosest2Ball().second;
    
    worldModel->getRVSender()->drawPoint("onballfall",fallentest.getX(),fallentest.getY(),20.0f, RVSender::RED);
    VecPosition target = collisionAvoidance(true,false,false,.8,.3,ball, false);
    return goToTarget(target);
  }else{
     return goToTarget(VecPosition(ball.getX()-1,ball.getY(),0));
  }
}*/
 //vector <VecPosition> NaoBehavior ::supporterDelivery(int num,vector <VecPosition> currentBeam){
  //double littleLenth=10000;
  // int num=-1;
  //   for(int n=0;n<currentBeam.size();++n ){
  // vector<int> replac
  // for(int i=0;i<oppo.size();++i){
  //     double lenth=currentBeam[n].getDistanceTo(oppo[i]);
  //     if (lenth<littleLenth){
	// littleLenth=lenth;
	// num=n;
  //     }
  //   }
  //   replace.push_back(num);
  // }
  // //return replace;
  
  // for (int i=0;i<replace.size();++i){
  //   currentBeam[replace[i]]=oppo[i];
  //   cout<<"replace "<<replace[i]<<endl;
  // }

  //return currentBeam;
//}
