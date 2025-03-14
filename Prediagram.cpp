#include "Prediagram.h"
#include "Domain.h"

#include <algorithm>
#include <numeric>
#include <queue>

// Main constructor. Creates `pre_diagram` with `num` marked points.
pre_diagram::pre_diagram(size_t num) noexcept : _number_of_points(0), _closed(false) {
	_atlas[0] = domain(0);
	add_new_marked_points(num);
}

// Returns a reference to the `domain` containing the endpoint (mutable).
domain& pre_diagram::find_domain_with_endpoint() noexcept {
	return std::find_if(_atlas.begin(), _atlas.end(),
		[](const auto& dom) { return dom.second.contains_endpoint(); }
	)->second;
}

// Returns a const reference to the `domain` containing the endpoint (immutable).
domain const& pre_diagram::find_domain_with_endpoint() const noexcept {
	return std::find_if(_atlas.cbegin(), _atlas.cend(),
		[](const auto& dom) { return dom.second.contains_endpoint(); }
	)->second;
}

// Adds `num_of_new_points` new marked points to the diagram.
void pre_diagram::add_new_marked_points(int num_of_new_points) noexcept {
	if (num_of_new_points < 1) 
		return;

	// Reserve space and add new points to the Gauss code.
	_gauss_code.reserve(_gauss_code.size() + num_of_new_points); 
	std::generate_n(std::back_inserter(_gauss_code), num_of_new_points,
		[this]() mutable { return ++this->_number_of_points; });

	// Find the domain with the endpoint and add new points there.
	domain& domain_with_end = find_domain_with_endpoint();
	int domain_with_end_label = domain_with_end.get_label();
	domain_with_end.add_new_marked_points_before_end(_number_of_points - num_of_new_points + 1, num_of_new_points);

	// Register new marked points in `_marked_points_as_edges`.
	for (int i = _number_of_points - num_of_new_points; i < _number_of_points; ++i)
		_marked_points_as_edges[i+1] = std::make_pair(domain_with_end_label, domain_with_end_label);
}

// Extends the diagram by passing through an existing marked point (`transition point`).
// The function assumes that this is possible. If `transition_point` is negative, the passage is performed from the right.
void pre_diagram::go_through_point(int transition_point) noexcept {
	// Add the transition point to Gauss code.
	_gauss_code.push_back(std::abs(transition_point));

	// Create new domain that appears after passing through the transition point.
	int new_domain_label = _atlas.size();
	_atlas[new_domain_label] = domain(new_domain_label);

	// Update the boundary of the domain that currently contains the endpoint.
	int domain_with_end_label = find_domain_with_endpoint().get_label();
	find_domain_with_endpoint().go_throught_point(std::abs(transition_point), _atlas[new_domain_label], transition_point > 0);

	// Update the second domain on the boundary of which the transition point lay.
	int other_domain_label = _marked_points_as_edges[std::abs(transition_point)].first + _marked_points_as_edges[std::abs(transition_point)].second - domain_with_end_label;
	_atlas[other_domain_label].transform_point_into_endpoint(std::abs(transition_point), transition_point > 0);

	// Transition point is no more a marked point.
	_marked_points_as_edges.erase(std::abs(transition_point));

	// Update connections for marked points now belonging to the new domain.
	for (int elt : _atlas[new_domain_label].all_marked_points()) {
		//In pair `_marked_points_as_edges[elt]` at least one element is `domain_with_end_label`. Change it to `new_domain_label`.
		auto& domain_pair = _marked_points_as_edges[elt];
		(domain_pair.first == domain_with_end_label ? domain_pair.first : domain_pair.second) = new_domain_label;
	}
}

// Returns labels of all marked points currently available for continuation.
std::unordered_set<int> pre_diagram::available_points() const noexcept {
	return find_domain_with_endpoint().all_marked_points();
}

