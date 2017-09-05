//
// Created by dudu on 9/4/17.
//

#ifndef CLIONPROJECT_PARTWORD_H
#define CLIONPROJECT_PARTWORD_H
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <stack>
using namespace std;
enum wordFeature{
    DECLARTION, OPERATER, IDENTIFIER, NUMBER, STRING, JUDGE, FUNCTION
};
typedef enum wordFeature wordFlag;
struct wd{
    string name;
    wordFlag type;
};

typedef struct wd worder;

vector<string> separateSentence(string sentence);
vector<string> separateSentence(string sentence);
vector<worder> mergeNegNumber(vector<worder> worders);
bool judgeNumber(string word);
bool judgeString(string word);
vector<worder> judgeWord(vector<string> wordVec);
vector<worder> complateSepSentence(string sentence);
vector<vector<worder>> processArgumentsOfFunction(worder word);
FILE* openFile(char* fileName);
vector<string> readFile(FILE* fd);

#endif //CLIONPROJECT_PARTWORD_H