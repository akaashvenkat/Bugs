#include "Actor.h"
#include "GraphObject.h"
#include "StudentWorld.h"
#include "Compiler.h"
#include <cmath>
#include <string>
using namespace std;

//Actor constructor
Actor::Actor(int imageID, int startX, int startY, int depth, StudentWorld* world)
:GraphObject(imageID, startX, startY, right, depth)
{
    mWorld = world;
    moveTo(startX, startY);
}

//returns world
StudentWorld* Actor::getWorld() const
{
    return mWorld;
}

//AnimateObject constructor
AnimateObject::AnimateObject(int imageID, int startX, int startY, int depth, int hp, StudentWorld* world)
:Actor(imageID, startX, startY, depth, world)
{
    mHealth = hp;
    mSleepCounter = 0;
    mAlive = true;
    mStunned = false;
    mPoisoned = false;
}

bool AnimateObject::isAnimated()
{
    return true;
}

bool AnimateObject::isAlive()
{
    return mAlive;
}

bool AnimateObject::isStunned() const
{
    return mStunned;
}

bool AnimateObject::isPoisoned() const
{
    return mPoisoned;
}

int AnimateObject::getCounter() const
{
    return mSleepCounter;
}

void AnimateObject::setStunned(bool stunned)
{
    mStunned = stunned;
    if (stunned)
        setCounter(getCounter() + 2);
}

void AnimateObject::setPoisoned(bool poisoned)
{
    mPoisoned = poisoned;
}

void AnimateObject::setCounter(int c)
{
    mSleepCounter = c;
}

int AnimateObject::getHealth() const
{
    return mHealth;
}

void AnimateObject::setHealth(int hp)
{
    mHealth = hp;
    if (getHealth() <= 0)
        mAlive = false;
}

//returns a random direction
GraphObject::Direction AnimateObject::getRandomDirection() const
{
    int randDirection = randInt(0, 3);
    Direction direction;
    switch (randDirection)
    {
        case 0:
            direction = Direction::up;
            break;
        case 1:
            direction = Direction::right;
            break;
        case 2:
            direction = Direction::down;
            break;
        case 3:
            direction = Direction::left;
            break;
        default:
            break;
    }
    return direction;
}

//InanimateObject constructor
InanimateObject::InanimateObject(int imageID, int startX, int startY, int depth, StudentWorld* world)
:Actor(imageID, startX, startY, depth, world)
{
    
}

bool InanimateObject::isAnimated()
{
    return false;
}

bool InanimateObject::isAlive()
{
    return false;
}

//Food constructor
Food::Food(int startX, int startY, int hp, StudentWorld* world)
:AnimateObject(IID_FOOD, startX, startY, 2, hp, world)
{
    
}

string Food::getName() const
{
    return "food";
}

void Food::increaseFood(int hp)
{
    setHealth(getHealth() + hp);
}

void Food::decreaseFood(int hp)
{
    setHealth(getHealth() - hp);
}

//Ant constructor
Ant::Ant(int colonyID, int colonyNum, int startX, int startY, StudentWorld* world, Compiler* c)
:AnimateObject(colonyID, startX, startY, 1, 1500, world)
{
    mColonyNum = colonyNum;
    mFoodHeld = 0;
    setDirection(getRandomDirection());
    mCompiler = c;
    mBitten = false;
    mBlocked = false;
    mInstructionCounter = 0;
    mLastRandomNum = 0;
}

void Ant::doSomething()
{
    //decrement health and return if health <= 0
    setHealth(getHealth() - 1);
    if (getHealth() <= 0)
    {
        //food added in setHealth if ant is dead
        return;
    }
    
    //return if still sleeping
    if (getCounter() != 0)
    {
        setCounter(getCounter() - 1);
        return;
    }
    else
        setStunned(false);
    
    //run through commands 10 times
    for (int i = 0; i < 10; i++)
    {
        struct Compiler::Command c1;
        Compiler::Command&c = c1;
        bool getCommandSuccess = mCompiler->getCommand(mInstructionCounter, c);
        if (!getCommandSuccess)
        {
            setHealth(0);
            return;
        }
        else
        {
            runCommand(c);
            // if anything other than if or goto
            if (!(c.opcode == Compiler::goto_command || c.opcode == Compiler::if_command))
            {
                mInstructionCounter++;
            }
            if (!(c.opcode == Compiler::goto_command || c.opcode == Compiler::if_command || c.opcode == Compiler::generateRandomNumber))
            {
                mBlocked = false;
                mBitten = false;
                setStunned(false);
                break;
            }
        }
    }
    //set sleep counter back to 2
    setCounter(2);
}

