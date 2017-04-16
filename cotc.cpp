#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;


template<typename T,int N>
class fast_vect
{
public:

  T arr[N];
  int size = 0;

  inline void reset(){size = 0;}
  inline void push_back(const T& e)
  {
   
    assert(size < N);
    arr[size++] = e;

  }


  void print()
  {
    for(int i =0;i<size;++i)
      {
	cerr << arr[i] << endl;
      }
  }
};

struct pos
{
  pos(int x_,int y_):x(x_),y(y_){}
  int x,y;
};


struct entity
{
  entity():id(0),p{0,0}{}
  entity(short id_,pos p_):id(id_),p(p_){}
  short id;
  pos p;
};

struct ship : public entity
{
  ship():entity(),ori(0),speed(0),rhum(0){}
  ship(short id,pos p,short ori_,short speed_,short rhum_):entity(id,p),ori(ori_),speed(speed_),rhum(rhum_){}
  short ori,speed,rhum;

};

struct barrel : public entity
{
  barrel():entity(),level(0){}
  barrel(short id,pos p,short level_):entity(id,p),level(level_){}
  short level;
};

struct ball : public entity
{
  ball():entity(),origin(0),turn(0){}
  ball(short id,pos p,ship * ori_,short turn_):entity(id,p),origin(ori_),turn(turn_){}
  ship * origin;
  short turn;
};

struct mine : public entity
{
  mine():entity(){}
  mine(short id,pos p):entity(id,p){}
};


ostream& operator<<(ostream& out, const pos& p)
{
    out << "(" << p.x << ", " << p.y << ")"; 
    return out;
}

ostream& operator<<(ostream& out, const ship& s)
{
  out << "SHIP " << s.id <<";"<<s.p<<";"<<s.ori << ";"<<s.speed<<";"<<s.rhum; 
    return out;
}
ostream& operator<<(ostream& out, const barrel& b)
{
  out  << "BARR " << b.id <<";"<<b.p<<";"<<b.level; 
    return out;
}
ostream& operator<<(ostream& out, const ball& b)
{
  out << "BALL " << b.id <<";"<<b.p<<";"<<b.turn; 
    return out;
}

ostream& operator<<(ostream& out, const mine& m)
{
  out << "MINE " << m.id <<";"<<m.p; 
    return out;
}

using fv_ships_t = fast_vect<ship,3>;
using fv_barrels_t = fast_vect<barrel,26>;
using fv_balls_t = fast_vect<ball,50>;
using fv_mines_t = fast_vect<mine,50>;



 
class game_stat
{
public:
  void update_state()
  {
    reset_all_vect();
    int myShipCount; // the number of remaining ships
    cin >> myShipCount; cin.ignore();

    
    int entityCount; // the number of entities (e.g. ships, mines or cannonballs)
    cin >> entityCount; cin.ignore();
    for (int i = 0; i < entityCount; i++)
      {
	int entityId;
	string entityType;
	int x;
	int y;
	int arg1;
	int arg2;
	int arg3;
	int arg4;
	cin >> entityId >> entityType >> x >> y >> arg1 >> arg2 >> arg3 >> arg4; cin.ignore();

	cerr << entityId << endl;
	if(entityType == "SHIP")
	  {
	    if(arg4 == 1)
	      my_ships.push_back(ship(entityId,{x,y},arg1,arg2,arg3));
	    else
	      adv_ships.push_back(ship(entityId,{x,y},arg1,arg2,arg3));
	    continue;
	  }

	if(entityType == "BARREL")
	  {
	    bars.push_back(barrel(entityId,{x,y},arg1));
	    continue;
	  }

	if(entityType == "CANNONBALL")
	  {
	    balls.push_back(ball(entityId,{x,y},NULL,arg2));
	    continue;
	  }

	if(entityType == "MINE")
	  {
	    mines.push_back(mine(entityId,{x,y}));
	    continue;
	  }
	
      }
  }

  inline int get_my_ship_count() const
  {
    return my_ships.size;
  }
  void print_state()
  {
    my_ships.print();
    adv_ships.print();
    bars.print();
    balls.print();
    mines.print();
  }

  
private:

  
  fv_ships_t my_ships;

  fv_ships_t adv_ships;
  fv_barrels_t bars;
  fv_balls_t balls;
  fv_mines_t mines;

  void reset_all_vect()
  {
    my_ships.reset();
    adv_ships.reset();
    bars.reset();
    balls.reset();
    mines.reset();
  }
};
/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main()
{
  game_stat gs;
    // game loop
  while (1)
    {
      gs.update_state();
      gs.print_state();
      for (int i = 0; i < gs.get_my_ship_count(); i++) {

            // Write an action using cout. DON'T FORGET THE "<< endl"
            // To debug: cerr << "Debug messages..." << endl;

            cout << "MOVE 11 10" << endl; // Any valid action, such as "WAIT" or "MOVE x y"
        }
    }
}


  
