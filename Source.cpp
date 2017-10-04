#include "Subdiagram.h"

int domain::label_counter = 0;
set<vector<int>> all_bad_gc;


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