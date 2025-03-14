#pragma once
#include<unordered_map>

#include "Domain.h"

/*
* This file defines the `pre_diagram` class, representing an immersed oriented curve (open or closed) on the plane in general position,
* having a finite number of marked points (that will later become double points of the closed curve). The class provides a structured
* way to incrementally build and validate diagrams.
* Internally, a `pre_diagram` is represented via a Gauss code, which is a vector of positive integers, where each element corresponds
* either to a crossing, or a marked point. If `pre_diagram` corresponds to a closed curve, its Gauss code is considered as a circular vector.
* Technically, information about how the curve divides the plane is also stored (see `Domain.h` for details on the domain class).
* 
* For constructing a closed immersed curve consistently, the following key operations are provided:
*	- add_new_marked_points (n):
*		Adds `n` new marked points in a small neighborhood of the endpoint of the curve. In Gauss code terms, this appends `n` new elements.
*	- go_through_point (pt):
*		The curve is extended with a simple arc that intersects the curve at a single marked point labeled `pt`. In Gauss code terms, this
*		adds `pt` at the end of Gauss code.
*	- close_up ():
*		Closes the curve by connecting its endpoints with a simple arc whose interior does not intersect the existing diagram. 
*		Returns `false` if closure isn't possible or if there are marked points left unused.
* 
* Additional utility functions verify diagram properties:
*	- check_correctness(): Checks whether the diagram can be extended to a closed curve without marked points.
*	- only_one_way(): Checks whether the diagram has a unique irreducible continuation.
*	- check_irreducibility(): Checks whether the `pre_diagram` is irreducible.
*	- has_simple_arc() and has_simple_arc_with_flypes(): Check for specific structural patterns (simple arcs) in the Gauss code.
*	- is_equivalent(): Checks whether, after a suitable renumbering, a given Gauss code corresponds to this `pre_diagram`.
* 
* Two constants (`simple_arc_size_no_flypes` and `simple_arc_size_with_flypes`) are also stored as static members of the class. They
* correspond to the property under investigation -- the existence of a simple arc with a given number of double points. Specifically,
* the constant `simple_arc_size_no_flypes` is used when irreducible curves are considered without using flypes, and the constant
* `simple_arc_size_with_flypes` is used when it is allowed to apply flypes to the curves.
*/

class pre_diagram {
public:
	// Basic meaningful constants (see class description above).
	constexpr static int simple_arc_size_no_flypes		= 6;
	constexpr static int simple_arc_size_with_flypes	= 8;

public:
	explicit pre_diagram(size_t num) noexcept;	// Creates `pre_diagram` with `num` marked points.

	// Core functions to incrementally build and modify the diagram (see class description above).
	void add_new_marked_points(int num) noexcept;	// Adds `num` new marked points near the endpoint.
	void go_through_point(int pt) noexcept;			// Passes through an available marked point labeled `pt`.
	bool close_up() noexcept;						// Closes the diagram if possible, returns `false` otherwise.

	std::unordered_set<int> available_points() const noexcept;	// Returns labels of points available to pass through.

	// Functions verifying various diagram properties (see class description above).
	bool check_correctness() const noexcept;								// Checks if the diagram can be extended to a correct closed curve.
	bool only_one_way() const noexcept;										// Checks if the diagram has a unique irreducible continuation.
	bool check_irreducibility() const noexcept;								// Checks if the diagram is irreducible.
	bool is_equivalent(const std::vector<int>& gauss_code) const noexcept;	// Checks equivalence to a given Gauss code after relabeling.

	// Check for the presence of a simple arc of a specified size.
	bool has_simple_arc(size_t length = simple_arc_size_no_flypes) const noexcept;
	bool has_simple_arc_with_flypes(size_t length = simple_arc_size_with_flypes) const noexcept;

	const std::vector<int>& get_gauss_code() const noexcept;	// Returns the Gauss code of the curve.
	// Outputs the diagram details. If `only_gc = true` prints only the Gauss code.
	void print(std::ostream& out_stream = std::cout, bool only_gc = false) const noexcept;

private:
	domain& find_domain_with_endpoint() noexcept;				// Returns the `domain` containing the endpoint (mutable).
	domain const& find_domain_with_endpoint() const noexcept;	// Returns the `domain` containing the endpoint (immutable).

	// Performs a flype operation in the provided Gauss code at position `index`.
	std::vector<int> make_flype_in(const std::vector<int>& gauss_code, size_t index) const noexcept;

	// Checks for a simple arc of specified length in the provided Gauss code.
	bool has_simple_arc(const std::vector<int>& gauss_code, size_t length) const noexcept;

private:
	size_t _number_of_points;	// Number of different special point (either marked or double).
	bool _closed;				// If given `pre_diagram` is closed.

	std::vector <int> _gauss_code;				// Vector representing the Gauss code.
	std::unordered_map <int, domain> _atlas;	// Mapping of domain labels to `domain` objects.

	// Mapping of marked point lables to labels of domains on the border of which it lies.
	std::unordered_map <int, std::pair <int, int>> _marked_points_as_edges;
};