string Ant::getName() const
{
    return "ant";
}

void Ant::setHealth(int hp)
{
    if (isAlive())
    {
        AnimateObject::setHealth(hp);
        if (!isAlive())
            getWorld()->addFood(getX(), getY(), 100);
    }
}

int Ant::getColonyNumber() const
{
    return mColonyNum;
}

void Ant::doBite(int hp)
{
    if (!mBitten)
    {
        setHealth(getHealth() - hp); // food added if dead
        mBitten = true;
    }
}

void Ant::doStunned()
{
    if (!isStunned())
    {
        setStunned(true);
        setCounter(2);
    }
}

void Ant::doPoisoned()
{
    setPoisoned(true);
    setHealth(getHealth() - 150);
}

//runs specific functions for the command called by bug file
bool Ant::runCommand(const Compiler::Command& c)
{
    switch((int)c.opcode)
    {
        case Compiler::moveForward:
            moveForward();
            return true;
        case Compiler::eatFood:
            eatFood();
            return true;
        case Compiler::dropFood:
            dropFood();
            return true;
        case Compiler::bite:
            doRandomBite();
            return true;
        case Compiler::pickupFood:
            pickupFood();
            return true;
        case Compiler::emitPheromone:
            emitPheromone();
            return true;
        case Compiler::faceRandomDirection:
            faceRandomDirection();
            return true;
        case Compiler::rotateClockwise:
            rotateClockwise();
            return true;
        case Compiler::rotateCounterClockwise:
            rotateCounterClockwise();
            return true;
        case Compiler::generateRandomNumber:
            generateRandomNumber(c);
            return true;
        case Compiler::goto_command:
            goto_command(c);
            return true;
        case Compiler::if_command:
            if_command(c);
            return true;
        default:
            return false;
    }
}

void Ant::moveForward()
{    
    int newX = getX();
    int newY = getY();
    Direction dir = getDirection();
    
    //sets new position as one spot forward in specific direction
    switch (dir)
    {
        case GraphObject::up:
            newY--;
            break;
        case GraphObject::down:
            newY++;
            break;
        case GraphObject::right:
            newX++;
            break;
        case GraphObject::left:
            newX--;
            break;
        default:
            break;
    }
    
    //moves to next position if in bounds and no pebble present
    if (getWorld()->isInBounds(newX, newY) && !getWorld()->hasPebble(newX, newY))
    {
        getWorld()->removeActor(getX(), getY(), this);
        moveTo(newX, newY);
        getWorld()->addActor(newX, newY, this);
        mBlocked = false;
        mBitten = false;
    }
    else
        mBlocked = true;
}

void Ant::eatFood()
{
    //eat amount of food held
    if (mFoodHeld >= 100)
    {
        setHealth(getHealth() + 100);
        mFoodHeld -= 100;
    }
    else
    {
        setHealth(getHealth() + mFoodHeld);
        mFoodHeld = 0;
    }
}

void Ant::dropFood()
{
    getWorld()->addFood(getX(), getY(), mFoodHeld);
    mFoodHeld = 0;
}

void Ant::doRandomBite()
{
    vector<Actor*> canBiteActors;
    //get vector of actors that can be bitten
    for (int i = 0; i < getWorld()->getActorVector(getX(), getY()).size(); i++)
    {
        Actor* a = getWorld()->getActorVector(getX(), getY()).at(i);
        string name = a->getName();
        if (a == this || !isAlive())
            continue;
        if (name == "babyGrasshopper" || name == "adultGrasshopper" || name == "ant")
        {
            canBiteActors.push_back(a);
        }
    }
    if (canBiteActors.size() == 0)
        return;
    
    //bite random insect from vector
    int insectTobeBitten = randInt(0, (int)canBiteActors.size() - 1);
    AnimateObject* tempActor = dynamic_cast<AnimateObject*>(canBiteActors[insectTobeBitten]);
    if (tempActor != nullptr)
        tempActor->doBite(15);
}

