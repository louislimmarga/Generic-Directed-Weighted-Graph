#include "gdwg/graph.hpp"

#include <catch2/catch.hpp>
#include <sstream>

TEST_CASE("Default constructor") {
	auto g = gdwg::graph<std::string, int>{};
	CHECK(g.empty());
}

TEST_CASE("Constructor with Initializer list") {
	auto v = std::vector<std::string>{"hello", "hey"};
	auto g = gdwg::graph<std::string, double>{"hello", "hey"};
	CHECK(!g.empty());
	CHECK(g.nodes() == v);
}

TEST_CASE("Constructor with Iterator") {
	auto v = std::vector<std::string>{"hello", "hey"};
	auto g = gdwg::graph<std::string, double>{v.begin(), v.end()};
	CHECK(!g.empty());
	CHECK(g.nodes() == v);
}

TEST_CASE("Move Constructor") {
	auto v = std::vector<std::string>{"hello", "hey"};
	auto g1 = gdwg::graph<std::string, double>{"hello", "hey"};
	auto g2 = gdwg::graph<std::string, double>{std::move(g1)};
	CHECK(!g2.empty());
	CHECK(g1.empty());
	CHECK(g2.nodes() == v);
}

TEST_CASE("Copy Constructor") {
	auto v = std::vector<std::string>{"hello", "hey"};
	auto g1 = gdwg::graph<std::string, double>{"hello", "hey"};
	auto g2 = gdwg::graph<std::string, double>{g1};
	CHECK(!g1.empty());
	CHECK(g2.nodes() == g1.nodes());
}

TEST_CASE("Move Assignment") {
	auto v = std::vector<std::string>{"hello", "hey"};
	auto g1 = gdwg::graph<std::string, double>{"hello", "hey"};
	auto g2 = std::move(g1);
	CHECK(!g2.empty());
	CHECK(g1.empty());
	CHECK(g2.nodes() == v);
}

TEST_CASE("Copy Assignment") {
	auto v = std::vector<std::string>{"hello", "hey"};
	auto g1 = gdwg::graph<std::string, double>{"hello", "hey"};
	auto g2 = g1;
	CHECK(!g1.empty());
	CHECK(g2.nodes() == g1.nodes());
}

TEST_CASE("Insert node") {
	auto v = std::vector<std::string>{"are", "hello", "how", "you?"};
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");
	g.insert_node("are");
	g.insert_node("you?");

	CHECK(g.nodes() == v);
}

TEST_CASE("Insert edge") {
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");
	g.insert_node("are");
	g.insert_node("you?");

	// Single out edge
	auto v2 = std::vector<std::string>{"you?"};
	g.insert_edge("are", "you?", 3);
	CHECK(g.connections("are") == v2);

	// Multiple out edge with duplicates
	g.insert_edge("hello", "how", 5);
	g.insert_edge("hello", "are", 8);
	g.insert_edge("hello", "are", 2);
	g.insert_edge("hello", "are", 2);
	auto v3 = std::vector<std::string>{"are", "how"};
	CHECK(g.connections("hello") == v3);

	// Multiple out edge with 1 edge pointing to itself
	g.insert_edge("how", "you?", 1);
	g.insert_edge("how", "hello", 4);
	g.insert_edge("how", "how", 15);
	auto v4 = std::vector<std::string>{"hello", "how", "you?"};
	CHECK(g.connections("how") == v4);

	REQUIRE_THROWS_WITH(g.insert_edge("adsfasdf", "hello", 1),
	                    "Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node "
	                    "does not exist");
	REQUIRE_THROWS_WITH(g.insert_edge("hello", "asdfasdfa", 1),
	                    "Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node "
	                    "does not exist");
	REQUIRE_THROWS_WITH(g.insert_edge("adsfasdf", "hasdf", 1),
	                    "Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node "
	                    "does not exist");
}

