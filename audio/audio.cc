/*
Optional communication protocol for 2015 RoboCup 3D simulation
drop-in player challenge
*/

#include <iostream>
#include "audio.h"

#include "../headers/Field.h"


using namespace std;

//#define HALF_FIELD_X 15.0
//#define HALF_FIELD_Y 10.0
//#define NUM_AGENTS 11
/*
  These are x and y limits for the ball and the players.
*/
/*设置球场边界坐标*/
double minBallX = -HALF_FIELD_X - 2.0;
double maxBallX = HALF_FIELD_X + 2.0;
double minBallY = -HALF_FIELD_Y - 2.0;
double maxBallY = HALF_FIELD_Y + 2.0;
/*设置球员最小边界*/
double minAgentX = -HALF_FIELD_X - 5.0;
double maxAgentX = HALF_FIELD_X + 5.0;
double minAgentY = -HALF_FIELD_Y - 5.0;
double maxAgentY = HALF_FIELD_Y + 5.0;

// ------ Inteface Methods (call these) ------

/*
 * Creates say message if it currently time for the agent to speak.
 * uNum - Uniform number of agent (1-11)
 * currentServerTime - the current server time, NOT the game time
 * ballLastSeenServerTime - last server time ball was seen, NOT last game time
 * ballX - global X position of ball
 * ballY - global Y position of ball
 * myX - my global X position
 * myY - my global Y position
 * fFallen - whether or not I'm currently fallen
 * message - string to return say message in
 * RETURN - true if message is created, false otherwise
 */
 /******************************************************************************************
  *功能：
  * 生成目前轮到的该发言的机器人要说的字符串
  *输入参数：
  * uNum - 机器人的编号（1——11）
  * currentServerTime - 目前的服务器时间
  * ballLastSeenServerTime - 足球最新一次机器人看到的时间
  * ballX - 球的全局坐标x
  * ballY - 球的全局坐标y
  * myX - 我的位置的全局坐标x
  * myY - 我的位置的全局坐标y
  * fFallen - 表示此机器人是否摔倒的flag值
  * message - 储存机器人要发言的字符串内容
  *closestNum -距离求最近的球员编号
  *返回值：
  * RETURN -  若成功返回true，失败返回false
  ***********************************************************************************************/
/*****************************************************************************************************
 *决定是否发言：从cycles=0开始，由我方1号机器人开始发言，然后当cycles%(NUM_AGENTS*2)即cycles%22==2时，由我方2号发言，
 等于4时，由3号发言……直至11号，为一个循环
 *发言内容的生成：先将需要标号的代码转为编码，再转为字符串，目的是将不同类型的需要发言表达的数据均统一成string类型
 *******************************************************************************************************/
bool makeSayMessage(const int &uNum, const double &currentServerTime, const double &ballLastSeenServerTime, const double &ballX, const double &ballY, const double &myX, const double &myY, const bool &fFallen, string &message,const int &closestNum) {
    int cycles = int((currentServerTime * 50) + 0.1);
    if (cycles % (NUM_AGENTS*2) != (uNum-1)*2) {
        // Not our time slice turn to say a message
        return false;
    }

    vector<int> bits;
    if(!(dataToBits(currentServerTime, ballLastSeenServerTime, ballX, ballY, myX, myY, fFallen, bits))) {
        return false;
    }

    if(!(bitsToString(bits, message))) {
        return false;
    }
    string str;
    stringstream ss;
    ss<< closestNum;
    str=ss.str();
    message=message+str;
    message = "(say " + message + ")";

    return true;
}

/**
 * Parses message body of hear perceptor (need to first strip off hear header, time, and self/direction).
 * message - message to process
 * heardServerTime, the server time the message was heard, NOT the game time
 * uNum - uniform of the agent who said the message
 * ballLastSeenServerTime - server time agent who said the message saw the ball, NOT the game time
 * ballX - reported global X position of the ball
 * ballY - reported global Y position of the ball
 * agentX - reported global X position of the agent who said the message
 * agentY - reported glboal Y position of the agent who said the message
 * fFallen - whether or not agent who said the message is currently fallen
 * time - time message was said
 * RETURN - true if valid message parsed, false otherwise
 */