void Ant::pickupFood()
{
    //pickup max possible food in one tick
    if (mFoodHeld == 1800) // calculate amount
        return;
    // calculate remaining amount it can hold
    int amountNeeded = 400;
    if (mFoodHeld + 400 > 1800)
        amountNeeded = 1800 - mFoodHeld;
    
    if(getWorld()->hasFood(getX(), getY()))
    {
        //search for food in current position
        Food* eatenFood = nullptr;
        int pos = 0;
        for (int i = 0; i < getWorld()->getActorVector(getX(), getY()).size(); i++)
        {
            if (!isAlive())
                continue;
            if (getWorld()->getActorVector(getX(), getY()).at(i)->getName() == "food")
            {
                eatenFood = dynamic_cast<Food*>(getWorld()->getActorVector(getX(), getY()).at(i));
                pos = i;
                break;
            }
        }
        
        //pickup any food present
        if (eatenFood != nullptr)
        {
            if (eatenFood->getHealth() > amountNeeded)
            {
                eatenFood->decreaseFood(amountNeeded);
                mFoodHeld += amountNeeded;
            }
            else
            {
                int numUnits = eatenFood->getHealth();
                eatenFood->decreaseFood(numUnits);
                mFoodHeld += numUnits;
            }
            getWorld()->getActorVector(getX(), getY()).at(pos) = eatenFood;
            getWorld()->setActorVector(getX(), getY(), getWorld()->getActorVector(getX(), getY()));
        }
    }
}

void Ant::emitPheromone()
{
    Pheromone *pher = nullptr;
    bool emitScent = false;
    
    //search for Pheromone in current position
    vector<Actor*> actorList = getWorld()->getActorVector(getX(), getY());
    for (int i = 0; i < actorList.size(); i++)
    {
        pher = dynamic_cast<Pheromone*>(actorList[i]);
        if (pher != nullptr)
        {
            //emit scent and edit strength of pheromone if pheromone is of same colony
            if (mColonyNum == pher->getColonyNumber())
            {
                int strength = 768 - pher->getHealth();
                if (strength < 256)
                {
                    pher->setHealth(getHealth() - strength);
                    emitScent = true;
                }
                else
                {
                    pher->setHealth(getHealth() - 256);
                    emitScent = true;
                }
            }
        }
    }
    
    //if scent not emited, add new pheromone object to field
    if (!emitScent)
    {
        switch(mColonyNum)
        {
            case 0:
                pher = new Pheromone(IID_PHEROMONE_TYPE0, mColonyNum, getX(), getY(), getWorld());
                break;
            case 1:
                pher = new Pheromone(IID_PHEROMONE_TYPE1, mColonyNum, getX(), getY(), getWorld());
                break;
            case 2:
                pher = new Pheromone(IID_PHEROMONE_TYPE2, mColonyNum, getX(), getY(), getWorld());
                break;
            case 3:
                pher = new Pheromone(IID_PHEROMONE_TYPE3, mColonyNum, getX(), getY(), getWorld());
                break;
            default:
                break;
        }
        getWorld()->addActor(getX(), getY(), pher);
    }
}

void Ant::faceRandomDirection()
{
    setDirection(getRandomDirection());
}

//set direction 90 degrees to the right
void Ant::rotateClockwise()
{
    if (getDirection() == Direction::right)
        setDirection(Direction::down);
    if (getDirection() == Direction::down)
        setDirection(Direction::left);
    if (getDirection() == Direction::left)
        setDirection(Direction::up);
    if (getDirection() == Direction::up)
        setDirection(Direction::right);
}

//set direction 90 degrees to the left
void Ant::rotateCounterClockwise()
{
    if (getDirection() == Direction::right)
        setDirection(Direction::up);
    if (getDirection() == Direction::down)
        setDirection(Direction::right);
    if (getDirection() == Direction::left)
        setDirection(Direction::down);
    if (getDirection() == Direction::up)
        setDirection(Direction::left);
}

void Ant::generateRandomNumber(const Compiler::Command& c)
{
    if (c.operand1 == "" || stoi(c.operand1) == 0)
        mLastRandomNum = 0;
    else
        mLastRandomNum = randInt(0, stoi(c.operand1)-1);
}