TEST_CASE("Replace node") {
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");
	g.insert_node("are");
	g.insert_node("you?");

	g.insert_edge("are", "you?", 3);

	g.insert_edge("hello", "how", 5);
	g.insert_edge("hello", "are", 8);
	g.insert_edge("hello", "are", 2);
	g.insert_edge("hello", "are", 2);

	g.insert_edge("how", "you?", 1);
	g.insert_edge("how", "hello", 4);
	g.insert_edge("how", "how", 15);

	g.replace_node("how", "aaa");
	CHECK(g.is_node("aaa"));
	CHECK(!g.is_node("how"));

	auto v1 = std::vector<std::string>{"aaa", "are"};
	CHECK(g.connections("hello") == v1);

	auto v2 = std::vector<std::string>{"aaa", "hello", "you?"};
	CHECK(g.connections("aaa") == v2);

	REQUIRE_THROWS_WITH(g.replace_node("adsfasdf", "hasdf"),
	                    "Cannot call gdwg::graph<N, E>::replace_node on a node that doesn't exist");
}

TEST_CASE("Merge replace node") {
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");
	g.insert_node("are");
	g.insert_node("you?");

	g.insert_edge("are", "you?", 3);

	g.insert_edge("hello", "how", 5);
	g.insert_edge("hello", "are", 8);
	g.insert_edge("hello", "are", 2);
	g.insert_edge("hello", "are", 2);

	g.insert_edge("how", "you?", 1);
	g.insert_edge("how", "hello", 4);
	g.insert_edge("how", "how", 15);

	g.merge_replace_node("how", "are");
	CHECK(g.is_node("are"));
	CHECK(!g.is_node("how"));

	auto v1 = std::vector<std::string>{"are"};
	CHECK(g.connections("hello") == v1);

	auto v2 = std::vector<std::string>{"are", "hello", "you?"};
	CHECK(g.connections("are") == v2);

	REQUIRE_THROWS_WITH(g.merge_replace_node("adsfasdf", "hasdf"),
	                    "Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if "
	                    "they don't exist in the graph");
	REQUIRE_THROWS_WITH(g.merge_replace_node("hello", "hasdf"),
	                    "Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if "
	                    "they don't exist in the graph");
	REQUIRE_THROWS_WITH(g.merge_replace_node("adsfasdf", "hello"),
	                    "Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if "
	                    "they don't exist in the graph");
}

TEST_CASE("Erase node") {
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");

	g.insert_edge("hello", "how", 5);
	g.insert_edge("how", "hello", 4);

	g.erase_node("how");
	CHECK(!g.is_node("how"));

	g.erase_node("hello");
	CHECK(!g.is_node("hello"));

	CHECK(g.empty());
}

TEST_CASE("Erase edge") {
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");
	g.insert_node("are");
	g.insert_node("you?");

	g.insert_edge("are", "you?", 3);

	g.insert_edge("hello", "how", 5);
	g.insert_edge("hello", "are", 8);
	g.insert_edge("hello", "are", 2);
	g.insert_edge("hello", "are", 2);

	g.insert_edge("how", "you?", 1);
	g.insert_edge("how", "hello", 4);
	g.insert_edge("how", "how", 15);

	g.erase_edge("how", "how", 15);
	auto v1 = std::vector<std::string>{"hello", "you?"};
	CHECK(g.connections("how") == v1);

	g.erase_edge("hello", "are", 2);
	auto v2 = std::vector<std::string>{"are", "how"};
	CHECK(g.connections("hello") == v2);
	CHECK(g.find("hello", "are", 2) == g.end());

	REQUIRE_THROWS_WITH(g.erase_edge("hello", "hasdf", 2),
	                    "Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't "
	                    "exist in the graph");
	REQUIRE_THROWS_WITH(g.erase_edge("adsfasdf", "are", 2),
	                    "Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't "
	                    "exist in the graph");
	REQUIRE_THROWS_WITH(g.erase_edge("adsfasdf", "hasdf", 2),
	                    "Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't "
	                    "exist in the graph");
}

