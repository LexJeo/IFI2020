#include "pkbehaviors.h"


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
/////// GOALIE
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

PKGoalieBehavior::
PKGoalieBehavior( const std::string teamName,
                  int uNum,
                  const map<string, string>& namedParams_,
                  const string& rsg_)
    : NaoBehavior( teamName,
                   uNum,
                   namedParams_,
                   rsg_) {
}

void PKGoalieBehavior::
beam( double& beamX, double& beamY, double& beamAngle ) {
    beamX = -HALF_FIELD_X+.5;
    beamY = 0;
    beamAngle = 0;
}




SkillType PKGoalieBehavior::selectSkill() {
    double distance,angle;
    getTargetDistanceAndAngle(ball,distance,angle);
    isPKGoal = true;
    return NaoBehavior::selectKeeperSkills(ball,distance,angle);
}





////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
/////// SHOOTER
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
PKShooterBehavior::
PKShooterBehavior( const std::string teamName,
                   int uNum,
                   const map<string, string>& namedParams_,
                   const string& rsg_ )
    : NaoBehavior( teamName,
                   uNum,
                   namedParams_,
                   rsg_ ) {
}

void PKShooterBehavior::
beam( double& beamX, double& beamY, double& beamAngle ) {
    beamX = -0.5;
    beamY = 0;
    beamAngle = 0;
}

SkillType PKShooterBehavior::
selectSkill() {
    int i = WO_TEAMMATE1;
    cout<<worldModel->getWorldObject(i)<<endl;
    VecPosition center=VecPosition(15,0.5, 0);

    double distance=ball.getDistanceTo(center);;
    if(ball.getX()<5)
    return kickBall(KICK_IK,center);
    else if(ball.getX()<10)
    return kickBall(KICK_DRIBBLE,center);
    else
    return kickBall(7,center);

}
