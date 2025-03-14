#pragma once
#include <vector>
#include <unordered_set>
#include <iostream>

/*
 * This file defines the `domain` class, which represents regions into which a curve divides the plane.
 * Each domain is described by its boundary, represented by a vector of integeres. Each positive integer
 * corresponds to a marked point that later will become double points on the curve. If the integer is
 * negative it reffers either the start or the end of an oriented curve.
 * 
 * Most memeber functions are technical and trivial, the only meaningful ones are:
 *		- add_new_marked_points_before_end,
 *		- go_throught_point.
 */


class domain {
private:
	constexpr static int end_point = -1'000'000;	// Technical value for the enpoint of the curve.
	constexpr static int start_point = -9'000'000;	// Technical value for the startpoint of the curve.

private:
	int _domain_label;			// Unique identifier.
	std::vector<int> _boundary;	// Labels of special points on the boundary. Either marked points or the ends of the curve.

private:
	// Checks if the boundary of this domain contains any element of a special type. If point_type > 0 then the marked point is searched.
	bool contains_special_point(int point_type) const noexcept {
		return std::find_if(_boundary.cbegin(), _boundary.cend(),
			[&point_type](int e) {
				return point_type < 0 ? e == point_type : e >= 0; // point_type < 0 means that it is one of the endpoint.
			}) != _boundary.cend();
	}

	// Searches for a boundary element with a given label.
	auto find_boundary_element(int label, bool need_last = false) noexcept {
		auto lambda_search = [label](const int& e) { return e == label; };
		if (need_last)
			return std::find_if(_boundary.rbegin(), _boundary.rend(), lambda_search).base() - 1;
		return std::find_if(_boundary.begin(), _boundary.end(), lambda_search);
	}

public:
	// Basic constructor.
	explicit domain(int domain_label = 0) noexcept : _domain_label(domain_label) {
	if (domain_label == 0)	// For the very first domain we also need to add the endpoints.
		_boundary = { start_point, end_point };
	}

	// Checks whether the boundary of this domain contains the end of the curve.
	bool contains_endpoint() const noexcept {
		return contains_special_point(end_point);
	}

	// Checks whether there exists any marked point on the boundary.
	bool contains_marked_points() const noexcept {
		return contains_special_point(0);	// Any non-negative parameter refers to a marked point.
	}

	// Checks whether the boundary of this domain contains both endpoints.
	bool can_be_closed() const noexcept {
		return contains_special_point(end_point) && contains_special_point(start_point);
	}

	// Returns the unordered_set of labels of all marked points on the boundary.
	// If a point occurs more than once, its negative label is also stored.
	std::unordered_set<int> all_marked_points() const noexcept{
		std::unordered_set <int> temp;
		for (const auto& boundary_elt : _boundary)
			if (boundary_elt > 0)
				temp.emplace(temp.contains(boundary_elt) ? -boundary_elt : boundary_elt);
		return temp;
	}

	// Adds new marked points before the endpoint. This function requires that an `end_point` is part of the boundary.
	void add_new_marked_points_before_end(int start_num, int num_of_points) noexcept {
		/*
		* Explanation:
		* Adding `k` new points changes the boundary structure as follows:
		*	X [end] Y -> X p_1 p_2 ... p_k [end] p_k p_{k-1} ... p_1 Y
		* where:
		*	- `X` and `Y` denotes unchanged parts of the boundary.
		*	- `[end]` denotes the endpoint.
		*	- `p_i` denotes newly added marked points.
		*
		* New elements are initially treated as endpoints so that, after initializing
		* the marked points, the endpoint position does not need to be explicitly calculated.
		*/
		auto it_end_point = std::find_if(_boundary.begin(), _boundary.end(),
			[](const int& e) { return e == end_point; });

		it_end_point = _boundary.insert(it_end_point, 2 * num_of_points, end_point);
		for (int i = 0, start_index = it_end_point - _boundary.begin(); i < num_of_points; ++i) 
			_boundary[start_index + i] = _boundary[start_index + 2 * num_of_points - i] = start_num + i;
	}

	// Modifies the domain by moving through the transition point.
	void go_throught_point(int transition_point, domain& new_domain, bool pass_from_left) noexcept {
		/*
		* Explanation:
		* After passing through the marked point, the boundary undergoes the following changes:
		* 1) The boundary of this domain transforms as follows:
		*		X t Y [end] Z  ->  X Z
		*	where:
		*		- `X`, `Y`, `Z` are unchanged parts of the boundary.
		*		- `[end]` is the endpoint.
		*		- `t` is the transition point.
		* 2) The new domain initially has an empty boundary, but after the transition, it takes the form Y,
		*	where `Y` is the same as in the first case.
		*/
		auto it_start_point = find_boundary_element(transition_point, !pass_from_left),
			it_end_point = find_boundary_element(end_point);

		if (it_start_point > it_end_point)
			std::swap(it_start_point, it_end_point);

		// Update the boundary of the new domain. Case 2) in explanation above.
		new_domain._boundary.insert(new_domain._boundary.end(),
			std::make_move_iterator(it_start_point + 1),
			std::make_move_iterator(it_end_point));
		
		// Update the boundary of this domain. Case 1) in explanation above.
		it_start_point = _boundary.erase(it_start_point, it_end_point + 1);
	}

	// Find marked point with a given label and change it into endpoint.
	void transform_point_into_endpoint(int marked_point_label, bool pass_from_left) noexcept {
		*find_boundary_element(marked_point_label, pass_from_left) = end_point;
	}

	int get_label() const noexcept {
		return _domain_label;
	}

	// Prints information about the domain, including its label and boundary elements.
	void print(std::ostream& out_stream = std::cout) const noexcept{
		out_stream << "Domain " << _domain_label << ":\n";
		for (const auto& boundary_elt : _boundary)
			if (boundary_elt > 0)
				out_stream << "\t point " << boundary_elt << "\n";
			else if (boundary_elt == end_point)
				out_stream << "\t end of the curve\n";
			else
				out_stream << "\t beginning of the curve\n";
	}
};