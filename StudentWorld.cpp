#include "StudentWorld.h"
#include "GameWorld.h"
#include "GameConstants.h"
#include "Field.h"
#include "Actor.h"
#include "Compiler.h"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(std::string assetDir)
{
	return new StudentWorld(assetDir);
}

//constructor
StudentWorld::StudentWorld(std::string assetDir)
: GameWorld(assetDir)
{
    numTicks = 0;
    mField = new Field();
    for (int i = 0; i < 4; i++)
        numAntsInCol[i] = 0;
}

//destructor
StudentWorld::~StudentWorld()
{
    cleanUp();
    delete mField;
}

//loads ant files
bool StudentWorld::loadAnts()
{
    vector<string> antNames = getFilenamesOfAntPrograms();
    string error = "";
    for (int i = 0; i < antNames.size(); i++)
    {
        //push back colony info
        Compiler* c = new Compiler();
        if(c->compile(antNames.at(i), error))
        {
            mAnts.push_back(c);
            antCol.push_back(c->getColonyName());
        }
    }
    if (error == "")
        return true;
    return false;
}

//loads field.txt
int StudentWorld::loadField()
{
    string fieldFile = getFieldFilename();
    
    if (mField->loadField(fieldFile) != Field::LoadResult::load_success)
        return false;
    
    for (int i = 0; i < VIEW_WIDTH; i++)
    {
        for (int j = 0; j < VIEW_HEIGHT; j++)
        {
            //push actors into vectors at specific positions
            Field::FieldItem object = mField->getContentsOf(i, j);
            switch(object)
            {
                case(Field::FieldItem::grasshopper):
                    mActors[i][j].push_back(new BabyGrasshopper(i , j, this));
                    break;
                case(Field::FieldItem::rock):
                    mActors[i][j].push_back(new Pebble(i, j, this));
                    break;
                case(Field::FieldItem::food):
                    mActors[i][j].push_back(new Food(i , j, 6000, this));
                    break;
                case(Field::FieldItem::water):
                    mActors[i][j].push_back(new WaterPool(i, j, this));
                    break;
                case(Field::FieldItem::poison):
                    mActors[i][j].push_back(new Poison(i, j, this));
                    break;
                
                    //additionally push anthills to vector containing anthills
                case(Field::FieldItem::anthill0):
                    if (mAnts.size() >= 1)
                    {
                        AntHill* newAnt0 = new AntHill(i, j, 0, mAnts.at(0), this);
                        mActors[i][j].push_back(newAnt0);
                        mAnthills.push_back(newAnt0);
                    }
                    break;
                case(Field::FieldItem::anthill1):
                    if (mAnts.size() >= 2)
                    {
                        AntHill* newAnt1 = new AntHill(i, j, 1, mAnts.at(1), this);
                        mActors[i][j].push_back(newAnt1);
                        mAnthills.push_back(newAnt1);
                    }
                    break;
                case(Field::FieldItem::anthill2):
                    if (mAnts.size() >= 3)
                    {
                        AntHill* newAnt2 = new AntHill(i, j, 2, mAnts.at(2), this);
                        mActors[i][j].push_back(newAnt2);
                        mAnthills.push_back(newAnt2);
                    }
                    break;
                case(Field::FieldItem::anthill3):
                    if (mAnts.size() >= 4)
                    {
                        AntHill* newAnt3 = new AntHill(i, j, 3, mAnts.at(3), this);
                        mActors[i][j].push_back(newAnt3);
                        mAnthills.push_back(newAnt3);
                    }
                    break;
                default:
                    break;
            }
        }
    }
    return true;
}

//initializes game
int StudentWorld::init()
{
    if (loadAnts() && loadField())
    {
        numTicks = 0;
        return GWSTATUS_CONTINUE_GAME;
    }
    else
        return GWSTATUS_LEVEL_ERROR;
}

//runs set of operations that needs to be done every tick
int StudentWorld::move()
{
    numTicks++;
    //call doSomething function of all actors
    for (int i = 0; i < VIEW_WIDTH; i++)
    {
        for (int j = 0; j < VIEW_HEIGHT; j++)
        {
            for (int k = 0; k < mActors[i][j].size(); k++)
            {
                mActors[i][j].at(k)->doSomething();
            }
        }
    }
    
    //delete any actors that are set dead
    for (int i = 0; i < VIEW_WIDTH; i++)
    {
        for (int j = 0; j < VIEW_HEIGHT; j++)
        {
            for (int k = 0; k < mActors[i][j].size(); k++)
            {
                if (mActors[i][j].at(k)->isAnimated())
                {
                    if (!mActors[i][j].at(k)->isAlive())
                    {
                        Actor* tempActor = mActors[i][j].at(k);
                        mActors[i][j].erase(mActors[i][j].begin()+k);
                        delete tempActor;
                    }
                }
            }
        }
    }
    
    setDisplay();
    
    //set final display after game over
    if (numTicks >= 2000)
    {
        if (findWinningAnt() == -1)
            return GWSTATUS_NO_WINNER;
        else
        {
            setWinner(antCol.at(findWinningAnt()));
            return GWSTATUS_PLAYER_WON;
        }
    }
    return GWSTATUS_CONTINUE_GAME;
}

