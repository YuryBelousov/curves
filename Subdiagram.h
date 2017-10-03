#pragma once
#include "Domain.h"

using namespace std;

class subdiagram {
private:
	bool closed;
	vector <history_element> history;

	vector <int> gauss_code;
	vector <domain*> atlas;
	int number_of_points;
public:
	subdiagram(int num);
	subdiagram(vector <history_element>& _history);
	~subdiagram();

	void add_new_points(int n);
	void go_throught_point(int x, bool pass_below);

	bool check_correctness();
	bool find_simple_arc(int length = 8);

	void make_flip_in(int pos);
	bool find_simple_arc_with_flips(int length = 8, int after = 0);

	int get_number_of_points();
	vector<int> get_gc();

	bool only_one_way();
	bool close_up();

	set <int> available_points();

	void print(bool only_gc = false);
};
