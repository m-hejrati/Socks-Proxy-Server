#include <vector> 
#include <unordered_set>
#include <iostream>
#include <map>

#ifndef LOGVARS_H
#define LOGVARS_H


extern int passPacket;
extern int passTraffic;
extern std::vector <int> activeSessionsID;
extern int updateSession;
extern int finishSession;
extern int newSession;
extern int filterPacket;
extern int filterTraffic;
extern std::map <std::string, int> domainSession;
extern std::map <std::string, int> domainTraffic;

extern std::unordered_set<int> changeSessionsID;

#endif