TEST_CASE("Erase edge with iterator") {
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");
	g.insert_node("are");
	g.insert_node("you?");

	g.insert_edge("are", "you?", 3);

	g.insert_edge("hello", "how", 5);
	g.insert_edge("hello", "are", 8);
	g.insert_edge("hello", "are", 2);
	g.insert_edge("hello", "are", 2);

	g.insert_edge("how", "you?", 1);
	g.insert_edge("how", "hello", 4);
	g.insert_edge("how", "how", 15);

	auto deleteEdge = g.find("how", "how", 15);
	auto iterAfterDeleteEdgeCheck = ++(g.find("how", "how", 15));
	auto iterAfterDeleteEdge = g.erase_edge(deleteEdge);
	auto v1 = std::vector<std::string>{"hello", "you?"};
	CHECK(g.connections("how") == v1);
	CHECK(iterAfterDeleteEdge == iterAfterDeleteEdgeCheck);

	auto deleteEdge2 = g.find("hello", "are", 2);
	auto iterAfterDeleteEdgeCheck2 = ++(g.find("hello", "are", 2));
	auto iterAfterDeleteEdge2 = g.erase_edge(deleteEdge2);
	auto v2 = std::vector<std::string>{"are", "how"};
	CHECK(g.connections("hello") == v2);
	CHECK(g.find("hello", "are", 2) == g.end());
	CHECK(iterAfterDeleteEdge2 == iterAfterDeleteEdgeCheck2);
}

TEST_CASE("Erase edge between 2 iterator") {
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");
	g.insert_node("are");
	g.insert_node("you?");

	g.insert_edge("are", "you?", 3);

	g.insert_edge("hello", "how", 5);
	g.insert_edge("hello", "are", 8);
	g.insert_edge("hello", "are", 2);
	g.insert_edge("hello", "are", 2);

	g.insert_edge("how", "you?", 1);
	g.insert_edge("how", "hello", 4);
	g.insert_edge("how", "how", 15);

	auto iterAfterDelete = g.erase_edge(g.begin(), g.end());
	CHECK(iterAfterDelete == g.end());
	auto v = std::vector<std::string>{};
	CHECK(g.connections("are") == v);
	CHECK(g.connections("you?") == v);
	CHECK(g.connections("hello") == v);
	CHECK(g.connections("how") == v);
}

TEST_CASE("Clear") {
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");
	g.insert_node("are");
	g.insert_node("you?");

	g.insert_edge("are", "you?", 3);

	g.insert_edge("hello", "how", 5);
	g.insert_edge("hello", "are", 8);
	g.insert_edge("hello", "are", 2);
	g.insert_edge("hello", "are", 2);

	g.insert_edge("how", "you?", 1);
	g.insert_edge("how", "hello", 4);
	g.insert_edge("how", "how", 15);

	g.clear();
	CHECK(g.empty());
}

TEST_CASE("Is node") {
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");

	CHECK(g.is_node("hello"));
	CHECK(g.is_node("how"));
	CHECK(!g.is_node("are"));
}

TEST_CASE("Empty") {
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");

	CHECK(!g.empty());

	auto g2 = gdwg::graph<std::string, double>{};

	CHECK(g2.empty());
}

TEST_CASE("Is connected") {
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");

	g.insert_edge("hello", "how", 2);

	CHECK(g.is_connected("hello", "how"));
	CHECK(!g.is_connected("how", "hello"));

	REQUIRE_THROWS_WITH(g.is_connected("hello", "hasdf"),
	                    "Cannot call gdwg::graph<N, E>::is_connected if src or dst node don't exist "
	                    "in the graph");
	REQUIRE_THROWS_WITH(g.is_connected("asdasd", "how"),
	                    "Cannot call gdwg::graph<N, E>::is_connected if src or dst node don't exist "
	                    "in the graph");
	REQUIRE_THROWS_WITH(g.is_connected("hasdo", "hasdf"),
	                    "Cannot call gdwg::graph<N, E>::is_connected if src or dst node don't exist "
	                    "in the graph");
}

TEST_CASE("Nodes") {
	auto v = std::vector<std::string>{"hello", "hey"};
	auto g1 = gdwg::graph<std::string, double>{"hello", "hey"};
	CHECK(g1.nodes() == v);
}

TEST_CASE("Weights") {
	auto v = std::vector<int>{1, 2, 3, 4, 5};
	auto g = gdwg::graph<std::string, int>{"hello", "hey"};
	g.insert_edge("hello", "hey", 5);
	g.insert_edge("hello", "hey", 4);
	g.insert_edge("hello", "hey", 3);
	g.insert_edge("hello", "hey", 2);
	g.insert_edge("hello", "hey", 1);

	CHECK(g.weights("hello", "hey") == v);

	REQUIRE_THROWS_WITH(g.weights("hello", "hasdf"),
	                    "Cannot call gdwg::graph<N, E>::weights if src or dst node don't exist in "
	                    "the graph");
	REQUIRE_THROWS_WITH(g.weights("hasd", "hey"),
	                    "Cannot call gdwg::graph<N, E>::weights if src or dst node don't exist in "
	                    "the graph");
	REQUIRE_THROWS_WITH(g.weights("hasd", "hasdf"),
	                    "Cannot call gdwg::graph<N, E>::weights if src or dst node don't exist in "
	                    "the graph");
}

