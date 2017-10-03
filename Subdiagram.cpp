#include "Subdiagram.h"

subdiagram::subdiagram(int num) : number_of_points(0), closed(false){
	gauss_code.push_back(start_pt);						//Add begginng of the curve
	gauss_code.push_back(end_pt);						//Add end of the curve
	domain* v_0 = new domain;							//Create first domain
	atlas.push_back(v_0);								//Add first domain to atlas

	add_new_points(num);								//Add start points
}

subdiagram::subdiagram(vector <history_element>& _history) : number_of_points(0), closed(false) {
	gauss_code.push_back(start_pt);
	gauss_code.push_back(end_pt);
	domain* v_0 = new domain;
	atlas.push_back(v_0);
	for (auto e : _history)
		if (e.is_add)
			add_new_points(e.point);
		else
			go_throught_point(abs(e.point), e.point < 0);
}

subdiagram::~subdiagram(){
	for (int i = 0; i < atlas.size(); ++i)
		delete atlas[i];
}

void subdiagram::add_new_points(int n){
	if (n < 1)
		return;
	//Change gauss code
	gauss_code.pop_back();
	for (int i = 0; i < n; ++i)
		gauss_code.push_back(++number_of_points);
	gauss_code.push_back(end_pt);

	//Change areas
	for (int i = 0; i < atlas.size(); ++i)
		if (atlas[i]->find_point(end_pt)) {
			atlas[i]->add_new_points(number_of_points - n + 1, n);
			break;
		}

	//Add to history
	history.push_back({ true, n });
}

void subdiagram::go_throught_point(int x, bool pass_below){
	//change gauss code 
	gauss_code.pop_back();
	gauss_code.push_back(x);
	gauss_code.push_back(end_pt);

	//find domain with end of the curve
	domain* domain_with_end = atlas[0];
	for (int i = 0; i < atlas.size(); ++i)
		if (atlas[i]->find_point(end_pt) != nullptr) {
			domain_with_end = atlas[i];
			break;
		}
	//create new domain
	domain* new_domain = new domain(atlas.size());
	atlas.push_back(new_domain);
	domain_with_end->go_throught_point(x, new_domain, pass_below);	

	//Add to history
	history.push_back({ false, (pass_below) ? -x : x });
}

bool subdiagram::check_correctness(){
	if (closed) {
		for (int i = 0; i < gauss_code.size(); ++i) {
			//Check for trifol
			if ((gauss_code[i] == gauss_code[(i + 3) % gauss_code.size()]) &&
				(gauss_code[(i + 1) % gauss_code.size()] == gauss_code[(i + 4) % gauss_code.size()]) &&
				(gauss_code[(i + 2) % gauss_code.size()] == gauss_code[(i + 5) % gauss_code.size()]))
				return false;
			if ((gauss_code[i] == gauss_code[(i + 1) % gauss_code.size()]) || (gauss_code[i] == gauss_code[(i - 1) % gauss_code.size()]))
					return false;
		}
		return true;
	}
	for (int i = 1; i < gauss_code.size() - 1; ++i) {
		//Check for trifol
		if (i <(int) gauss_code.size() - 6) 
			if ((gauss_code[i] == gauss_code[(i + 3)]) &&
			(gauss_code[(i + 1)] == gauss_code[(i + 4)]) &&
			(gauss_code[(i + 2)] == gauss_code[(i + 5)]))
				return false;
		for (int j = i + 1; j < gauss_code.size() - 1; ++j)
			if ((gauss_code[i] == gauss_code[j]) && (((j - i) % 2 == 0) || (j == i + 1)))
				return false;
	}
	return true;
}

bool subdiagram::find_simple_arc(int length){
	if (length > number_of_points)
		return false;
	if (closed) {
		bool found = true;
		for (int i = 0; i < gauss_code.size(); ++i) {
			for (int j = i; j < i + length; ++j) {
				found = true;
				for (int k = j+1; k < i + length; ++k) {
					if (gauss_code[j % gauss_code.size()] == gauss_code[k  % gauss_code.size()]) {
						found = false;
						break;
					}
				}
				if (found == false)
					break;
			}
			if (found == true)
				return true;
		}
		return false;
	}

	bool found = true;
	for (int i = 1; i < (int)gauss_code.size() - length - 1; ++i) {
		for (int j = i; j < i + length; ++j) {
			found = true;
			for (int k = j+1; k < i + length; ++k) {
				if (gauss_code[j] == gauss_code[k]) {
					found = false;
					break;
				}
			}
			if (found == false)
				break;
		}
		if (found == true)
			return true;
	}
	return false;
}