void Ant::goto_command(const Compiler::Command& c)
{
    if (c.operand1 == "")
        mInstructionCounter = 0;
    else
        mInstructionCounter = stoi(c.operand1);
}

//runs conditions if bug is told the if_command
void Ant::if_command(const Compiler::Command& c)
{
    int newX = 0;
    int newY = 0;
    int condition = stoi(c.operand1);
    switch (condition)
    {
        case Compiler::last_random_number_was_zero:
            if (mLastRandomNum == 0)
                mInstructionCounter = stoi(c.operand2);
            else
                mInstructionCounter++;
            break;
        case Compiler::i_am_carrying_food:
            if(mFoodHeld > 0)
                mInstructionCounter = stoi(c.operand2);
            else
                mInstructionCounter++;
            break;
        case Compiler::i_am_hungry:
            if(getHealth() <= 25)
                mInstructionCounter = stoi(c.operand2);
            else
                mInstructionCounter++;
            break;
        case Compiler::i_am_standing_with_an_enemy:
            if(hasActorAtPostion(getX(), getY(), "enemypresent" ))
                mInstructionCounter = stoi(c.operand2);
            else
                mInstructionCounter++;
            break;
        case Compiler::i_am_standing_on_food:
            if (hasActorAtPostion(getX(), getY(), "foodpresent"))
                mInstructionCounter = stoi(c.operand2);
            else
                mInstructionCounter++;
            break;
        case Compiler::i_am_standing_on_my_anthill:
            if (hasActorAtPostion(getX(), getY(), "myanthillpresent"))
                mInstructionCounter = stoi(c.operand2);
            else
                mInstructionCounter++;
            break;
        case Compiler::i_smell_pheromone_in_front_of_me:
            newX = getX();
            newY = getY();
            switch (getDirection())
            {
                case Direction::up:
                    newY -= 1;
                    break;
                case Direction::down:
                    newY += 1;
                    break;
                case Direction::right:
                    newX += 1;
                    break;
                case Direction::left:
                    newX -= 1;
                    break;
                default:
                    break;
            }
            if (getWorld()->isInBounds(newX, newY))
            {
                if (hasActorAtPostion(newX, newY, "pheromonepresent"))
                    mInstructionCounter = stoi(c.operand2);
                else
                    mInstructionCounter++;
            }
            else
                mInstructionCounter++;
            break;
        case Compiler::i_was_bit:
            if(mBitten)
                mInstructionCounter = stoi(c.operand2);
            else
                mInstructionCounter++;
            break;
        case Compiler::i_was_blocked_from_moving:
            if (mBlocked)
                mInstructionCounter = stoi(c.operand2);
            else
                mInstructionCounter++;
            break;
    }
}

bool Ant::hasActorAtPostion(int x, int y, string instruction)
{
    for (int i = 0; i < getWorld()->getActorVector(x, y).size(); i++)
    {
        Actor* currActor = getWorld()->getActorVector(x, y).at(i);
        if (currActor == this || !currActor->isAlive())
            continue;
        string name = currActor->getName();
        if (instruction == "enemypresent")
        {
            if (name == "babyGrasshopper" || name == "adultGrasshopper")
                return true;
            else if (name == "ant")
            {
                Ant* ant = dynamic_cast<Ant*>(currActor);
                if (ant->getColonyNumber() != mColonyNum)
                    return true;
            }
        }
        else if (instruction == "foodpresent" && name == "food")
            return true;
        else if (instruction == "myanthillpresent" && name == "anthill")
        {
            AntHill* anthill = dynamic_cast<AntHill*>(currActor);
            if (anthill->getColonyNumber() == mColonyNum)
                return true;
        }
        else if (instruction == "pheromonepresent" && name == "pheromone")
        {
            Pheromone* pher = dynamic_cast<Pheromone*>(currActor);
            if (pher->getColonyNumber() == mColonyNum)
                return true;
        }
    }
    return false;
}

//Pheromone constructor
Pheromone::Pheromone(int pheromoneID, int colonyNum, int startX, int startY, StudentWorld* world)
:AnimateObject(pheromoneID, startX, startY, 2, 256, world)
{
    mColonyNumber = colonyNum;
}

void Pheromone::doSomething()
{
    setHealth(getHealth()-1);
}