/******************************************************************************************
 *功能：
 * 将字符串转为信息
 *输入参数：
 * uNum - 机器人的编号（1——11）
 * heardServerTime - 收到的该条消息的发出时间，确定消息的实时性
 * ballLastSeenServerTime - 足球最新一次机器人看到的时间
 * ballX - 球的全局坐标x
 * ballY - 球的全局坐标y
 * myX - 我的位置的全局坐标x
 * myY - 我的位置的全局坐标y
 * fFallen - 表示此机器人是否摔倒的flag值
 * message - 储存机器人要发言的字符串内容
 *closestNumber -距离求最近的球员编号
 *返回值：
 * RETURN -  若成功返回true，失败返回false
 ***********************************************************************************************/
 /*****************************************************************************************************

  *******************************************************************************************************/
bool processHearMessage(const string &message, const double &heardServerTime, int &uNum, double &ballLastSeenServerTime, double &ballX, double &ballY, double &agentX, double &agentY, bool &fFallen, double &time, int& closestNumber) {

    // Initialize values
    uNum = 0;
    ballLastSeenServerTime = 0;
    ballX = 0;
    ballY = 0;
    agentX = 0;
    agentY = 0;
    fFallen = false;
    time = 0;
    closestNumber=0;
    string str1;
    stringstream ss1;
    //int closestNumber;
    //str1.assign(message,0,message.size());
    str1=message;
    int tmp=0;
    //ss1.str(str1);
    //ss1>>closestNumber;
    //closestNumber=(int)(str1[13])*10+(int)(str1[14]);
    for (int i=13;i<str1.size();++i){
      ss1<<str1[i];
      ss1>>closestNumber;
      closestNumber=closestNumber+tmp*10;
      tmp=closestNumber;
      ss1.str("");
    }


    vector<int> bits;
    if(!(stringToBits(message, bits))) {
        return false;
    }

    if(!(bitsToData(bits, time, ballLastSeenServerTime, ballX, ballY, agentX, agentY, fFallen))) {
        return false;
    }

    time += double(int((heardServerTime-time)/1310.72))*1310.72;
    ballLastSeenServerTime +=  double(int((heardServerTime-ballLastSeenServerTime)/1310.72))*1310.72;

    if (heardServerTime-time >= .07 || heardServerTime-time < -.001) {
        return false;
    }

    int cycles = int((time * 50) + 0.1);
    uNum = (cycles%(NUM_AGENTS*2))/2+1;

    return true;
}

//------ Implentation Methods (called by inteface methods) ------
/******************************************************************************************
 *功能：
 *将整型转为编码

 *输入参数：
  n-待编码的整型变量
  numBits-该变量可用的最大编码位数
 *返回值：
 * RETURN -  该变量的编码，共numBits位
 ***********************************************************************************************/

vector<int> intToBits(const int &n, const int &numBits) {

    vector<int> bits;

    if(n < 0) {
        bits.clear();
        return bits;
    }

    int m = n; //Copy.

    bits.resize(numBits);
    for(int i = numBits - 1; i >= 0; i--) {
        bits[i] = m % 2;
        m /= 2;
    }

    return bits;
}

vector<int> intToBits(const unsigned long long &n, const int &numBits) {
    vector<int> bits;

    unsigned long long m = n; //Copy.

    bits.resize(numBits);
    for(int i = numBits - 1; i >= 0; i--) {
        bits[i] = m % 2;
        m /= 2;
    }

    return bits;
}

