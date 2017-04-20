#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <type_traits>
#include <cstring>

using namespace std;
const int MAP_WIDTH = 23;
const int MAP_HEIGHT = 21;

const int COOLDOWN_CANNON = 2;
const int COOLDOWN_MINE = 5;
const int FIRE_DISTANCE_MAX = 10;
const int MAX_SHIP_SPEED = 2;
const int MAX_SHIP_HEALTH = 100;


const int LOW_DAMAGE = 25;
const int HIGH_DAMAGE = 50;
const int MINE_DAMAGE = 25;
const int NEAR_MINE_DAMAGE = 10;
const int REWARD_RUM_BARREL_VALUE = 30;


const int directions[6][3] { { 1, -1, 0 }, { +1, 0, -1 }, { 0, +1, -1 }, { -1, +1, 0 }, { -1, 0, +1 }, { 0, -1, +1 } };
const int DIRECTIONS_EVEN[6][2] { { 1, 0 }, { 0, -1 }, { -1, -1 }, { -1, 0 }, { -1, 1 }, { 0, 1 } };

const   int DIRECTIONS_ODD[6][2] =  { { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, 0 }, { 0, 1 }, { 1, 1 } };


enum action_e {NONE,MINE,PORT,STARBOARD,FASTER,SLOWER,WAIT,FIRE};



template<typename T,int N>
class fast_vect
{
public:

  T arr[N];
  int size = 0;

  //fast_vect(T arr_init[N],int s):arr(arr_init),size(s){}
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
    //assert(size > 0);
    return &arr[size];
  }

  void remove_to_rem()
  {
    T arr_tmp[N];
    memcpy(arr_tmp,arr,size*sizeof(T));
    int j = 0;
    for(int i=0;i<size;++i)
      {
	if(arr_tmp[i].to_remove)
	  {
	    continue;
	  }
	arr[j++] = arr_tmp[i];
      }
    size = j;
  }
};


class CubeCoordinate
{
private:

  
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




  inline bool operator==(const pos& rhs) const
  {
    return rhs.x == x && rhs.y == y;
  }
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
  
  //private:
  

};

ostream& operator<<(ostream& out, const pos& p)
{
    out << "(" << p.x << ", " << p.y << ")"; 
    return out;
}

const action_e avail_act[5] =  {PORT,STARBOARD,FASTER,SLOWER,WAIT};

struct action
{
  action():act(NONE),arg({0,0}){}
  action(bool):arg({0,0}) //random init
  {
    init_random();
  }
  
  action_e act;
  pos arg;

  void init_random()
  {
    // random_shuffle(action_e.begin(), action_e.end());

    int ran = rand()%5;
    act = avail_act[ran];
  }

 

};


ostream& operator<<(ostream& out, const action& a)
{

  switch(a.act)
    {
    case WAIT:
      out << "WAIT";
      break;
    case FASTER:
      out << "FASTER";
      break;
    case SLOWER:
      out << "SLOWER";
      break;
    case PORT:
      out << "PORT";
      break;
    case STARBOARD:
      out << "STARBOARD";
      break;
    case FIRE:
      out << "FIRE " << a.arg.x << " " << a.arg.y;
      break;
    }
    return out;
}

struct entity
{
  entity():id(0),p{0,0}{}
  entity(short id_,pos p_):id(id_),p(p_){}
  short id;
  pos p;
  bool to_remove = false;
};

struct ship : public entity
{
  ship():entity(),ori(0),speed(0),rhum(0){}
  ship(short id,pos p,short ori_,short speed_,short rhum_):entity(id,p),ori(ori_),speed(speed_),rhum(rhum_),initial_rhum(rhum_){}
  int ori,speed,rhum;

  bool killed_by_exp = false;
  int initial_rhum;
  int cannonCooldown = 0;
  int mineCooldown = 0;
  action perf_action;// = NONE;

  pos newPosition{-1,-1};
  pos newBowCoordinate{-1,-1};
  pos newSternCoordinate{-1,-1};
  int newOrientation = 0;

  inline void damage(int health)
  {
    rhum -= health;
    if (rhum <= 0)
      {
	rhum = 0;
	killed_by_exp = true;
      }
  }

  
  inline pos stern() const
  {
    return p.neighbor((ori + 3) % 6);
  }

  inline pos bow() const
  {
    return p.neighbor(ori);
  }