int Pheromone::getColonyNumber() const
{
    return mColonyNumber;
}

string Pheromone::getName() const
{
    return "pheromone";
}

//Anthill constructor
AntHill::AntHill(int startX, int startY, int colonyNum, Compiler* c, StudentWorld* world)
:AnimateObject(IID_ANT_HILL, startX, startY, 2, 8999, world)
{
    mColNumber = colonyNum;
    mCompiler = c;
}

void AntHill::doSomething()
{
    //decrement health and return if health <= 0
    setHealth(getHealth()-1);
    if (getHealth() <= 0)
    {
        return;
    }
    //get food present in current position, if possible
    if (getWorld()->hasFood(getX(), getY()))
    {
        Food* eatenFood = nullptr;
        int pos = 0;
        for (int i = 0; i < getWorld()->getActorVector(getX(), getY()).size(); i++)
        {
            if (getWorld()->getActorVector(getX(), getY()).at(i)->getName() == "food")
            {
                eatenFood = dynamic_cast<Food*>(getWorld()->getActorVector(getX(), getY()).at(i));
                pos = i;
                break;
            }
        }
        
        //increase health from eating the food
        if (eatenFood != nullptr)
        {
            if (eatenFood->getHealth() > 10000)
            {
                setHealth(getHealth() + 10000);
                eatenFood->decreaseFood(10000);
            }
            else
            {
                int numUnits = eatenFood->getHealth();
                setHealth(getHealth() + numUnits);
                eatenFood->decreaseFood(numUnits);
            }
            getWorld()->getActorVector(getX(), getY()).at(pos) = eatenFood;
            getWorld()->setActorVector(getX(), getY(), getWorld()->getActorVector(getX(), getY()));
        }
        return;
    }
    
    //spawn new ant if health >= 2000
    if (getHealth() >= 2000)
    {
        Ant* newAnt = new Ant(getAntID(), mColNumber, getX(), getY(), getWorld(), mCompiler);
        getWorld()->addActor(getX(), getY(), newAnt);
        setHealth(getHealth() - 1500);
        getWorld()->addAntCount(getColonyNumber());
    }
}

string AntHill::getName() const
{
    return "anthill";
}

int AntHill::getAntID() const
{
    switch (mColNumber)
    {
        case 0:
            return IID_ANT_TYPE0;
        case 1:
            return IID_ANT_TYPE1;
        case 2:
            return IID_ANT_TYPE2;
        case 3:
            return IID_ANT_TYPE3;
        default:
            return -1;
    }
}

int AntHill::getColonyNumber() const
{
    return mColNumber;
}

//BabyGrasshopper constructor
BabyGrasshopper::BabyGrasshopper(int startX, int startY, StudentWorld* world, int imageID, int health)
:AnimateObject(imageID, startX, startY, 0, health, world)
{
    setRandomDistAndDir();
}

void BabyGrasshopper::doSomething()
{
    if (canMove())
    {
        if (getHealth() >= 1600)
        {
            //turn into adult grasshopper if health >= 1600
            AdultGrasshopper* adultHopper = new AdultGrasshopper(getX(), getY(), getWorld());
            getWorld()->addActor(getX(), getY(), adultHopper);
            setHealth(0);
            return;
        }
        if (dealWithFood())
        {
            hopperMove();
            setStunned(false);
            setPoisoned(false);
        }
    }
}

void BabyGrasshopper::setRandomDistAndDir()
{
    setDirection(getRandomDirection());
    setDistance(randInt(2, 10));
}

void BabyGrasshopper::moveToNextPos(int startX, int startY, int endX, int endY)
{
    getWorld()->removeActor(startX, startY, this);
    moveTo(endX, endY);
    getWorld()->addActor(endX, endY, this);
}

bool BabyGrasshopper::canMove()
{
    setHealth(getHealth() - 1);
    if (getHealth() <= 0)
    {
        //food added in setHealth if grasshopper is dead
        return false;
    }
    if (getCounter() != 0)
    {
        setCounter(getCounter()-1);
        return false;
    }
    return true;
}