int bitsToInt(const vector<int> &bits, const int &start, const int &end) {

    if(start < 0 || end >= (int)bits.size()) {
        return 0;//Error.
    }

    int n = 0;
    for(int i = start; i <= end; i++) {
        n *= 2;
        n += bits[i];
    }

    return n;
}
/******************************************************************************************
 *功能：
 * 生成发言编码bits

 *输入参数：
 * time - 目前的服务器时间
 * ballLastSeenServerTime - 足球最新一次机器人看到的时间
 * ballX - 球的全局坐标x
 * ballY - 球的全局坐标y
 * myX - 我的位置的全局坐标x
 * myY - 我的位置的全局坐标y
 * fFallen - 表示此机器人是否摔倒的flag值
 * bits-储存生成的发言内容的编码，用于返回

 *返回值：
 * RETURN -  储存发言内容的字符串容器bits
 ***********************************************************************************************/
 /*****************************************************************************************************
  *编码已有内容：
   服务器时间						16位
   看到球的最新时间					16位
   球的x,y坐标						x，y分别10位，共20位
   自己的x，y坐标					x，y分别10位，共20位
   摔倒flag，1倒，0不倒				1位
   距离球最近的球员编号				4位
  *******************************************************************************************************/
bool dataToBits(const double &time, const double &ballLastSeenTime, const double &ballX, const double &ballY, const double &myX, const double &myY, const bool &fFallen, vector<int> &bits) {

    int cycles = (time * 50) + 0.1;
    cycles = cycles%(1<<16);
    vector<int> timeBits = intToBits(cycles, 16);

    int ballLastSeenCycle = (ballLastSeenTime * 50) + 0.1;
    ballLastSeenCycle = ballLastSeenCycle%(1<<16);
    vector<int> ballLastSeenTimeBits = intToBits(ballLastSeenCycle, 16);

    double clippedBallX = (ballX < minBallX)? minBallX : ((ballX > maxBallX)? maxBallX : ballX);
    int bx = (((clippedBallX - minBallX) * 1023) / (maxBallX - minBallX)) + 0.5;
    vector<int> ballXBits = intToBits(bx, 10);

    double clippedBallY = (ballY < minBallY)? minBallY : ((ballY > maxBallY)? maxBallY : ballY);
    int by = (((clippedBallY - minBallY) * 1023) / (maxBallY - minBallY)) + 0.5;
    vector<int> ballYBits = intToBits(by, 10);

    double clippedMyX = (myX < minAgentX)? minAgentX : ((myX > maxAgentX)? maxAgentX : myX);
    int mx = (((clippedMyX - minAgentX) * 1023) / (maxAgentX - minAgentX)) + 0.5;
    vector<int> myXBits = intToBits(mx, 10);

    double clippedMyY = (myY < minAgentY)? minAgentY : ((myY > maxAgentY)? maxAgentY : myY);
    int my = (((clippedMyY - minAgentY) * 1023) / (maxAgentY - minAgentY)) + 0.5;
    vector<int> myYBits = intToBits(my, 10);

    int fallenBit = (fFallen)? 1 : 0;

    bits.insert(bits.end(), timeBits.begin(), timeBits.end());//16
    bits.insert(bits.end(), ballLastSeenTimeBits.begin(), ballLastSeenTimeBits.end());//16
    bits.insert(bits.end(), ballXBits.begin(), ballXBits.end());//10
    bits.insert(bits.end(), ballYBits.begin(), ballYBits.end());//10
    bits.insert(bits.end(), myXBits.begin(), myXBits.end());//10
    bits.insert(bits.end(), myYBits.begin(), myYBits.end());//10
    bits.push_back(fallenBit);//1

    return true;

}
/******************************************************************************************
 *功能：
 * 将二进制整型编码转为string类型消息message
 *输入参数：
   bits-待处理的编码
   message-待更新的发言内容
 *返回值：
 * RETURN -  返回message即机器人要发言的内容
 ***********************************************************************************************/
 /*****************************************************************************************************
 *转为字符串的机制：
      将编码每六位转为一个整数，然后在字符串中存入一个字符表中与之相对映的字符，最后得到的是一个可以根据密码表commAlphabet翻译出具体意思的字符串
  *******************************************************************************************************/
bool bitsToString(const vector<int> &bits, string &message) {

    message = "";
    if(commAlphabet.size() != 64) {
        cerr << "bitsToString: alphabet size not 64!\n";
        return false;
    }

    vector<int> index;
    index.resize((bits.size() + 5) / 6);
    size_t ctr = 0;
    for(size_t i = 0; i < index.size(); i++) {

        index[i] = 0;
        for(int j = 0; j < 6; j++) {

            index[i] *= 2;

            if(ctr < bits.size()) {
                index[i] += bits[ctr];
                ctr++;
            }

        }
    }

    for(size_t i = 0; i < index.size(); i++) {
        message += commAlphabet.at(index[i]);
    }

    return true;
}

