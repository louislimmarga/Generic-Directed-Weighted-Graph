#ifndef GDWG_GRAPH_HPP
#define GDWG_GRAPH_HPP

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <utility>
#include <vector>
// TODO: Make this graph generic
//       ... this won't just compile
//       straight away
namespace gdwg {
	template<typename N, typename E>
	class graph {
	public:
		struct edge;
		struct node;
		struct value_type {
			N from;
			N to;
			E weight;
		};

		class iterator {
			using edge_t = typename std::set<edge>::const_iterator;

		public:
			using value_type = graph<N, E>::value_type;
			using reference = value_type;
			using pointer = void;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::bidirectional_iterator_tag;

			// Iterator constructor
			iterator() = default;
			explicit iterator(N const& src, N const& dst, E const& weight, std::set<edge> edges) {
				struct edge findEdge;
				findEdge.from = std::make_shared<N>(src);
				findEdge.to = std::make_shared<N>(dst);
				findEdge.weight = weight;

				auto it = edges.find(findEdge);
				if (it != edges.end()) {
					data_ = it;
					std::cout << *(data_->from) << " " << *(data_->to) << " " << data_->weight << "\n";
				}
				else {
					data_ = edges.end();
					std::cout << "AAA\n";
				}
			}

			// Iterator source
			auto operator*() -> reference {
				struct graph<N, E>::value_type ret;
				ret.from = *(data_->from);
				ret.to = *(data_->to);
				ret.weight = data_->weight;

				return ret;
			}

			// Iterator traversal
			auto operator++() -> iterator& {
				data_++;
				return *this;
			}
			auto operator++(int) -> iterator {
				auto old = *this;
				++*(this);
				return old;
			}

			auto operator--() -> iterator& {
				data_--;
				return *this;
			}
			auto operator--(int) -> iterator {
				auto old = *this;
				--*(this);
				return old;
			}

			// Iterator comparison
			auto operator==(iterator const& other) const -> bool {
				return data_ == other.data_;
			}

		private:
			edge_t data_;

			iterator(edge_t begin)
			: data_{begin} {}

			friend class graph;
		};

		[[nodiscard]] auto begin() const -> iterator {
			return iterator(edges.begin());
		}

		[[nodiscard]] auto end() const -> iterator {
			return iterator(edges.end());
		}

		struct edge {
			std::shared_ptr<N> from;
			std::shared_ptr<N> to;
			E weight;

			bool operator<(const edge& rhs) const {
				if (*from != *(rhs.from)) {
					return *from < *(rhs.from);
				}
				else if (*to != *(rhs.to)) {
					return *to < *(rhs.to);
				}
				else {
					return weight < rhs.weight;
				}
			}

			bool operator==(const edge& rhs) const {
				return *from == *(rhs.from) && *to == *(rhs.to) && weight == rhs.weight;
			}
		};

		struct node {
			std::shared_ptr<N> value;

			bool operator<(const node& rhs) const {
				return *value < *(rhs.value);
			}

			bool operator==(const node& rhs) const {
				return *value == *(rhs.value);
			}
		};

		graph() = default;

		graph(std::initializer_list<N> il)
		: graph(il.begin(), il.end()) {}

		template<typename InputIt>
		graph(InputIt first, InputIt last) {
			for (auto it = first; it != last; it++) {
				insert_node(*it);
			}
		}

		graph(graph&& other) noexcept {
			this->nodes_.clear();
			this->edges.clear();
			std::swap(this->nodes_, other.nodes_);
			std::swap(this->edges, other.edges);
		}

		graph(graph& other) noexcept {
			this->nodes_.clear();
			this->edges.clear();
			for (auto const& it : other.nodes_) {
				insert_node(*(it.value));
			}
			for (auto const& it : other.edges) {
				insert_edge(*(it.from), *(it.to), it.weight);
			}
		}

		auto operator=(graph&& other) noexcept -> graph& {
			this->nodes_.clear();
			this->edges.clear();
			std::swap(this->nodes_, other.nodes_);
			std::swap(this->edges, other.edges);
			return *this;
		}