TEST_CASE("Find") {
	auto g = gdwg::graph<std::string, int>{"hello", "hey"};
	g.insert_edge("hello", "hey", 5);

	CHECK(g.find("hello", "hey", 5) == g.begin());
	CHECK(g.find("adsf", "asdf", 2) == g.end());
}

TEST_CASE("Connections") {
	auto g = gdwg::graph<std::string, int>{"hello", "hey", "are"};
	g.insert_edge("hello", "hey", 5);
	g.insert_edge("hello", "are", 1);

	auto v = std::vector<std::string>{"are", "hey"};
	CHECK(g.connections("hello") == v);
	REQUIRE_THROWS_WITH(g.connections("hasdf"),
	                    "Cannot call gdwg::graph<N, E>::connections if src doesn't exist in the "
	                    "graph");
}

TEST_CASE("Iterator access") {
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");
	g.insert_node("are");
	g.insert_node("you?");

	g.insert_edge("are", "you?", 3);

	g.insert_edge("hello", "how", 5);
	g.insert_edge("hello", "are", 8);
	g.insert_edge("hello", "are", 2);
	g.insert_edge("hello", "are", 2);

	g.insert_edge("how", "you?", 1);
	g.insert_edge("how", "hello", 4);
	g.insert_edge("how", "how", 15);

	CHECK(g.find("are", "you?", 3) == g.begin());
}

TEST_CASE("Comparison") {
	auto g1 = gdwg::graph<std::string, double>{"hello", "are", "you"};
	auto g2 = gdwg::graph<std::string, double>{"hello", "are", "you"};
	auto g3 = gdwg::graph<std::string, double>{"hello"};

	CHECK(g1 == g2);
	CHECK(!(g1 == g3));
}

TEST_CASE("os") {
	using graph = gdwg::graph<int, int>;
	auto const v = std::vector<graph::value_type>{
	   {4, 1, -4},
	   {3, 2, 2},
	   {2, 4, 2},
	   {2, 1, 1},
	   {6, 2, 5},
	   {6, 3, 10},
	   {1, 5, -1},
	   {3, 6, -8},
	   {4, 5, 3},
	   {5, 2, 7},
	};

	auto g = graph{};
	for (const auto& [from, to, weight] : v) {
		g.insert_node(from);
		g.insert_node(to);
		g.insert_edge(from, to, weight);
	}

	g.insert_node(64);
	auto out = std::ostringstream{};
	out << g;
	auto const expected_output = std::string_view(R"(1 (
  5 | -1
)
2 (
  1 | 1
  4 | 2
)
3 (
  2 | 2
  6 | -8
)
4 (
  1 | -4
  5 | 3
)
5 (
  2 | 7
)
6 (
  2 | 5
  3 | 10
)
64 (
)
)");
	CHECK(out.str() == expected_output);
}

TEST_CASE("Custom iterator") {
	auto g = gdwg::graph<std::string, double>{};
	g.insert_node("hello");
	g.insert_node("how");
	g.insert_node("are");

	g.insert_edge("are", "hello", 3);

	g.insert_edge("hello", "how", 5);
	g.insert_edge("hello", "are", 8);
	g.insert_edge("hello", "are", 2);

	g.insert_edge("how", "how", 15);

	auto it = g.begin();
	auto firstEdge = it++;
	CHECK(firstEdge == g.begin());

	auto secondEdge = it;
	CHECK(secondEdge == ++g.begin());

	auto secondEdgewithDecrement = it--;
	CHECK(secondEdgewithDecrement == secondEdge);

	auto secondEdgeWithPrefix = ++it;
	CHECK(secondEdgeWithPrefix == secondEdge);

	auto secondEdgewithDecrementPrefix = --it;
	CHECK(secondEdgewithDecrementPrefix == firstEdge);

	CHECK(firstEdge == g.begin());
	CHECK(!(firstEdge == g.end()));
	CHECK(firstEdge != g.end());
}