/******************************************************************************************
 *功能：
 *将整型转为编码

 *输入参数：
  n-待编码的整型变量
  numBits-该变量可用的最大编码位数
 *返回值：
 * RETURN -  该变量的编码，共numBits位
 ***********************************************************************************************/

 /******************************************************************************************
  *功能：
  * 将编码bits转化为各类型数据的信息

  *输入参数：
  * time - 目前的服务器时间
  * ballLastSeenServerTime - 足球最新一次机器人看到的时间
  * ballX - 球的全局坐标x
  * ballY - 球的全局坐标y
  * myX - 我的位置的全局坐标x
  * myY - 我的位置的全局坐标y
  * fFallen - 表示此机器人是否摔倒的flag值
  * bits-储存生成的发言内容的编码，用于返回

  *返回值：
  * RETURN -  成功返回true，失败返回false
  ***********************************************************************************************/
bool bitsToData(const vector<int> &bits, double &time, double &ballLastSeenTime, double &ballX, double &ballY, double &agentX, double &agentY, bool &fFallen) {
    if(bits.size() < (16 + 16 + 10 + 10 + 10 + 10 + 1)) {//若传输二进制编码位数不对，则舍弃，并将引用变量赋值
        time = 0;
        ballLastSeenTime = 0,
        ballX = 0;
        ballY = 0;
        agentX = 0;
        agentY = 0;
        fFallen = false;
        return false;
    }
       //进行编码的逆运算分别得到各个数据
    int ctr = 0;                                           //ctr用于记录目前已经解码的位数和该解码的下一位的位置


    int cycles = bitsToInt(bits, ctr, ctr + 15);
    time = cycles * 0.02;
    ctr += 16;

    int ballLastSeenCycles = bitsToInt(bits, ctr, ctr + 15);
    ballLastSeenTime = ballLastSeenCycles * 0.02;
    ctr += 16;

    int bx = bitsToInt(bits, ctr, ctr + 9);
    ballX = minBallX + ((maxBallX - minBallX) * (bx / 1023.0));
    ctr += 10;

    int by = bitsToInt(bits, ctr, ctr + 9);
    ballY = minBallY + ((maxBallY - minBallY) * (by / 1023.0));
    ctr += 10;

    int ax = bitsToInt(bits, ctr, ctr + 9);
    agentX = minAgentX + ((maxAgentX - minAgentX) * (ax / 1023.0));
    ctr += 10;

    int ay = bitsToInt(bits, ctr, ctr + 9);
    agentY = minAgentY + ((maxAgentY - minAgentY) * (ay / 1023.0));
    ctr += 10;

    fFallen = (bits[ctr] == 0)? false : true;
    ctr += 1;

    return true;
}

/******************************************************************************************
  *功能：
  * 将字符串转为二进制编码
  *输入参数：
  *message - 待翻译的字符串
  *bits    - 得到的二进制数据
  *返回值：
  * RETURN -  bits
  ***********************************************************************************************/
bool stringToBits(const string &message, vector<int> &bits) {

    if(commAlphabet.size() != 64) {
        cerr << "bits2String: alphabet size not 64!\n";
        return false;
    }

    bits.resize(message.length() * 6);
    //将每一位字符翻译为十进制数值，存储在bits中
    for(size_t i = 0; i < message.length(); i++) {

        // Make sure every letter in the message comes from our alphabet. Make a mapping.
        // If any violation, return false;
        const char c = message.at(i);
        size_t n = commAlphabet.find(c);
        if(n == string::npos) {
            bits.clear();
            return false;
        }
            //将bits中的十进制数值转化为二进制
        for(int j = 5; j >= 0; j--) {
            bits[(i * 6) + j] = n % 2;
            n /= 2;
        }
    }

    return true;
}