		auto operator=(graph const& other) -> graph& {
			this->nodes_.clear();
			this->edges.clear();
			for (auto const& it : other.nodes_) {
				insert_node(*(it.value));
			}
			for (auto const& it : other.edges) {
				insert_edge(*(it.from), *(it.to), it.weight);
			}
			return *this;
		}

		~graph() noexcept = default;

		auto insert_node(N const& value) -> bool {
			struct node newNode;
			newNode.value = std::make_shared<N>(value);
			return nodes_.insert(newNode).second;
		}

		auto insert_edge(N const& src, N const& dst, E const& weight) -> bool {
			if (!is_node(src) || !is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::insert_edge when either src "
				                         "or dst node does not exist");
			}
			struct node findNode;
			findNode.value = std::make_shared<N>(src);
			auto srcNode = nodes_.find(findNode);

			struct node findNode2;
			findNode2.value = std::make_shared<N>(dst);
			auto dstNode = nodes_.find(findNode2);

			struct edge newEdge;
			newEdge.from = srcNode->value;
			newEdge.to = dstNode->value;
			newEdge.weight = weight;

			bool edgeUnique = edges.insert(newEdge).second;
			if (!edgeUnique) {
				return false;
			}

			return true;
		}

		auto replace_node(N const& old_data, N const& new_data) -> bool {
			if (!is_node(old_data)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::replace_node on a node that "
				                         "doesn't exist");
			}
			if (is_node(new_data)) {
				return false;
			}

			insert_node(new_data);

