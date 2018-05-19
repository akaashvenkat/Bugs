#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Field.h"
#include "Actor.h"
#include "Compiler.h"
#include <string>
#include <vector>
using namespace std;
class Actor;
class AntHill;

//StudentWorld class (inherits GameWorld)
class StudentWorld: public GameWorld
{
//public members
public:
    StudentWorld(string assetDir);
    ~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    void addAntCount(int colonyNum);
    void setActorVector(int x, int y, vector<Actor*> a);
    void addActor(int x, int y, Actor* a);
    void removeActor(int x, int y, Actor* a);
    vector<Actor*> getActorVector(int x, int y);
    bool hasPebble(int x, int y);
    bool hasFood(int x, int y);
    bool hasWater(int x, int y);
    bool hasPoison(int x, int y);
    void addFood(int x, int y, int amt);
    bool isInBounds(int x, int y);
//private members
private:
    int numTicks;
    int numAntsInCol[4];
    int numCol;
    vector<string> antCol;
    vector<Actor*> mActors[VIEW_WIDTH][VIEW_HEIGHT];
    vector<Compiler*> mAnts;
    vector<AntHill*> mAnthills;
    Field* mField;
    int loadField();
    bool hasActor(int x, int y, string name);
    void setDisplay();
    int findWinningAnt();
    int getAntCount(int colonyNum);
    Field* getField();
    bool loadAnts();
    vector<Compiler*>* getAntProg();
};

#endif // STUDENTWORLD_H_


