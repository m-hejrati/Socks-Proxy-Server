#include "LogVariables.h"


int passPacket = 0;
int passTraffic = 0;
std::vector <int> activeSessionsID;
int updateSession = 0;
int finishSession = 0;
int newSession = 0; 
int filterPacket = 0;
int filterTraffic = 0;

//std::vector <int> changeSessionsID;
std::unordered_set<int> changeSessionsID;