bool BabyGrasshopper::dealWithFood()
{
    if(getWorld()->hasFood(getX(), getY()))
    {
        //check if food is present at current position
        Food* eatenFood = nullptr;
        int pos = 0;
        for (int i = 0; i < getWorld()->getActorVector(getX(), getY()).size(); i++)
        {
            if (getWorld()->getActorVector(getX(), getY()).at(i)->getName() == "food")
            {
                eatenFood = dynamic_cast<Food*>(getWorld()->getActorVector(getX(), getY()).at(i));
                pos = i;
                break;
            }
        }
        
        //increase health from eating food
        if (eatenFood != nullptr)
        {
            if (eatenFood->getHealth() > 200)
            {
                setHealth(getHealth() + 200);
                eatenFood->decreaseFood(200);
            }
            else
            {
                int numUnits = eatenFood->getHealth();
                setHealth(getHealth() + numUnits);
                eatenFood->decreaseFood(numUnits);
            }
            getWorld()->getActorVector(getX(), getY()).at(pos) = eatenFood;
            getWorld()->setActorVector(getX(), getY(), getWorld()->getActorVector(getX(), getY()));
            
            int chanceEating = randInt(1, 2);
            if (chanceEating == 1)
            {
                setCounter(2);
                return false;
            }
        }
    }
    return true;
}

void BabyGrasshopper::hopperMove()
{
    //move forward one step in current direction
    if (getDistance() == 0)
        setRandomDistAndDir();
    int newX = getX();
    int newY = getY();
    switch (getDirection())
    {
        case Direction::up:
            newY -= 1;
            break;
        case Direction::down:
            newY += 1;
            break;
        case Direction::right:
            newX += 1;
            break;
        case Direction::left:
            newX -= 1;
            break;
        default:
            break;
    }
    if (!getWorld()->isInBounds(newX, newY)) // behavior same as pebble present
    {
        setDistance(0);
        setCounter(2);
        return;
    }
    
    if (getWorld()->hasPebble(newX, newY))
    {
        setDistance(0);
        setCounter(2);
        return;
    }
    else
        moveToNextPos(getX(), getY(), newX, newY);
    setDistance(getDistance() - 1);
    setCounter(2);
}

int BabyGrasshopper::getDistance() const
{
    return mDistance;
}

void BabyGrasshopper::setDistance(int d)
{
    mDistance = d;
}

string BabyGrasshopper::getName() const
{
    return "babyGrasshopper";
}

void BabyGrasshopper::setHealth(int hp)
{
    if (isAlive())
    {
        AnimateObject::setHealth(hp);
        if (!isAlive())
            getWorld()->addFood(getX(), getY(), 100);
    }
}

void BabyGrasshopper::doBite(int hp)
{
    setHealth(getHealth() - hp);
}

void BabyGrasshopper::doStunned()
{
    if (!isStunned())
    {
        setStunned(true);
        setCounter(getCounter() +2);
    }
}

void BabyGrasshopper::doPoisoned()
{
    setPoisoned(true);
    setHealth(getHealth() - 150);
}

//AdultGrasshopper constructor
AdultGrasshopper::AdultGrasshopper(int startX, int startY, StudentWorld* world, int imageID, int health)
:BabyGrasshopper(startX, startY, world, imageID, health)
{
    
}

void AdultGrasshopper::doSomething()
{
    int startX = getX();
    int startY = getY();
    if (canMove())
    {
        if (!canBite())
            if (!canJump())
                if(dealWithFood())
                    //move grasshopper if it can't bite or jump, and can deal with food
                    hopperMove();
        setCounter(2);
        if (startX != getX() && startY != getY())
            doRandomBite();
    }
}

string AdultGrasshopper::getName() const
{
    return "adultGrasshopper";
}

bool AdultGrasshopper::canBite()
{
    int biteChance = randInt(1, 3);
    if (biteChance == 1)
    {
        //set vector of insects that can be bitten
        vector<Actor*> canBiteActors;
        for (int i = 0; i < getWorld()->getActorVector(getX(), getY()).size(); i++)
        {
            Actor* a = getWorld()->getActorVector(getX(), getY()).at(i);
            string name = a->getName();
            if (a == this)
                continue;
            if (name == "babyGrasshopper" || name == "adultGrasshopper" || name == "ant")
            {
                canBiteActors.push_back(a);
            }
        }
        if (canBiteActors.size() == 0)
            return false;
        
        //bite a random insect from the vector
        int insectTobeBitten = randInt(0, (int)canBiteActors.size()-1);
        AnimateObject* tempActor = dynamic_cast<AnimateObject*>(canBiteActors[insectTobeBitten]);
        if (tempActor != nullptr)
            tempActor->doBite(50);
        return true;
    }
    return false;
}

