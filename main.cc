#include <map>
#include <chrono>
#include <thread>
#include <iostream>
#include "/public/read.h"
#include "/public/colors.h"

using namespace std;

void die(string blank = "Exited"){
	set_raw_mode(false);
	system("clear");
	set_mouse_mode(false);
	show_cursor(true);
	cerr << blank << endl;
	exit(0);
}

struct Color {
	int r=0,g=0,b=0;
	Color(int new_r, int new_g, int new_b){
		r = new_r;
		g = new_g;
		b = new_b;
	}
	auto operator<=>(const Color &c) const = default;
};

const map<string, vector<Color>> palettes = {
	{"Endesga 32",{ //Default palette
					  {24,20,37}, // The Void
					  {38,43,68}, // Roads
					  {62,137,72}, // Grass 
					  {25,60,62}, // Mountains
					  {234,212,170}, // Deserts
					  {18,78,137}, // Rivers
					  {255,255,255}, // Arctic
					  {99,199,77}, // Swamp
					  {62,39,49} // Walls 
				  }
	},
	{"SLSO8", {
				  {13,43,69},   //Extra palettes for the big brains
				  {32,60,86},
				  {84,78,104},
				  {141,105,122},
				  {208,129,89},
				  {255,170,94},
				  {255,212,163},
				  {255,236,214},
				  {0,0,0}
			  }
	},
	{"Kirokaze", {
					 {51,44,80}, 
					 {70,135,143},
					 {148,227,68},
					 {226,243,228},
					 {0,0,0},
					 {0,0,0},
					 {0,0,0},
					 {0,0,0},
					 {0,0,0}
				 }
	},
	{"Chocolate Milk", {
						   {214,245,228},
						   {247,255,197},
						   {180,199,136},
						   {138,137,105},
						   {139,106,68},
						   {98,85,76},
						   {70,60,60},
						   {49,38,41},
						   {0,0,0}
					   }
	},
	{"Steam Lords", {
						{33,59,37},
						{58,96,74},
						{161,159,124},
						{119,116,79},
						{59,33,55},
						{23,14,25},
						{79,82,119},
						{105,115,140},
						{192,209,204}
					}
	}
};

struct Point {
	int x=0,y=0;
	int r=0,g=0,b=0; //For map saving
	auto operator<=>(const Point &p) const = default;
	friend ostream& operator<< (ostream &outs, const Point &p) {
		return outs << '(' << p.x << ',' << p.y << ')';
	}
};

struct Quadtree {
	bool leaf = false;
	Point p; //If we are a leaf, then this is the point we contain.
	Point min, mid1, mid2, max;
	Quadtree *ul=0, *ur=0, *ll=0, *lr=0;
	Quadtree(const Point &new_min, const Point &new_max) : min(new_min), max(new_max) {
		mid1.x = (min.x + max.x)/2;
		mid1.y = (min.y + max.y)/2;
		mid2.x = (min.x + max.x)/2 + 1;
		mid2.y = (min.y + max.y)/2 + 1;
	}
	void operator=(const Quadtree &q) = delete;
	Quadtree(const Quadtree &q) = delete;
	~Quadtree() { 
		delete ul;
		delete ur;
		delete ll;
		delete lr;
	}
	bool inside(const Point &p) {
		return (min.x <= p.x and p.x <= max.x and min.y <= p.y and p.y <= max.y);
	}
	void insert(const Point &p) {
		if (!leaf and !ul) {
			leaf = true;
			this->p = p;
			return;
		}
		if (p == this->p) return; //Discard duplicates
		leaf = false;
		if (!ul) {
			ul = new Quadtree({min.x,mid2.y},{mid1.x,max.y});
			ll = new Quadtree(min,mid1);
			ur = new Quadtree(mid2,max);
			lr = new Quadtree({mid2.x,min.y},{max.x,mid1.y});
			if (ul->inside(this->p)) ul->insert(this->p);
			else if (ll->inside(this->p)) ll->insert(this->p);
			else if (ur->inside(this->p)) ur->insert(this->p);
			else if (lr->inside(this->p)) lr->insert(this->p);
			else die("Failed to insert");
		}
		if (ul->inside(p)) ul->insert(p);
		else if (ll->inside(p)) ll->insert(p);
		else if (ur->inside(p)) ur->insert(p);
		else if (lr->inside(p)) lr->insert(p);
		else die("Failed to insert");
	}
	bool search(const Point &p) {
		if (leaf) {
			//cout << p << " " << this->p << endl;
			if (p == this->p) return true;
			else return false;
		}
		if (ul and ul->inside(p)) return ul->search(p);
		if (ll and ll->inside(p)) return ll->search(p);
		if (ur and ur->inside(p)) return ur->search(p);
		if (lr and lr->inside(p)) return lr->search(p);
		return false;
	}
	bool raycast() {
		return false;
	}
};
Quadtree *root = new Quadtree({0,0},{100,100});

