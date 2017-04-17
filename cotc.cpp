#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>

using namespace std;
const int MAP_WIDTH = 23;
const int MAP_HEIGHT = 21;

const int COOLDOWN_CANNON = 2;
const int COOLDOWN_MINE = 5;
const int FIRE_DISTANCE_MAX = 10;
const int MAX_SHIP_SPEED = 2;
enum action_e {NONE,MINE,PORT,STARBOARD,FASTER,SLOWER,WAIT,FIRE};



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


  void print() const
  {
    for(int i =0;i<size;++i)
      {
	cerr << arr[i] << endl;
      }
  }

  T* begin() {return &arr[0];}
  T* end()
  {
    assert(size > 0);
    return &arr[size - 1];
  }
};


class CubeCoordinate
{
private:
  static constexpr int directions[6][3] { { 1, -1, 0 }, { +1, 0, -1 }, { 0, +1, -1 }, { -1, +1, 0 }, { -1, 0, +1 }, { 0, -1, +1 } };
  
  int x, y, z;
public:
  CubeCoordinate(int x_, int y_, int z_):x(x_),y(y_),z(z_) {}
  

  /* pos toOffsetCoordinate() const
  {
    int newX = x + (z - (z & 1)) / 2;
    int newY = z;
    return pos(newX, newY);
    }*/

  CubeCoordinate neighbor(int orientation) const
  {
    int nx = x + directions[orientation][0];
    int ny = y + directions[orientation][1];
    int nz = z + directions[orientation][2];

    return CubeCoordinate(nx, ny, nz);
  }
  int distanceTo(const CubeCoordinate &dst) const
  {
    return (abs(x - dst.x) + abs(y - dst.y) + abs(z - dst.z)) / 2;
  }

  
};


struct pos
{
  pos(int x_,int y_):x(x_),y(y_){}
  int x,y;

  inline pos neighbor(int orientation) const
  {
    int newY, newX;
    if (y % 2 == 1)
      {
	newY = y + DIRECTIONS_ODD[orientation][1];
	newX = x + DIRECTIONS_ODD[orientation][0];
      }
    else
      {
	newY = y + DIRECTIONS_EVEN[orientation][1];
	newX = x + DIRECTIONS_EVEN[orientation][0];
      }

    return  pos(newX, newY);
  }

  inline bool isInsideMap() const
  {
    return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;
  }
  
  CubeCoordinate toCubeCoordinate() const
  {
    int xp = x - (y - (y & 1)) / 2;
    int zp = y;
    int yp = -(xp + zp);
    return CubeCoordinate(xp, yp, zp);
  }
  
  int distanceTo(const pos &dst) const
  {
    return toCubeCoordinate().distanceTo(dst.toCubeCoordinate());
  }
  
private:
   static constexpr int DIRECTIONS_EVEN[6][2] =  { { 1, 0 }, { 0, -1 }, { -1, -1 }, { -1, 0 }, { -1, 1 }, { 0, 1 } };
  static constexpr int DIRECTIONS_ODD[6][2] =  { { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, 0 }, { 0, 1 }, { 1, 1 } };

};

struct action
{
  action_e act = NONE;
  pos arg{0,0};
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
  int ori,speed,rhum;

  int cannonCooldown = COOLDOWN_CANNON;
  action perf_action;// = NONE;

  inline pos stern() const
  {
    return p.neighbor((ori + 3) % 6);
  }

  inline pos bow() const
  {
    return p.neighbor(ori);
  }
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

using fv_actions_t = fast_vect<action,3>;


 
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


  void simul_next_state(fv_actions_t a_play, fv_actions_t a_adv, game_stat& new_gs) const
  {

    new_gs = *this;

    //update barrels
    for_each(new_gs.my_ships.begin(),new_gs.my_ships.end(),[](ship &s){s.rhum=max(0,s.rhum - 1);});
    for_each(new_gs.adv_ships.begin(),new_gs.adv_ships.end(),[](ship &s){s.rhum=max(0,s.rhum - 1);});

    //update cannonball

    for_each(new_gs.balls.begin(),new_gs.balls.end(),[](ball &b){assert(b.turn > 0);--b.turn;});

    
    for(int i=0;i<a_play.size;++i)
      {
	assert(a_play.size == new_gs.my_ships.size);

	new_gs.my_ships.arr[i].perf_action = a_play.arr[i];
      }
    
    for(int i=0;i<a_adv.size;++i)
      {
	assert(a_adv.size == new_gs.adv_ships.size);

	new_gs.adv_ships.arr[i].perf_action = a_adv.arr[i];
      }
    

    function<void(ship&)> fct_action = [&new_gs](ship &s)
      {


	switch (s.perf_action.act)
	  {
	  case FASTER:
	    if(s.speed < 2) ++(s.speed);
	    break;
	    
	  case SLOWER:
	    if(s.speed > 0) --(s.speed);
	    break;
	    
	  case PORT:
	    s.ori = (s.ori + 1) % 6;
	    break;
	    
	  case STARBOARD:
	    s.ori = (s.ori + 5) % 6;
	    break;
	  case FIRE:
	  int distance = s.bow().distanceTo(s.perf_action.arg);
	  if (s.perf_action.arg.isInsideMap() && distance <= FIRE_DISTANCE_MAX && s.cannonCooldown == 0)
	    {
	      int travelTime = (int) (1 + round(s.bow().distanceTo(s.perf_action.arg) / 3.0));
	      //cannonballs.add(new Cannonball(ship.target.x, ship.target.y, ship.id, ship.bow().x, ship.bow().y, travelTime));

	      ball b(0,s.perf_action.arg,0,travelTime);
	      new_gs.balls.push_back(ball(0,s.perf_action.arg,0,travelTime));
	      s.cannonCooldown = COOLDOWN_CANNON;
	    }
	  }
      };

    
    function<void(ship&)> fct_move_ship = [&new_gs](ship &s)
      {
	for (int i = 1; i <= MAX_SHIP_SPEED; i++)
	  {
	    if (i > s.speed)
	      {
		continue;
	      }

	    pos newCoordinate = s.p.neighbor(s.ori);
	    if (newCoordinate.isInsideMap())
	      {
		// Set new coordinate.
		s.p = newCoordinate;
		//ship.newBowCoordinate = newCoordinate.neighbor(ship.orientation);
		//ship.newSternCoordinate = newCoordinate.neighbor((ship.orientation + 3) % 6);
	      } else
	      {
		// Stop ship!
		s.speed = 0;
	      }
	  }
      };
  


    
  }

  
  inline int get_my_ship_count() const
  {
    return my_ships.size;
  }
  void print_state() const
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


  