//cleans up the screen
void StudentWorld::cleanUp()
{
    //deletes all the actor in the vectors
    for (int i = 0; i < VIEW_WIDTH; i++)
    {
        for (int j = 0; j < VIEW_HEIGHT; j++)
        {
            while (!mActors[i][j].empty())
            {
                Actor* temp = mActors[i][j].back();
                mActors[i][j].pop_back();
                delete temp;
            }
        }
    }
    
    //pop back all the ants
    for (int i = (int)mAnts.size(); i > 0; i--)
    {
        Compiler* c = mAnts.at(i-1);
        mAnts.pop_back();
        delete c;
    }
}

//set display of game
void StudentWorld::setDisplay()
{
    findWinningAnt();
    
    ostringstream convert;
    string tick = std::to_string(2000 - numTicks);
    convert << "Ticks:" << std::setw(5) << tick;
    for (int i = 0; i < antCol.size(); i++)
    {
        int numAnts = numAntsInCol[i];
        string resultNum = std::to_string(numAnts);
        if (i == findWinningAnt())
            convert << "  " << antCol.at(i) << "*: " << setfill('0') << std::setw(2) << resultNum << " ants";
        else
            convert << "  " << antCol.at(i) << ": " << setfill('0') << std::setw(2) << resultNum << " ants";
    }
    setGameStatText(convert.str());
}

//find ant colony with the most ants
int StudentWorld::findWinningAnt()
{
    int index = -1, max = 0;
    for (int i = 0; i < 4; i++)
    {
        if (max < numAntsInCol[i])
        {
            index = i;
            max = numAntsInCol[i];
        }
    }
    for (int i = 0; i < 4; i++)
    {
        if (i == index)
            continue;
        else
        {
            if (max == numAntsInCol[i])
                return -1;
        }
    }
    return index;
}

//increments ant count for a specific colony
void StudentWorld::addAntCount(int colonyNum)
{
    numAntsInCol[colonyNum]++;
}

//returns ant count for a specific colony
int StudentWorld::getAntCount(int colonyNum)
{
    return numAntsInCol[colonyNum];
}

//returns field
Field* StudentWorld::getField()
{
    return mField;
}

//sets vector at specific position
void StudentWorld::setActorVector(int x, int y, vector<Actor*> a)
{
    mActors[x][y] = a;
}

//adds actor to vector at specific position
void StudentWorld::addActor(int x, int y, Actor* a)
{
    mActors[x][y].push_back(a);
}

//removes specific actor from specified position
void StudentWorld::removeActor(int x, int y, Actor* a)
{
    for (int i = 0; i < mActors[x][y].size(); i++)
    {
        if (mActors[x][y].at(i) == a)
        {
            mActors[x][y].erase(mActors[x][y].begin() + i);
            break;
        }
    }
}

//returns vector at specific position
vector<Actor*> StudentWorld::getActorVector(int x, int y)
{
    return mActors[x][y];
}

//returns ant program
vector<Compiler*>* StudentWorld::getAntProg()
{
    return &mAnts;
}

//checks if pebble is at specific position
bool StudentWorld::hasPebble(int x, int y)
{
    return hasActor(x, y, "rock");
}

//checks if food is at specific position
bool StudentWorld::hasFood(int x, int y)
{
    return hasActor(x, y, "food");
}

//check if water is at specific position
bool StudentWorld::hasWater(int x, int y)
{
    return hasActor(x, y, "water");
}

//check if position is at specific position
bool StudentWorld::hasPoison(int x, int y)
{
    return hasActor(x, y, "poison");
}

//check if specific actor is at specific position
bool StudentWorld::hasActor(int x, int y, string name)
{
    for (int i = 0; i < mActors[x][y].size(); i++)
        if (mActors[x][y].at(i) != nullptr)
            if (mActors[x][y].at(i)->getName() == name)
                return true;
    return false;
}

//add food with specific amount to specific position
void StudentWorld::addFood(int x, int y, int amt)
{
    Food* food = nullptr;
    for (int i = 0; i < getActorVector(x, y).size(); i++)
    {
        if (getActorVector(x, y).at(i)->getName() == "food")
        {
            food = (Food*)getActorVector(x, y).at(i);
            break;
        }
    }
    if (food != nullptr)
        food->increaseFood(amt);
    else
    {
        Food* food = new Food(x, y, amt, this);
        addActor(x, y, food);
    }
}

//check if specific position is in bounds
bool StudentWorld::isInBounds(int x, int y)
{
    if (x > 0 && x < VIEW_WIDTH)
        if (y > 0 && y < VIEW_HEIGHT)
            return true;
    return false;
}