  pos newStern() const
  {
    return p.neighbor((newOrientation + 3) % 6);
  }

  pos newBow() const
  {
    return p.neighbor(newOrientation);
  }

  inline bool newPositionsIntersect(const ship &other) const
  {
    bool sternCollision =  newSternCoordinate == other.newBowCoordinate || newSternCoordinate == other.newPosition || newSternCoordinate == other.newSternCoordinate;
    
    bool centerCollision = newPosition == other.newBowCoordinate || newPosition == other.newPosition || newPosition == other.newSternCoordinate;
    return newBowIntersect(other) || sternCollision || centerCollision;
  }

  inline bool newPositionsIntersect(const fast_vect<ship,3>& s1,const fast_vect<ship,3>& s2) const
  {

    for(int i=0;i<s1.size + s2.size;++i)
      {
	const ship *cur_ship;
	if(i<s1.size)
	  {
	    cur_ship = &s1.arr[i];
	  }
	else
	  {
	    cur_ship = &s2.arr[i-s1.size];
	  }

	
	if (cur_ship->id != id && newPositionsIntersect(*cur_ship))
	  {
	    return true;
	  }
      }
    return false;
  }
   
  
  inline bool newBowIntersect(const ship &other) const
  {
    
    bool b =  (newBowCoordinate == other.newBowCoordinate || newBowCoordinate == other.newPosition || newBowCoordinate == other.newSternCoordinate);

    //cerr << newBowCoordinate << " oth " <<  other.newBowCoordinate << " " << b << endl;
    
    return b;
  }

  inline bool newBowIntersect(const fast_vect<ship,3>& s1,const fast_vect<ship,3>& s2) const
  {

    for(int i=0;i<s1.size + s2.size;++i)
      {
	const ship *cur_ship;
	if(i<s1.size)
	  {
	    cur_ship = &s1.arr[i];
	  }
	else
	  {
	    cur_ship = &s2.arr[i-s1.size];
	  }

	
	if (cur_ship->id != id && newBowIntersect(*cur_ship))
	  {
	    return true;
	  }
      }
    return false;
  }

