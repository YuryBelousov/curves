#pragma once
#include <vector>
#include <set>
#include <iostream>
#include <math.h>

using namespace std;


const int not_pt = -7;
const int start_pt = -1;
const int end_pt = 0;

class domain;

class history_element {
private:
public:
	bool is_add;
	int point;
	history_element(bool _add, int _pt) : is_add(_add), point(_pt) {};
};

class domain_border_element {
private:
	int p;			//This can be a point, a the beginning/end of a curve or connection with other domain.
	domain* d;		//Pointer to the domain to which this element belongs
	int label;		//This use for labeling connections between domains

public:
	//constructor
	domain_border_element(int _p, domain* _d = nullptr, int _label = 0) 
		:p(_p), d(_d), label(_label) {}; 

	void set_point(int _pt) { p = _pt; };
	void set_domain(domain* _d) {d = _d; };
	void set_label(int _label) { label = _label; };

	domain* get_domain() { return d; };
	int get_label() { return label; };
	int get_point() { return p; }
};


class domain {
private:
	int d_num;									//name of the domain
	vector <domain_border_element> border;		//array with it's border elements
	static int label_counter;					//static member for labeling connections between domains

public:
	//Constructors
	//constructor for first domain
	domain() {		
		d_num = 0;
		border.push_back(start_pt);
		border.push_back(end_pt);
	}
	//basic constructor
	domain(int d_name) : d_num(d_name) {};

	const int get_name() { return d_num; };

	set <int> all_points() {
		set <int> temp;
		for (auto e : border) {
			int pt = e.get_point();
			if ((pt != end_pt) && (pt != start_pt) && (pt != not_pt))
				temp.insert(temp.find(pt) == temp.end() ? pt : -pt);
		}
		return temp;
	}

	domain_border_element* find_point(int pt) {
		for (int i = 0; i < border.size(); ++i)
			if (border[i].get_point() == pt)
				return &border[i];
		return nullptr;
	}

	domain_border_element* find_connection_with_label(int lab) {
		for (int i = 0; i < border.size(); ++i)
			if (border[i].get_label() == lab)
				return &border[i];
		return nullptr;
	}

	void add_new_points(int start_num, int n) {		
		//renew border
		vector <domain_border_element> new_border;
		for (int i = 0; i < border.size(); ++i) {		
			if (border[i].get_point() == end_pt) {		//If it is end of curve							
				for (int i = 0; i < n; ++i) {			//before end add new points and connections between domain
					domain_border_element new_nb(not_pt, this, ++label_counter);
					domain_border_element new_pt(start_num + i, this);
					new_border.push_back(new_nb);
					new_border.push_back(new_pt);					
				}
				new_border.push_back(border[i]);		//than add end point
				for (int i = 0; i < n; ++i) {			//and add new points and connections in reverse order	
					domain_border_element new_pt(start_num + n - i - 1, this);
					domain_border_element new_nb(not_pt, this, new_border[new_border.size() - 3 - 4 * i].get_label());
					new_border.push_back(new_pt);
					new_border.push_back(new_nb);
				}
			}
			else
				new_border.push_back(border[i]);
		}
		border = new_border;
	}

	void go_throught_point(int tr_point, domain* new_domain, bool pass_below) {
		//find indeces of transition point and of the end of the curve
		int ind_start = -1,
			ind_end = -1;
		for (int i = 0; i < border.size(); ++i) {
			if (border[i].get_point() == end_pt)
				ind_start = i;
			if (border[i].get_point() == tr_point){
				if (pass_below)
					ind_end = i;
				else
					if (ind_end == -1)
						ind_end = i;
			}
			
		}
		if (ind_start > ind_end) 
			swap(ind_start, ind_end);
	
		//change this domain border
		vector <domain_border_element> new_border;
		new_border.insert(new_border.end(), border.begin(), border.begin() + ind_start );
		domain_border_element new_border_element({ not_pt }, new_domain, ++label_counter);
		new_border.push_back(new_border_element);
		new_border.insert(new_border.end(), border.begin() + ind_end + 1, border.end());

		//change border of new domain
		new_border_element = domain_border_element({ not_pt }, this, label_counter);
		new_domain->border.push_back(new_border_element);
		new_domain->border.insert(new_domain->border.end(), border.begin() + ind_start + 1, border.begin() + ind_end);

		//change neighbor domain with transition point
		domain* neighbor_by_tr_point = find_point(tr_point)->get_domain();
		
		//renew border
		border = new_border;

		neighbor_by_tr_point->find_point(tr_point)->set_point(end_pt);

		//change neighbors of domain 
		for (int i = 1; i < new_domain->border.size(); ++i) {
			if ((new_domain->border[i].get_point() != end_pt) && (new_domain->border[i].get_point() != start_pt)) {
				domain* neighbor = new_domain->border[i].get_domain();
				domain_border_element* tmp = nullptr;
				if (new_domain->border[i].get_point() == not_pt)
					tmp = neighbor->find_connection_with_label(new_domain->border[i].get_label());
				else
					tmp = neighbor->find_point(new_domain->border[i].get_point());
				tmp->set_domain(new_domain);
			}
		}
	}

	void close_up() {
		vector <domain_border_element> new_border;
		for (int i = 0; i < border.size(); ++i) 
			if ((border[i].get_point() != end_pt) && (border[i].get_point() != start_pt))
				new_border.push_back(border[i]);
		border = new_border;
	}

	void print() {
		cout << "\n###### DOMAIN V_" <<d_num << " ######\n";
		for (int i = 0; i < border.size(); ++i) {
			switch (border[i].get_point()) {
			case start_pt:
				cout << "\t beginning of the curve\n";
				break;
			case end_pt:
				cout << "\t end of the curve\n";
				break;
			case not_pt:
				cout << "\t domain V_" << border[i].get_domain()->get_name() << " (it's label " << border[i].get_label() << ")\n";
				break;
			default:
				cout << "\t point " << (char)('a' + border[i].get_point() - 1) << " (it goes to V_" << border[i].get_domain()->get_name() << ")\n";
				break;
			}
		}
	}
};