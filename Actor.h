#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"
#include "Compiler.h"
#include <cmath>
#include <string>
using namespace std;
class StudentWorld;

//class Actor (inherits GraphObject)
class Actor: public GraphObject
{
//public members
public:
    Actor(int imageID, int startX, int startY, int depth, StudentWorld* world);
    virtual ~Actor() {}
    virtual void doSomething() = 0;
    virtual bool isAnimated() = 0;
    virtual bool isAlive() = 0;
    virtual string getName() const = 0;
    StudentWorld* getWorld() const;
//private members
private:
    StudentWorld* mWorld;
};

//class AnimateObject (inherits Actor)
class AnimateObject: public Actor
{
//public members
public:
    AnimateObject(int imageID, int startX, int startY, int depth, int hp, StudentWorld* world);
    virtual ~AnimateObject() {}
    virtual bool isAnimated();
    virtual bool isAlive();
    bool isStunned() const;
    bool isPoisoned() const;
    int getCounter() const;
    void setStunned(bool stunned);
    void setPoisoned(bool poisoned);
    void setCounter(int c);
    virtual string getName() const = 0;
    int getHealth() const;
    virtual void setHealth(int hp);
    virtual void doBite(int hp) {}
    virtual void doStunned() {}
    virtual void doPoisoned() {}
    GraphObject::Direction getRandomDirection() const;
//private members
private:
    int mHealth;
    int mSleepCounter;
    bool mAlive;
    bool mStunned;
    bool mPoisoned;
};

//InanimateObject class (inherits Actor)
class InanimateObject: public Actor
{
//public members
public:
    InanimateObject(int imageID, int startX, int startY, int depth, StudentWorld* world);
    virtual ~InanimateObject() {}
    virtual bool isAnimated();
    virtual bool isAlive();
    virtual string getName() const = 0;
};

//Food class (inherits AnimateObject)
class Food: public AnimateObject
{
//public members
public:
    Food(int startX, int startY, int hp, StudentWorld* world);
    virtual ~Food() {}
    virtual void doSomething() {}
    virtual string getName() const;
    void increaseFood(int hp);
    void decreaseFood(int hp);
};

//Ant class (inherits AnimateObject)
class Ant: public AnimateObject
{
//public members
public:
    Ant(int colonyID, int colonyNum, int startX, int startY, StudentWorld* world, Compiler* c);
    virtual ~Ant() {}
    virtual void doSomething();
    virtual string getName() const;
    virtual void setHealth(int hp);
    int getColonyNumber() const;
    virtual void doBite(int hp);
    virtual void doStunned();
    virtual void doPoisoned();
//private members
private:
    int mColonyNum;
    int mFoodHeld;
    bool mBitten;
    bool mBlocked;
    int mInstructionCounter;
    int mLastRandomNum;
    Compiler* mCompiler;
    bool runCommand(const Compiler::Command& c);
    void moveForward();
    void eatFood();
    void dropFood();
    void doRandomBite();
    void pickupFood();
    void emitPheromone();
    void faceRandomDirection();
    void rotateClockwise();
    void rotateCounterClockwise();
    void generateRandomNumber(const Compiler::Command& c);
    void goto_command(const Compiler::Command& c);
    void if_command(const Compiler::Command& c);
    bool hasActorAtPostion(int x, int y, string instruction);
};

//Pheromone class (inherits AnimateObject)
class Pheromone: public AnimateObject
{
//public members
public:
    Pheromone(int pheromoneID, int colonyNum, int startX, int startY, StudentWorld* world);
    virtual ~Pheromone() {}
    virtual void doSomething();
    int getColonyNumber() const;
    virtual string getName() const;
//private members
private:
    int mColonyNumber;
};

//Anthill class (inherits AnimateObject)
class AntHill: public AnimateObject
{
//public members
public:
    AntHill(int startX, int startY, int colonyNum, Compiler* c, StudentWorld* world);
    virtual ~AntHill() {}
    virtual void doSomething();
    virtual string getName() const;
    int getColonyNumber() const;
//private members
private:
    int mColNumber;
    Compiler* mCompiler;
    int getAntID() const;
};

//BabyGrasshopper class (inherits AnimateObject)
class BabyGrasshopper: public AnimateObject
{
//public members
public:
    BabyGrasshopper(int startX, int startY, StudentWorld* world, int imageID = IID_BABY_GRASSHOPPER, int health = 500);
    virtual ~BabyGrasshopper() {}
    virtual void doSomething();
    virtual string getName() const;
    void setRandomDistAndDir();
    void moveToNextPos(int startX, int startY, int endX, int endY);
    bool canMove();
    bool dealWithFood();
    void hopperMove();
    int getDistance() const;
    void setDistance(int d);
    virtual void setHealth(int hp);
    virtual void doBite(int hp);
    virtual void doStunned();
    virtual void doPoisoned();
//private members
private:
    int mDistance;
};

//AdultGrasshopper class (inherits BabyGrasshopper)
class AdultGrasshopper: public BabyGrasshopper
{
//public members
public:
    AdultGrasshopper(int startX, int startY, StudentWorld* world, int imageID = IID_ADULT_GRASSHOPPER, int health = 1600);
    virtual ~AdultGrasshopper() {}
    virtual void doSomething();
    virtual string getName() const;
    virtual void doBite(int hp);
    virtual void doStunned() {}
    virtual void doPoisoned() {}
//private members
private:
    bool canBite();
    bool canJump();
    void doRandomBite();
};

//Pebble class (inherits InanimateObject)
class Pebble: public InanimateObject
{
//public members
public:
    Pebble(int startX, int startY, StudentWorld* world);
    virtual ~Pebble() {}
    virtual void doSomething() {}
    virtual string getName() const;
};

//WaterPool class (inherits InanimateObject)
class WaterPool: public InanimateObject
{
//public members
public:
    WaterPool(int startX, int startY, StudentWorld* world);
    virtual ~WaterPool() {}
    virtual void doSomething();
    virtual string getName() const;
};

//Poison class (inherits InanimateObject)
class Poison: public InanimateObject
{
//public members
public:
    Poison(int startX, int startY, StudentWorld* world);
    virtual ~Poison() {}
    virtual void doSomething();
    virtual string getName() const;
};

#endif // ACTOR_H_