// Checks if the diagram has a unique irreducible continuation. 
// It happens that there is only one available mark point. Recall that if a marked point appears twice on the boundary of
// some domain it's negative value is also stored.
bool pre_diagram::only_one_way() const noexcept {
	auto points = available_points();
	return (points.size() == 1) || ((points.size() == 2) && (std::accumulate(points.cbegin(), points.cend(), 0)));
}

// Checks if the diagram is irreducible.
// It means that in Gauss code there exist a (proper) substring of length 2*n that contains n distinct elements.
bool pre_diagram::check_irreducibility() const noexcept {
	// Sliding window algorythm is used.
	for (size_t i = 0; i < _gauss_code.size(); ++i) {
		std::unordered_map<int, int> freq;
		int unique_labels = 0;
		for (size_t j = i; j < _gauss_code.size(); ++j) {
			if (++freq[_gauss_code[j]] == 1) 
				++unique_labels;
			if (int len = j - i; len % 2 == 1)
				if (2 * unique_labels == len + 1)
					return false;
		}
	}
	return true;
}

// Attempts to close the diagram by connecting its endpoints.
// It is possible whaen all marked points where used, and the boundary of the domain with endpoint also contains starting point.
bool pre_diagram::close_up() noexcept {
	return _closed = (_number_of_points == _gauss_code.size() / 2 && find_domain_with_endpoint().can_be_closed());
}

// Checks whether the diagram can be correctly extended to a closed curve.
// Specifically, it verifies two necessary conditions:
//		1) all domains with non-empty boundary can be connected through marked points,
//		2) there are no loops with an odd number of marked (or double) points.
bool pre_diagram::check_correctness() const noexcept {
	// Check whether a loop with an odd number of marked or double points exists.
	for (size_t i = 0; i < _gauss_code.size(); ++i) {
		for (size_t k = 2; i + k < _gauss_code.size(); k += 2) {
			if (_gauss_code[i] == _gauss_code[i + k])
				return false;
		}
	}
	// Check connectivity of domains via BFS (domains are vertices, edges are marked points).
	if (!_marked_points_as_edges.empty()) {
		std::unordered_set<int> vertices;					// Labels of domains, that contain marked points on its boundary.
		std::unordered_map<int, std::vector<int>> graph;	// Present the graph of domains as an adjacency list.

		// Add edges to graph.
		for (const auto& [_, edge] : _marked_points_as_edges) {
			auto [v1, v2] = edge;
			vertices.insert({ v1, v2 });
			graph[v1].push_back(v2);
			graph[v2].push_back(v1);
		}

		// Create sets for visited and not visited vertices.
		std::unordered_set<int> visited;
		std::queue<int> not_visited;
		not_visited.push(*vertices.begin());
		visited.insert(*vertices.begin());

		// Starts the BFS.
		while (!not_visited.empty()) {
			int current = not_visited.front();
			not_visited.pop();
			for (int neighbour : graph[current]) {
				if (!visited.contains(neighbour)) {
					visited.insert(neighbour);
					not_visited.push(neighbour);
				}
			}
		}

		// If not all vertices were visited, the diagram is not correct.
		if (visited.size() != vertices.size())
			return false;
	}
	return true;
}

// Checks whether a simple arc with the given number of marked or double points exists in the provided Gauss code.
// Uses a sliding window algorithm.
bool pre_diagram::has_simple_arc(const std::vector<int>& gauss_code, size_t length) const noexcept {
	if (gauss_code.size() < length && !_closed)	// Remark. In theory it is more efficient to consider not `gauss_code.size()`,
		return false;							// but the number of unique elements in it. In practice it is insignificant.

	std::unordered_map<int, int> freq;
	for (size_t i = 0; i < length; ++i)
		freq[gauss_code[i]]++;

	if (freq.size() == length)
		return true;

	for (size_t i = length; i < gauss_code.size(); ++i) {
		int out = gauss_code[i - length];
		if (--freq[out] == 0)
			freq.erase(out);
		freq[gauss_code[i]]++;
		if (freq.size() == length)
			return true;
	}
	if (_closed) {	// If the curve is closed, treat the Gauss code as a circular vector.
		for (size_t i = gauss_code.size(); i < gauss_code.size() + length; ++i) {	// Need `length` extra checks.
			int out = gauss_code[i - length];
			if (--freq[out] == 0)
				freq.erase(out);
			freq[gauss_code[i % gauss_code.size()]]++;
			if (freq.size() == length)
				return true;
		}
	}
	return false;
}