void print_colors(){
	int x = 0; //For the options
	for(auto it = palettes.begin(); it != palettes.end(); it++){
		//for(const auto & [key, value] : palettes){
		cout << ++x << ") " << it->first << endl;
		for(Color c: it->second){
			setbgcolor(c.r, c.g, c.b);
			cout  << " ";
		}
		resetcolor();
		cout << endl;
	}
	}

	pair<int,int> rc = get_terminal_size();
	
	void print_world(Color c = {255,255,255}){
		//TODO Refactor this boi with the tree somehow
		for(size_t i = 0; i < rc.first-1; i++){
			for(size_t j = 0; j < rc.second; j++){ //TODO This does not work AND it crashes lmao
				Point p(i , j);
				if(root->search(p)){
					setbgcolor(c.r,c.g,c.b);
					cout << " "; 
				} 
				else {
					setbgcolor(0,0,0);
					cout << " ";
				}
			}	
			cout << endl;
		}
		resetcolor();
	}

	int click_r = 0, click_c = 0;
	bool clicked = false;
	void get_click(int row, int col){
		click_r = row;
		click_c = col;
		clicked = true;
	}

	void stop_click(int row, int col){
		clicked = false;
	}

	void change_pixel(Color c = {255,255,255}, int row = 0, int col = 0){
		Point p (row, col);
		p.r = c.r;
		p.g = c.g;
		p.b = c.b;
		root->insert(p);
		movecursor(click_r, click_c);
		setbgcolor(c.r,c.g,c.b);
		cout << " ";
		resetcolor();
		//TODO do some sort of onmouseup event then, if still clicked, recursively call change_pixel
		//Who knows if that will work

	}

	void print_palette(vector<Color> palette, int selected = 1){
		int x = 1;
		for(Color c: palette){
			if(x == selected){
				setbgcolor(255,255,255);
				setcolor(0,0,0);
			}
			cout << x++ << ") ";
			setcolor(255,255,255);
			setbgcolor(c.r, c.g, c.b);
			cout << " ";
			resetcolor();
			cout << "\t";
		}
	}

	//40 and 211 are god numbers
	void init(){
		system("figlet Welcome!");
		show_cursor(false);
	}

	int main() {
		init();
		//Quadtree *root = new Quadtree({0,0},{100,100}); 

		int input = 0;
		string palette_choice;
		vector<Color> palette;

		cout << endl << "Your first task is to pick a color palette!\n\n";

		print_colors();
		while(true){
			set_raw_mode(true);
			input = quick_read();
			if(input == 'q') break;
			if(input == ERR) usleep(10'000);
			if(input == '1'){
				system("clear");
				cout << endl << "You've chosen: Chocolate Milk" << endl;
				palette_choice = "Chocolate Milk";
				palette = palettes.at(palette_choice);
				break;
			} else if(input == '2'){
				system("clear");
				cout << endl << "You've chosen: Endesga 32" << endl;
				palette_choice = "Endesga 32";
				palette = palettes.at(palette_choice);
				break;
			} else if(input == '3'){
				system("clear");
				cout << "You've chosen: Kirokaze" << endl;
				palette_choice = "Kirokaze";
				palette = palettes.at(palette_choice);
				break;
			} else if(input == '4'){
				system("clear");
				cout << "You've chosen: SLSO8" << endl;
				palette_choice = "SLSO8";
				palette = palettes.at(palette_choice);
				break;
			} else if(input == '5'){
				system("clear");
				cout << "You've chosen: Steam Lords" << endl;
				palette_choice = "Steam Lords";
				palette = palettes.at(palette_choice);
				break;
			}
		}
		for(Color c: palette){
			setbgcolor(c.r, c.g, c.b);
			cout << " ";
		}
		resetcolor();
		cout << endl;
		usleep(1000000);

		system("clear");
		//slow_cout("Let's build the map!");
		cout << "Let's build the map!\n";
		//FIXME
		//Needs to use the quadtree 
		//Needs to not infinite loop
		//Needs to read input from mouse and keyboard at the same time 
		unordered_map<char, int> options = {
			{'1', 1},
			{'2', 2},
			{'3', 3},
			{'4', 4},
			{'5', 5},
			{'6', 6},
			{'7', 7},
			{'8', 8},
			{'9', 9}
		};
		print_world();
		print_palette(palette);
		int selected = 1; //The color they currently have selected 
		//int cur_r = 0, cur_g = 0, cur_b = 0; //Will act as the current color
		Color curr_color(255,255,255);
		while(true){
			set_mouse_mode(true);
			int user = quick_read();
			if(char(user) == 'q') break;
			for(pair<char, int> p: options){
				if(char(user) == p.first){
					if(selected != p.second){
						system("clear");
						print_world();
						print_palette(palette, p.second);
						selected = p.second;
						curr_color = palette.at(selected-1);
						cout << "RGB: " << curr_color.r << " " << curr_color.g << " " << curr_color.b << endl;
						cout << "\tPress q to quit";
					}
				}
			}

			on_mousedown(get_click);

			if(clicked){
				change_pixel(curr_color, click_r, click_c);
				clicked = false;
			}

			//system("clear");
			//print_world();
			//print_palette(palette, 1);
			usleep(10000);
			//cout << endl;
		}
		delete root;
		die();
	}