  inline void heal(int health)
  {
    rhum += health;
    if (rhum > MAX_SHIP_HEALTH)
      {
      rhum = MAX_SHIP_HEALTH;
      }
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


using fv_collision_t = fast_vect<ship*,30>;

fv_collision_t collisions;

class game_stat
{
public:
  void update_state()
  {
    reset_all_vect();
    int myShipCount; // the number of remaining ships
    cin >> myShipCount; cin.ignore();
    
    
    int entity_count;
    cin >> entity_count; cin.ignore();
    for (int i = 0; i < entity_count; i++)
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

	cerr << entityId <<" "<< entityType <<" "<< x<<" " << y<<" " << arg1<<" " << arg2<<" " << arg3 <<" "<< arg4 << endl;

	
	//cerr << entityId << endl;
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


  int simul_next_state(fv_actions_t a_play, fv_actions_t a_adv, game_stat& new_gs) const
  {
    new_gs = *this;


    
    //update ship
    for_each(new_gs.my_ships.begin(),new_gs.my_ships.end(),[](ship &s){s.rhum=max(0,s.rhum - 1);});
    for_each(new_gs.adv_ships.begin(),new_gs.adv_ships.end(),[](ship &s){s.rhum=max(0,s.rhum - 1);});

    //update cannonball

    for_each(new_gs.balls.begin(),new_gs.balls.end(),[](ball &b){
	if(b.turn == 0)
	  {
	    b.to_remove = true;
	    return;
	  }
	if(b.turn > 0)
	  --b.turn;

	
      });

    
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


    apply_action(new_gs);
    move_ships(new_gs);
    rotate_ships(new_gs);
    explode_min_bar(new_gs);
    
    //remove 0 ship
    for(ship &s: new_gs.adv_ships)
      {
	if(s.rhum <= 0)
	  {
	    s.to_remove = true;
	    //add new rum
	    if(s.killed_by_exp)
	      new_gs.bars.push_back(barrel(new_gs.get_new_id(),s.p,min(s.initial_rhum,REWARD_RUM_BARREL_VALUE)));
	  }
      }

    for(ship &s: new_gs.my_ships)
      {
	if(s.rhum <= 0)
	  {
	    s.to_remove = true;
	    //add new rum
	    if(s.killed_by_exp)
	      new_gs.bars.push_back(barrel(new_gs.get_new_id(),s.p,min(s.initial_rhum,REWARD_RUM_BARREL_VALUE)));
	  }
      }
      
    new_gs.my_ships.remove_to_rem();
    new_gs.adv_ships.remove_to_rem();
    new_gs.bars.remove_to_rem();
    new_gs.balls.remove_to_rem();
    new_gs.mines.remove_to_rem();

    if(new_gs.my_ships.size == 0)
      return 0; //adv win
    else if(new_gs.adv_ships.size == 0)
      return 1; //I win
    else return 2; //no win now
  }
  
  void apply_action(game_stat& new_gs) const
  {
    
    fv_ships_t *all_ships[2] = {&new_gs.adv_ships,&new_gs.my_ships};
    
    for(int p =0;p<2;++p)
      {
	for(ship &s:*all_ships[p]) //move all ships (adv and me)
	  {
	    if (s.mineCooldown > 0)
	      {
		s.mineCooldown--;
	      }
	    if (s.cannonCooldown > 0)
	      {
		s.cannonCooldown--;
	      }

	    s.newOrientation = s.ori;

	    switch (s.perf_action.act)
	      {
	      case WAIT:
		break;
	      case FASTER:
		if(s.speed < 2) ++(s.speed);
		break;
	    
	      case SLOWER:
		if(s.speed > 0) --(s.speed);
		break;
	    
	      case PORT:
		//s.ori = (s.ori + 1) % 6;
		s.newOrientation = (s.ori + 1) % 6;
		break;
		
	      case STARBOARD:
		//s.ori = (s.ori + 5) % 6;
		s.newOrientation = (s.ori + 5) % 6;
		break;
	      case FIRE:
		{
		  int distance = s.bow().distanceTo(s.perf_action.arg);
		  if (s.perf_action.arg.isInsideMap() && distance <= FIRE_DISTANCE_MAX && s.cannonCooldown == 0)
		    {
		      int travelTime = (int) (1 + round(s.bow().distanceTo(s.perf_action.arg) / 3.0));
		      //cannonballs.add(new Cannonball(ship.target.x, ship.target.y, ship.id, ship.bow().x, ship.bow().y, travelTime));
		      
		      new_gs.balls.push_back(ball(new_gs.get_new_id(),s.perf_action.arg,0,travelTime));
		      s.cannonCooldown = COOLDOWN_CANNON;
		    }
		}
		break;
	      default:
		assert(false);
	      
	      }

	  }
      }
  }

  
  //function<void(ship&)> fct_move_ship = [&new_gs](ship &s)
  void move_ships(game_stat& new_gs) const
  {

    fv_ships_t *all_ships[2] = {&new_gs.adv_ships,&new_gs.my_ships};

    
    for (int i = 1; i <= MAX_SHIP_SPEED; i++)
      {
	for(int p =0;p<2;++p)
	  {
	    for(ship &s:*all_ships[p])
	      {
		//cerr << s.speed << " i " <<i << (i>s.speed) << endl;
		s.newPosition = s.p;
		s.newBowCoordinate = s.bow();
		s.newSternCoordinate = s.stern();
		
		if (i > s.speed)
		  {
		    continue;
		  }
	
		pos newCoordinate = s.p.neighbor(s.ori);
		if (newCoordinate.isInsideMap())
		  {
		    // Set new coordinate.
		    s.newPosition = newCoordinate;
		    
		    s.newBowCoordinate = newCoordinate.neighbor(s.ori);
		    s.newSternCoordinate = newCoordinate.neighbor((s.ori + 3) % 6);

		    //cerr << "map " << s.newPosition << s.newBowCoordinate << s.newSternCoordinate << endl;
		  } else
		  {
		    // Stop ship!
		    //cerr << "out map " << endl;
		    s.speed = 0;
		  }
		//cerr << "map " << s.newPosition << s.newBowCoordinate << s.newSternCoordinate << endl;
	      }
	  }
      
    

      
    bool collisionDetected = true;
    while (collisionDetected)
      {
	collisionDetected = false;
	
	
	for(int i=0;i<new_gs.my_ships.size + new_gs.adv_ships.size;++i)
	  {
	    ship *cur_ship;
	    if(i<new_gs.my_ships.size)
	      {
		cur_ship = &new_gs.my_ships.arr[i];
	      }
	    else
	      {
		cur_ship = &new_gs.adv_ships.arr[i-new_gs.my_ships.size];
	      }

	    //cerr << "tt " <<  cur_ship->id << endl;
	    //cerr << "col " << collisions.size << endl;
	    
	    if (cur_ship->newBowIntersect(new_gs.my_ships,new_gs.adv_ships))
	      {
		//cerr << "COL " << endl;
		
		collisions.push_back(cur_ship);
		//		exit(1);
	      }
	  }
	
	
	for (ship *s : collisions)
	  {
	    // Revert last move
	    s->newPosition = s->p;
	    s->newBowCoordinate = s->bow();
	    s->newSternCoordinate = s->stern();
	    
	    // Stop ships
	    s->speed = 0;
	    
	    collisionDetected = true;
	  }
	  collisions.reset();
      }

    // Move ships to their new location
    for(int i=0;i<new_gs.my_ships.size + new_gs.adv_ships.size;++i)
      {
	ship *cur_ship;
	if(i<new_gs.my_ships.size)
	  {
	    cur_ship = &new_gs.my_ships.arr[i];
	  }
	else
	  {
	    cur_ship = &new_gs.adv_ships.arr[i-new_gs.my_ships.size];
	  }
	
	cur_ship->p = cur_ship->newPosition;
      }
    
    // Check collisions
    for(int i=0;i<new_gs.my_ships.size + new_gs.adv_ships.size;++i)
      {
	ship *cur_ship;
	if(i<new_gs.my_ships.size)
	  {
	    cur_ship = &new_gs.my_ships.arr[i];
	  }
	else
	  {
	    cur_ship = &new_gs.adv_ships.arr[i-new_gs.my_ships.size];
	  }
	checkCollisions(new_gs,*cur_ship);
      }
      }
  }

  void rotate_ships(game_stat& new_gs) const
  {
    // Rotate
    fv_ships_t *all_ships[2] = {&new_gs.adv_ships,&new_gs.my_ships};
    for(int p=0;p<2;++p)
      {
	for ( ship &s : *all_ships[p])
	  {
	    s.newPosition = s.p;
	    s.newBowCoordinate = s.newBow();
	    s.newSternCoordinate = s.newStern();
	  }
      }
  
    bool collisionDetected = true;
    while (collisionDetected)
      {
	collisionDetected = false;
	
	
	for(int i=0;i<new_gs.my_ships.size + new_gs.adv_ships.size;++i)
	  {
	    ship *cur_ship;
	    if(i<new_gs.my_ships.size)
	      {
		cur_ship = &new_gs.my_ships.arr[i];
	      }
	    else
	      {
		cur_ship = &new_gs.adv_ships.arr[i-new_gs.my_ships.size];
	      }

	    cerr << *cur_ship << " " << cur_ship->newOrientation << endl;
	    if (cur_ship->newPositionsIntersect(new_gs.my_ships,new_gs.adv_ships))
	      {
		
		collisions.push_back(cur_ship);
		//exit(1);
	      }
	  }
	
	//cerr << "size col " << collisions.size << endl;
	for (ship *s : collisions)
	  {
	    //cerr << "col val " << *s << endl;
	    // Revert last move
	    s->newPosition = s->p;
	    s->newOrientation = s->ori;
	    s->newBowCoordinate = s->bow();
	    s->newSternCoordinate = s->stern();

  
		

	    
	    // Stop ships
	    s->speed = 0;
	    
	    collisionDetected = true;
	  }
	  collisions.reset();
      }

    // apply rotation
    for(int i=0;i<new_gs.my_ships.size + new_gs.adv_ships.size;++i)
      {
	ship *cur_ship;
	if(i<new_gs.my_ships.size)
	  {
	    cur_ship = &new_gs.my_ships.arr[i];
	  }
	else
	  {
	    cur_ship = &new_gs.adv_ships.arr[i-new_gs.my_ships.size];
	  }
	
	//cur_ship->p = cur_ship->newPosition;
	cur_ship->ori = cur_ship->newOrientation;
      }
    
    // Check collisions
    for(int i=0;i<new_gs.my_ships.size + new_gs.adv_ships.size;++i)
      {
	ship *cur_ship;
	if(i<new_gs.my_ships.size)
	  {
	    cur_ship = &new_gs.my_ships.arr[i];
	  }
	else
	  {
	    cur_ship = &new_gs.adv_ships.arr[i-new_gs.my_ships.size];
	  }
	checkCollisions(new_gs,*cur_ship);
      }
    
        
    }
  




  void checkCollisions(game_stat &gs,ship &s) const
  {
    pos bow = s.bow();
    pos stern = s.stern();
    pos center = s.p;
    
    // Collision with the barrels
    for (barrel &b:gs.bars)
      {
	
	if (!b.to_remove && ( b.p == bow || b.p == stern || b.p == center))
	  {
	    s.heal(b.level);
	    b.to_remove = true;
	  }
      }

    //colision with the cannonball
    for(ball &cb:gs.balls)
      {
	if(cb.turn == 0 && !cb.to_remove  )
	  {
	    if(cb.p == s.bow() || cb.p == s.stern())
	      {
		s.damage(LOW_DAMAGE);
		cb.to_remove = true;
	      }
	    else if(cb.p == s.p)
	      {
		s.damage(HIGH_DAMAGE);
		cb.to_remove = true;
	      }
	  }
      }

    // Collision with the mines
    for(mine &m:gs.mines)
      {
	if(!m.to_remove)
	  {
	    if (m.p == s.bow() || m.p == s.stern() || m.p == s.p)
	      {
		//cerr << s.bow() << s.stern() << s.p << endl;
		//cerr << "** min dam " << m << " " << s << endl;
		s.damage(MINE_DAMAGE);
		m.to_remove = true;
	      }

	    /*
	    else if (s.stern().distanceTo(m.p) <= 1 || s.bow().distanceTo(m.p) <= 1 || s.p.distanceTo(m.p) <= 1)
	      {
		cerr << "**  min petit dam " << m << " " << s << endl;
		s.damage(NEAR_MINE_DAMAGE);
		m.to_remove = true;
		}*/
	  }
      }
    
    }

  void explode_min_bar(game_stat &gs) const
  {
    for(ball &cb:gs.balls)
      {
	for(mine &m:gs.mines)
	  {
	    if((cb.turn == 0 && !m.to_remove && !cb.to_remove) && (m.p == cb.p))
	      {
		m.to_remove = true;
		cb.to_remove = true;
	      }
	  }
      }
    for(ball &cb:gs.balls)
      {
	for(barrel &b:gs.bars)
	  {
	    if((cb.turn == 0 && !b.to_remove && !cb.to_remove) && (b.p == cb.p))
	      {
		b.to_remove = true;
		cb.to_remove = true;
	      }
	  }
      }
    
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

  
  //private:

  
  fv_ships_t my_ships;

  fv_ships_t adv_ships;
  fv_barrels_t bars;
  fv_balls_t balls;
  fv_mines_t mines;

  int orig_id = 0;

  
  inline int get_new_id()
  {
    ++orig_id;
    return orig_id;
  }
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

void bench(const game_stat &gs)
{


  game_stat curr_gs = gs;

  for(int t=0;t<1000;++t)
    {
      game_stat new_gs;

      fv_actions_t a1,a2;
      
      for (int i = 0; i < curr_gs.my_ships.size; i++)
	a1.push_back(action(true));
      for (int i = 0; i < curr_gs.adv_ships.size; i++)
	a2.push_back(action(true));
      
      if(curr_gs.simul_next_state(a1,a2, new_gs) != 2)
	{
	  cerr << "VICTO " << endl;
	  return;
	}

      //new_gs.print_state();
      curr_gs = new_gs;
    }
}

int main()
{
  srand(time(NULL));
  game_stat gs;
    // game loop
  while (1)
    {
      gs.update_state();
      gs.print_state();

      cerr << " BENCH " << endl;

      bench(gs);
      cerr << "END  BENCH " << endl;
      
      game_stat new_gs;
      fv_actions_t a1,a2;
      
   
      for (int i = 0; i < gs.my_ships.size; i++)
	a1.push_back(action(true));
      for (int i = 0; i < gs.adv_ships.size; i++)
	a2.push_back(action(true));
      
      gs.simul_next_state(a1,a2, new_gs);
      new_gs.print_state();
      
      for (action &a:a1) {

            // Write an action using cout. DON'T FORGET THE "<< endl"
            // To debug: cerr << "Debug messages..." << endl;

            cout << a << endl; // Any valid action, such as "WAIT" or "MOVE x y"
        }
    }
}



  
