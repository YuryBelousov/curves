#include "Subdiagram.h"

/*struct cycle_compare {
	bool operator() (const vector<int> gc1, const vector<int> gc2){
		if (gc1.size() < gc2.size)
			return true;

	}
};*/

int domain::label_counter = 0;
set<vector<int>> all_bad_gc;

/*
bool operator==(vector <int> gc1, vector <int> gc2) {
	if (gc)

	return true;
}*/

void brute_force(vector<history_element> history, history_element last_action) {
	history.push_back(last_action);
	subdiagram d(history);
	if (!(d.check_correctness()) || (d.find_simple_arc_with_flips(8)))
		return;
	if (history.back().is_add)
		for (auto e : d.available_points())
			brute_force(history, { false, e });
	else
		if (!d.only_one_way()) {	//Check for irreducible
			for (int i = 0; i < 5; ++i)
				brute_force(history, { true, i });
		}
		else brute_force(history, { true, 0 });
		if (d.close_up())
			if ((d.check_correctness()))
				all_bad_gc.insert(d.get_gc());
}

int main() {
	/*subdiagram k_3_1(3), k_4_1(4), k_7_7(6);
	k_3_1.go_throught_point(1, true);
	k_3_1.go_throught_point(2, true);
	k_3_1.go_throught_point(3, true);
	k_3_1.print();
	k_4_1.go_throught_point(2, true);
	k_4_1.go_throught_point(1, false);
	k_4_1.go_throught_point(4, true);
	k_4_1.go_throught_point(3, true);
	k_4_1.print();
	k_7_7.go_throught_point(4, true);
	k_7_7.add_new_points(1);
	k_7_7.go_throught_point(2, false);
	k_7_7.go_throught_point(1, true);
	k_7_7.go_throught_point(7, false);
	k_7_7.go_throught_point(3, false);
	k_7_7.go_throught_point(6, false);
	k_7_7.go_throught_point(5, false);
	k_7_7.print(true);
	cout << k_7_7.find_simple_arc(6);
	cout << k_7_7.find_simple_arc(7);
	k_7_7.close_up();
	k_7_7.print(true);
	*/

	/*subdiagram d(3);
	d.go_throught_point(1, true);
	d.add_new_points(3);
	d.go_throught_point(4, true);
	d.add_new_points(1);
	d.go_throught_point(6, true);
	d.go_throught_point(5, true);
	d.go_throught_point(7, true);
	d.add_new_points(1);
	d.go_throught_point(3, true);
	d.go_throught_point(2, true);
	d.go_throught_point(8, true);
	d.print(true);
	cout << d.find_simple_arc() << endl;
	cout << d.find_simple_arc_with_flips() << endl;*/

	for (int i = 3; i < 8; ++i) {
		brute_force({}, { true, i });
		cout << all_bad_gc.size() << " " << i << endl;
	}
	cout << all_bad_gc.size() << endl;
	system("pause");
	int max_size = 20;
	for (auto e : all_bad_gc) {
		if (e.size() == max_size) {
			for (int i = 0; i < e.size(); ++i)
				cout << "[" << (char)('a' + e[i] - 1) << "] ";
			cout << endl;
		}
	}
	system("pause");
	return 0;
}