// Checks whether a simple arc with the given number of marked or double points exists in the Gauss code.
bool pre_diagram::has_simple_arc(size_t length) const noexcept {
	if (length > _number_of_points && !_closed)
		return false;
	if (_closed)
		length = std::min(_number_of_points, length);
	return has_simple_arc(_gauss_code, length);
}

// Performs a flype operation in the provided Gauss code at position `index`.
// It is assumed that at position `index`, a flype operation is indeed possible.
std::vector<int> pre_diagram::make_flype_in(const std::vector<int>& gauss_code, size_t index) const noexcept {
	/*
	* Explanation:
	* Only the simplest from of the flype is considered (which turns out to be sufficient). Specifically, the 
	* transformation follows one of these patterns:
	*	1) S1 a b c a S2 (b||c) S3 (c||b) S4  ->  S1 b c S2 a (b||c) S3 (c||b) a S4,
	*	2) S1 a b c a S2 (b||c) S3  ->  S1 b c S2 a (b||c) S3,
	*	3) S1 a b c a S2  ->  S1 b c S2,
	* where
	*	- `a` is the double point where flype is applied.
	*	- `b` and `c` are marked (or double) points.
	*	- `(b||c)` and `(c||b)` mean either `b` or `c` appears in that position.
	*	- `S1`, `S2`, `S3`, `S4` are arbitrary substrings of the Gauss code. 
	* 
	* The pattern of flype applied depends on the nature of `b` and `c`:
	* - Pattern 1: Used if both `b` and `c` are double points.
	* - Pattern 2: Used if one of `b` or `c` is a marked point and the other is a double point.
	* - Pattern 3: Used if both `b` and `c` are marked points.
	*/
	size_t n = gauss_code.size();
	int a = gauss_code[index],
		b = gauss_code[(index + 1) % n],
		c = gauss_code[(index + 2) % n];

	int extra_counts = 0;						// Tracks occurrences of `b` and `c` in the Gauss code.
	std::vector<int> gauss_code_after_flype;	// Gauss code after flype.
	gauss_code_after_flype.reserve(n);

	if (!_closed) // The approach is slightly different depending on whether a closed curve is being considered or not.
		for (size_t k = 0; k < n; ++k) {
			if (k == index) {	// Replace the four-element pattern `a b c a` with `b c`.
				gauss_code_after_flype.push_back(b);
				gauss_code_after_flype.push_back(c);
				k = index + 3;	// Skip indices of paatern `a b c a`.
				continue;
			}
			if (gauss_code[k] == b || gauss_code[k] == c) {
				++extra_counts;
				if (extra_counts == 1) {
					gauss_code_after_flype.push_back(a);
					gauss_code_after_flype.push_back(gauss_code[k]);
				}
				else {
					gauss_code_after_flype.push_back(gauss_code[k]);
					gauss_code_after_flype.push_back(a);
				}
			}
			else 
				gauss_code_after_flype.push_back(gauss_code[k]);
		}
	else {
		gauss_code_after_flype.push_back(b);
		gauss_code_after_flype.push_back(c);
		for (size_t k = 4; k < n; ++k) {
			if (gauss_code[(index + k) % n] == b || gauss_code[(index + k) % n] == c) {
				++extra_counts;
				if (extra_counts == 1) {
					gauss_code_after_flype.push_back(a);
					gauss_code_after_flype.push_back(gauss_code[(index + k) % n]);
				}
				else {
					gauss_code_after_flype.push_back(gauss_code[(index + k) % n]);
					gauss_code_after_flype.push_back(a);
				}
			}
			else 
				gauss_code_after_flype.push_back(gauss_code[(index + k) % n]);
		}
	}
	return gauss_code_after_flype;
}