			merge_replace_node(old_data, new_data);
			return true;
		}

		auto merge_replace_node(N const& old_data, N const& new_data) -> void {
			if (!is_node(old_data) || !is_node(new_data)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::merge_replace_node on old or "
				                         "new data if they don't exist in the graph");
			}

			struct node findNode;
			findNode.value = std::make_shared<N>(old_data);
			auto oldNode = nodes_.find(findNode);

			struct node findNode2;
			findNode2.value = std::make_shared<N>(new_data);
			auto newNode = nodes_.find(findNode2);

			std::set<std::tuple<N, E>> froms;

			for (auto it = edges.begin(); it != edges.end();) {
				if (it->from == newNode->value || it->to == newNode->value) {
					it = edges.erase(it);
				}
				else {
					++it;
				}
			}

			for (auto it = edges.begin(); it != edges.end();) {
				if (it->from == oldNode->value) {
					froms.insert(make_tuple(*(it->to), it->weight));
					it = edges.erase(it);
				}
				else {
					++it;
				}
			}

			for (auto const& it : froms) {
				insert_edge(*(newNode->value), get<0>(it), get<1>(it));
			}

			std::set<std::tuple<N, E>> tos;

			for (auto it = edges.begin(); it != edges.end();) {
				if (it->to == oldNode->value) {
					tos.insert(make_tuple(*(it->from), it->weight));
					it = edges.erase(it);
				}
				else {
					++it;
				}
			}

			for (auto const& it : tos) {
				insert_edge(get<0>(it), *(newNode->value), get<1>(it));
			}

			nodes_.erase(oldNode);
		}

		auto erase_node(N const& value) -> bool {
			if (!is_node(value)) {
				return false;
			}

			struct node findNode;
			findNode.value = std::make_shared<N>(value);
			auto oldNode = nodes_.find(findNode);

			for (auto it = edges.begin(); it != edges.end();) {
				if (it->from == oldNode->value || it->to == oldNode->value) {
					it = edges.erase(it);
				}
				else {
					++it;
				}
			}

			nodes_.erase(oldNode);
			return true;
		}

		auto erase_edge(N const& src, N const& dst, E const& weight) -> bool {
			if (!is_node(src) || !is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::erase_edge on src or dst if "
				                         "they don't exist in the graph");
			}
			struct edge findEdge;
			findEdge.from = std::make_shared<N>(src);
			findEdge.to = std::make_shared<N>(dst);
			findEdge.weight = weight;

			auto it = edges.find(findEdge);
			if (it != edges.end()) {
				edges.erase(it);
				return true;
			}
			return false;
		}

		auto erase_edge(iterator i) -> iterator {
			auto ret = i++;
			struct edge findEdge;
			auto [from, to, weight] = *ret;
			findEdge.from = std::make_shared<N>(from);
			findEdge.to = std::make_shared<N>(to);
			findEdge.weight = weight;

			auto it = edges.find(findEdge);
			edges.erase(it);
			return i;
		}

		auto erase_edge(iterator i, iterator s) -> iterator {
			auto ret = i;
			while (ret != s) {
				auto del = ret;
				ret = erase_edge(del);
			}
			return ret;
		}

		auto clear() noexcept -> void {
			nodes_.clear();
			edges.clear();
		}

		[[nodiscard]] auto is_node(N const& value) -> bool {
			struct node findNode;
			findNode.value = std::make_shared<N>(value);
			auto it = nodes_.find(findNode);
			if (it != nodes_.end()) {
				return true;
			}
			return false;
		}

		[[nodiscard]] auto empty() -> bool {
			return nodes_.empty() && edges.empty();
		}

		[[nodiscard]] auto is_connected(N const& src, N const& dst) -> bool {
			if (!is_node(src) || !is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::is_connected if src or dst "
				                         "node don't exist in the graph");
			}
			auto search_src = find_if(edges.begin(), edges.end(), [&src, &dst](auto const& p) {
				return *p.from == src && *p.to == dst;
			});
			if (search_src != edges.end()) {
				return true;
			}
			return false;
		}

		[[nodiscard]] auto nodes() -> std::vector<N> {
			std::vector<N> ret;
			for (auto it = nodes_.begin(); it != nodes_.end(); it++) {
				ret.push_back(*(it->value));
			}
			return ret;
		}

		[[nodiscard]] auto weights(N const& src, N const& dst) -> std::vector<E> {
			if (!is_node(src) || !is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::weights if src or dst node "
				                         "don't exist in the graph");
			}

			struct node findNode;
			findNode.value = std::make_shared<N>(src);
			auto srcNode = nodes_.find(findNode);

			struct node findNode2;
			findNode2.value = std::make_shared<N>(dst);
			auto dstNode = nodes_.find(findNode2);

			std::vector<E> ret;
			for (auto it = edges.begin(); it != edges.end(); it++) {
				if (it->from == srcNode->value && it->to == dstNode->value) {
					ret.push_back(it->weight);
				}
			}

			return ret;
		}

		[[nodiscard]] auto find(N const& src, N const& dst, E const& weight) -> iterator {
			struct edge findEdge;
			findEdge.from = std::make_shared<N>(src);
			findEdge.to = std::make_shared<N>(dst);
			findEdge.weight = weight;

			auto it = edges.find(findEdge);
			if (it != edges.end()) {
				return iterator(it);
			}
			return iterator(edges.end());
		}

		[[nodiscard]] auto connections(N const& src) -> std::vector<N> {
			if (!is_node(src)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::connections if src doesn't "
				                         "exist in the graph");
			}

			struct node findNode;
			findNode.value = std::make_shared<N>(src);
			auto srcNode = nodes_.find(findNode);

			std::vector<N> ret;
			N prev;
			for (auto it = edges.begin(); it != edges.end(); it++) {
				if (it->from == srcNode->value) {
					if (prev != *(it->to)) {
						ret.push_back(*(it->to));
					}
					prev = *(it->to);
				}
			}

			return ret;
		}

		[[nodiscard]] auto operator==(graph const& other) const -> bool {
			return (nodes_ == other.nodes_ && edges == other.edges);
		}

		friend auto operator<<(std::ostream& os, graph const& g) -> std::ostream& {
			auto nodesit = g.nodes_.begin();
			N curr = *(nodesit->value);
			os << curr << " (\n";

			for (auto edgesit = g.edges.begin(); edgesit != g.edges.end();) {
				if (curr != *(edgesit->from)) {
					nodesit++;
					curr = *(nodesit->value);
					os << ")\n" << curr << " (\n";
				}
				else {
					os << "  " << *(edgesit->to) << " | " << edgesit->weight << "\n";
					edgesit++;
				}
			}

			nodesit++;
			while (nodesit != g.nodes_.end()) {
				os << ")\n" << *(nodesit->value) << " (\n";
				nodesit++;
			}
			os << ")\n";
			return os;
		}

	private:
		std::set<node> nodes_;
		std::set<edge> edges;
	};
} // namespace gdwg

#endif // GDWG_GRAPH_HPP