bool AdultGrasshopper::canJump()
{
    int jumpChance = randInt(1, 10);
    if (jumpChance == 1)
    {
        //there is a 10% chance the adult grasshopper will jump to a position somewhere within 10 units radius
        int randDirection = randInt(0, 359);
        int endX = getX() + 10 * cos(randDirection);
        int endY = getY() + 10 * sin(randDirection);
        if (getWorld()->isInBounds(endX, endY))
        {
            for (int i = 0; i < getWorld()->getActorVector(endX, endY).size(); i++)
            {
                if (getWorld()->getActorVector(endX, endY).at(i)->getName() != "rock")
                {
                    //move to new position if pebble is not present
                    moveToNextPos(getX(), getY(), endX, endY);
                    setCounter(2);
                    return true;
                }
            }
        }
    }
    return false;
}

void AdultGrasshopper::doBite(int hp)
{
    setHealth(getHealth() - hp);
    if (isAlive())
    {
        //randomly bite an insect of adult grasshopper is alive
        int chanceRetaliate = randInt(1,2);
        if (chanceRetaliate == 1)
            doRandomBite();
    }
    //if not, add food at current position
    else
        getWorld()->addFood(getX(), getY(), 100);
}

void AdultGrasshopper::doRandomBite()
{
    //set vector of insects that can be bitten
    vector<Actor*> canBiteActors;
    for (int i = 0; i < getWorld()->getActorVector(getX(), getY()).size(); i++)
    {
        Actor* a = getWorld()->getActorVector(getX(), getY()).at(i);
        string name = a->getName();
        if (a == this)
            continue;
        if (name == "babyGrasshopper" || name == "adultGrasshopper" || name == "ant")
        {
            canBiteActors.push_back(a);
        }
    }
    if (canBiteActors.size() == 0)
        return;
    
    //randomly bite an insect from the vector
    int insectTobeBitten = randInt(0, (int)canBiteActors.size() - 1);
    AnimateObject* tempActor = dynamic_cast<AnimateObject*>(canBiteActors[insectTobeBitten]);
    if (tempActor != nullptr)
        tempActor->doBite(50);
}

Pebble::Pebble(int startX, int startY, StudentWorld* world)
:InanimateObject(IID_ROCK, startX, startY, 1, world)
{
    
}

string Pebble::getName() const
{
    return "rock";
}

WaterPool::WaterPool(int startX, int startY, StudentWorld* world)
:InanimateObject(IID_WATER_POOL, startX, startY, 2, world)
{
    
}

void WaterPool::doSomething()
{
    for (int i = 0; i < getWorld()->getActorVector(getX(), getY()).size(); i++)
    {
        //stun baby grasshoppers or ants if they are in the same position as the pool of water
        AnimateObject* a = dynamic_cast<AnimateObject*>(getWorld()->getActorVector(getX(), getY()).at(i));
        if (a == nullptr)
            continue;
        else
        {
            string name = a->getName();
            if (name == "babyGrasshopper" || name == "ant")
            {
                if (!a->isStunned())
                    a->doStunned();
            }
        }
    }
}

string WaterPool::getName() const
{
    return "waterPool";
}

Poison::Poison(int startX, int startY, StudentWorld* world)
:InanimateObject(IID_POISON, startX, startY, 2, world)
{
    
}

void Poison::doSomething()
{
    for (int i = 0; i < getWorld()->getActorVector(getX(), getY()).size(); i++)
    {
        //poison baby grasshoppers or ants if they are in the same position as the poison
        AnimateObject* a = dynamic_cast<AnimateObject*>(getWorld()->getActorVector(getX(), getY()).at(i));
        if (a == nullptr)
            continue;
        else
        {
            string name = a->getName();
            if (name == "babyGrasshopper" || name == "ant")
            {
                if (!a->isPoisoned())
                    a->doPoisoned();
            }
        }
    }
}

string Poison::getName() const
{
    return "poison";
}
