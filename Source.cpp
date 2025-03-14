#include <algorithm>
#include <fstream>

#include "Prediagram.h"

using action_type = void (pre_diagram::*)(int) noexcept;			// Refers to either `add_new_marked_points` or `go_through_point`.
using check_type = bool (pre_diagram::*)(size_t) const noexcept;	// Refers to either `has_simple_arc` or `has_simple_arc_with_flypes`.

// Performs a single step of the brute-force algorithm. The algorithm is recursive.
void brute_force_step(pre_diagram diagram, action_type action, int action_parameter, check_type check, int check_parameter, std::vector < std::vector<int>>& exceptional_curves) {
	// Perform the given action. Note that `diagram` is modified here. 
	(diagram.*action)(action_parameter);
	// If `diagram` cannot be extended to a closed curve or satisfies the `check` condition, stop here.
	if (!diagram.check_correctness() || (diagram.*check)(check_parameter))
		return;
	// If `diagram` is a closed curve that does *not* satisfy the `check` condition, add it to the set of exceptional curves.
	if (diagram.close_up() && !(diagram.*check)(check_parameter)) {
		for (const auto& gauss_code : exceptional_curves)	// Check if this curve is already in the set of exceptional curves.
			if (diagram.is_equivalent(gauss_code))
				return;
		exceptional_curves.push_back(diagram.get_gauss_code());
	}
	// If `diagram` is irreducible, it can be extended by either adding new marked points or passing through an existing marked point.
	else if (diagram.check_irreducibility()){
		// New marked points are added only if they were not just added and if adding them does not create a reducible diagram.
		if (action != &pre_diagram::add_new_marked_points && !diagram.only_one_way())
			for (int new_points_num = 1; new_points_num < check_parameter; ++new_points_num)
				brute_force_step(diagram, &pre_diagram::add_new_marked_points, new_points_num, check, check_parameter, exceptional_curves);
		// Passing through a marked point is always an acceptable continuation.
		for (int pt : diagram.available_points())
			brute_force_step(diagram, &pre_diagram::go_through_point, pt, check, check_parameter, exceptional_curves);
	}
}

// Main brute-force function. Performs all calculations and stores the exceptional curves.
void brute_force(int magic_number, check_type check, std::ostream& out_stream = std::cout) {
	std::vector < std::vector<int>> exceptional_curves;	// Container for storing the exceptional curves.

	// Start brute force from a simple curve with several marked points.
	for (size_t initial_num_of_points = 3; initial_num_of_points < magic_number; ++initial_num_of_points)
		brute_force_step(pre_diagram(0), &pre_diagram::add_new_marked_points, initial_num_of_points, check, magic_number, exceptional_curves);

	// Output of the results of computations.
	out_stream << "There were found " << exceptional_curves.size() << " such curves.\n";
	if (!exceptional_curves.empty()) {
		// Sort exceptional curves by size for a cleaner output.
		std::sort(exceptional_curves.begin(), exceptional_curves.end(),
			[](const std::vector<int>& gc_1, const std::vector<int>& gc_2) {
				return gc_1.size() < gc_2.size();
			});

		out_stream << "Here are their Gauss codes:\n";
		for (auto& curve_gauss_code : exceptional_curves) {
			out_stream << "\t";
			for (int pt : curve_gauss_code)
				out_stream << "[" << pt << "] ";
			out_stream << std::endl;
		}
	}
}

int main() {
	std::ofstream out_stream("exceptional_curves.txt");

	// Find all curves with no more than 6 double points, that do not contain a simple arc with 6 double points.
	out_stream << "Finding all irreducible curves with more than "
		<< pre_diagram::simple_arc_size_no_flypes
		<< " crossings that do not contain a simple arc with "
		<< pre_diagram::simple_arc_size_no_flypes
		<< " double points...\n";
	brute_force(pre_diagram::simple_arc_size_no_flypes, &pre_diagram::has_simple_arc, out_stream);

	// Find all curves with no more than 6 double points, that do not contain a simple arc with 6 double points if flypes are allowed.
	out_stream << "\nFinding all irreducible curves with more than "
		<< pre_diagram::simple_arc_size_with_flypes
		<< " crossings, and that do not have simple arc with "
		<< pre_diagram::simple_arc_size_with_flypes 
		<< " double points if flypes are allowed...\n";
	brute_force(pre_diagram::simple_arc_size_with_flypes, &pre_diagram::has_simple_arc_with_flypes, out_stream);

	out_stream.close();
	return 0;
}