void subdiagram::make_flip_in(int pos) {
	/*
	When we do flip it act on gauss code as folows:
	(a)(b)(c)(a)...(b||c)...(c||b)... -> (b)(c)...(a)(b||c)...(c||b)(a)...
	here (b||c) means b or c
	*/
	if (closed) {
		//root of loop with char 2
		int np = gauss_code[pos];
		//points on the border of loop: n_1, n_2
		set<int> neighbors({ gauss_code[(pos + 1) % gauss_code.size()] , gauss_code[(pos + 2) % gauss_code.size()] });
		//delete root of loop
		gauss_code.erase(gauss_code.begin() + ((pos > (pos + 3) % gauss_code.size()) ? pos : (pos + 3) % gauss_code.size()));
		gauss_code.erase(gauss_code.begin() + ((pos < (pos + 3) % gauss_code.size()) ? pos : (pos + 3) % gauss_code.size()));
		//positions of points on the border in gauss code after deliting the root
		int pos_n1 = pos + (pos + 3 > gauss_code.size()) ? -1 : 0,
			pos_n2 = (pos + 1 + (pos + 3 > gauss_code.size()) ? -1 : 0) % gauss_code.size();
		//find second entrance of n_1, n_2 (if there is)
		for (int j = 0; j < gauss_code.size(); ++j) {
			if (neighbors.find(gauss_code[j]) != neighbors.end())
				//if it is the first time we meet n_1 || n_2 and it is on different position
				if ((neighbors.size() == 2) && (j != pos_n1) && (j != pos_n2)) {
					gauss_code.insert(gauss_code.begin() + j, np);
					neighbors.erase(gauss_code[j + 1]);
					//if first entrance was before pos_n1
					if (j < pos_n1) {
						++pos_n1;
						++pos_n2;
					}
				}
				else
					if ((neighbors.size() == 1) && (j != pos_n1) && (j != pos_n2)) {
						gauss_code.insert(gauss_code.begin() + (pos + j + 1) % gauss_code.size(), np);
						break;
					}
		}
	}
	//root of loop with char 2
	int np = gauss_code[pos];
	//points on the border of loop
	set<int> neighbors({ gauss_code[pos + 1] , gauss_code[pos + 2] });
	//delete root of loop
	gauss_code.erase(gauss_code.begin() + pos + 3);
	gauss_code.erase(gauss_code.begin() + pos);
	//positions of points on the border in gauss code after deliting the root
	int pos_n1 = pos, 
		pos_n2 = pos + 1;
	//find new place for root
	for (int j = 0; j < gauss_code.size(); ++j) {
		if (neighbors.find(gauss_code[j]) != neighbors.end()) {
			if ((neighbors.size() == 2) && (j != pos_n1) && (j != pos_n2)) {
				gauss_code.insert(gauss_code.begin() + j, np);
				neighbors.erase(gauss_code[j + 1]);
				if (j < pos_n1) {
					++pos_n1;
					++pos_n2;
				}
			}
			else
				if ((neighbors.size() == 1) && (j != pos_n1) && (j != pos_n2)) {
					gauss_code.insert(gauss_code.begin() + j + 1, np);
					//swap(gauss_code[pos_n1], gauss_code[pos + j]);
					break;
				}
		}
	}
}

bool subdiagram::find_simple_arc_with_flips(int length, int after) {
	if (length > number_of_points)
		return false;
	if (find_simple_arc(length))
		return true;
	auto basic_gc = gauss_code;
	if (closed) {
		for (int i = after + 1; i < gauss_code.size(); ++i) {
			if (gauss_code[i] == gauss_code[(i + 3) % gauss_code.size()]) {
				make_flip_in(i);
				print(true);
				if (find_simple_arc_with_flips(length, i)) {
					gauss_code = basic_gc;
					return true;
				}
				gauss_code = basic_gc;
			}
		}
		gauss_code = basic_gc;
		return false;
	}
	for (int i = after + 1; i < gauss_code.size() - 3; ++i) {
		if (gauss_code[i] == gauss_code[i + 3]) {
			make_flip_in(i);
			if (find_simple_arc_with_flips(length, i)) {
				gauss_code = basic_gc;
				return true;
			}
			gauss_code = basic_gc;
		}
	}
	gauss_code = basic_gc;
	return false;
}

int subdiagram::get_number_of_points(){
	return number_of_points;
}

vector<int> subdiagram::get_gc(){
	return gauss_code;
}
	
bool subdiagram::only_one_way(){
	if (available_points().size() == 1)
		return true;
	if (available_points().size() == 2) {
		int temp = 0;
		for (auto e : available_points())
			temp += e;
		if (temp == 0)
			return true;
	}
	return false;
}

bool subdiagram::close_up(){
	if (number_of_points == gauss_code.size() / 2 - 1)
		for (int i = 0; i < atlas.size(); ++i)
			if ((atlas[i]->find_point(end_pt)) && (atlas[i]->find_point(start_pt))) {
				atlas[i]->close_up();
				closed = true;
				gauss_code.pop_back();
				gauss_code.erase(gauss_code.begin());
				return true;
			}
	return false;
}

set<int> subdiagram::available_points(){
	for (int i = 0; i < atlas.size(); ++i)
		if (atlas[i]->find_point(end_pt)) {
			return atlas[i]->all_points();
		}
}

void subdiagram::print(bool only_gc){
	cout << endl;
	for (int i = 0; i < gauss_code.size(); ++i)
		switch (gauss_code[i]){
		case start_pt:
			cout << ">-";
			break;
		case end_pt:
			cout << "\b->";
			break;
		default:
			cout << "[" << (char)('a' + gauss_code[i] - 1) << "] ";
			break;
		}
	cout << endl;
	if (!only_gc)
		for (int i = 0; i < atlas.size(); ++i)
			atlas[i]->print();
}