// Checks for a simple arc in the Gauss code, allowing flype transformations. It does not consider all possible ways
// of applying flype. This function runs through the Gauss code skip several positions and then  applies a flype
// whenever possible. This approach is sufficient for the proof.
bool pre_diagram::has_simple_arc_with_flypes(size_t length) const noexcept {
	if (length > _number_of_points && !_closed)
		return false;
	if (_closed)
		length = std::min(_number_of_points, length);
	if (has_simple_arc(length))
		return true;
	for (size_t skip_first_positions = 0; skip_first_positions < _number_of_points; ++skip_first_positions) {	// This could be optimized.
		auto temp_gauss_code = _gauss_code;
		size_t skiped = 0;
		for (size_t i = 0; i < (_closed ? temp_gauss_code.size() : temp_gauss_code.size() - 3); ++i) {
			if (temp_gauss_code[i] == temp_gauss_code[(i + 3) % temp_gauss_code.size()] && ++skiped > skip_first_positions) {
				temp_gauss_code = make_flype_in(temp_gauss_code, i);
				if (has_simple_arc(temp_gauss_code, length))
					return true;
			}
		}
	}
	return false;
}

// Checks equivalence to a given Gauss code after relabeling.
bool pre_diagram::is_equivalent(const std::vector<int>& gauss_code) const noexcept {
	/*
	* Explanation:
	* Two Gauss code `G1` and `G2` of length `n` are said to be equivalent, if there exists a bijection
	* f: [1:n] -> [1:n] and an integer `k`, such that
	*		G1[i] = f(G2[(i+k) mod n],	for each i in {0, ..., n-1}.
	*
	* The function iterates through all possible shifts `k` and, for each one, attempts to construct a valid bijection `f`.
	*/
	if (_gauss_code.size() != gauss_code.size())
		return false;

	const size_t m = _gauss_code.size(),
		n = m / 2;

	for (size_t k = 0; k < m; ++k) {
		std::vector<int> f(n + 1, 0);			// Bijection. f[i] == 0 means means `f(i)` is not yet assigned.
		std::vector<bool> used(n + 1, false);	// Tracks whether `x` has already been assigned.
		bool valid = true;
		for (size_t i = 0; i < m; ++i) {
			int y = _gauss_code[(i + k) % m],
				x = gauss_code[i];
			if (f[y] == 0) {		// If f[y] is not yet defined, define it.
				if (used[x]) {		// Ensure that x has not been used for another y.
					valid = false;
					break;
				}
				f[y] = x;
				used[x] = true;
			}
			else if (f[y] != x) {	// If `f(y)` is already defined, it must equal `x`.
				valid = false;
				break;
			}
		}
		if (valid)
			return true;
	}
	return false;
}

// Returns the Gauss code of the curve.
const std::vector<int>& pre_diagram::get_gauss_code() const noexcept {
	return _gauss_code;
}

// Prints information about the diagram, including its Gauss code and all technical details (if required).
void pre_diagram::print(std::ostream& out_stream, bool only_gc) const noexcept{
	for (const auto& e : _gauss_code)
		out_stream << "[" << e << "] ";
	out_stream << std::endl;
	out_stream << "----------------------" << std::endl;
	if (!only_gc) {
		out_stream << "List of edges (" << _marked_points_as_edges.size() << " in total):" << std::endl;
		for (const auto& edge : _marked_points_as_edges)
			out_stream << "\t" << edge.first << ": " << edge.second.first << " -> " << edge.second.second << std::endl;
		out_stream << "----------------------" << std::endl;
		out_stream << "List of domains (" << _atlas.size() << " in total):" << std::endl;
		for (const auto& dom : _atlas)
			dom.second.print();
		out_stream << "----------------------" << std::endl;
	}
}