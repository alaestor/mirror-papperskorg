
#include <vector>
#include <utility>
#include <memory>
#include <concepts>
#include <cassert>
#include <optional>
#include <algorithm>
#include <stdexcept>
#include <functional>

/*
	Crude constexpr / compile-time containers

	BE WARNED

	Bad, naive, unsafe, ultralazy interfaces & implementations!
	No exception or time complexity guarantees! not properly tested!

	Likely not fit for any purpose: use with caution, ideally not at all.
*/

// singly linked; deletions can be a bit expensive.
template <typename T>
struct ctlinkedlist
{
	struct node
	{
		T value;
		std::unique_ptr<node> next{};
	};

	std::unique_ptr<node> head{};

	constexpr node* walk(std::predicate<node&> auto&& pred)
	{
		auto* uptr{ &head };
		while (*uptr && !pred(*uptr->get())) uptr = &(*uptr)->next;
		return uptr->get();
	}

	[[nodiscard]] constexpr std::size_t count()
	{
		std::size_t i{ 0 };
		(void)walk([&i](node&){ ++i; return false; });
		return i;
	}

	[[nodiscard]] constexpr std::optional<std::reference_wrapper<T>>
	search(std::predicate<T&> auto&& pred)
	{
		node* const ptr{ walk([&pred](node& n){ return pred(n.value); }) };
		if (ptr != nullptr)
			return { ptr->value };
		else return std::nullopt;
		// return ptr != nullptr ? { (*ptr).value } : std::nullopt;
	}

	constexpr void insert_after(auto&& item, node* after)
	{
		assert(after != nullptr);
		after->next = make_unique<node>(
			std::forward<decltype(item)>(item),
			std::move(after->next)
		);
	}

	constexpr T& append(auto&& item)
	{
		if (head)
		{
			auto* leaf{ walk([](node& n){ return n.next == nullptr; }) };
			assert(leaf != nullptr);
			insert_after(item, leaf);
			return leaf->next->value;
		}
		else
		{
			head = std::make_unique<node>(std::forward<decltype(item)>(item));
			return head->value;
		}
	}

	// constexpr void remove(node* const element)
	// {
	// 	if (!head) return;
	// 	else if (head.get() != element)
	// 	{
	// 		auto* owner{ walk([element](node& n){ return n.next.get() == element; }) };
	// 		assert(owner != nullptr); // element wasn't in this list?
	// 		owner->next = std::move(owner->next->next);
	// 	}
	// 	else head.reset(head->next.release());
	// }

	constexpr bool remove(std::predicate<T&> auto&& pred)
	{
		if (!head)
			return false;

		if (pred(head->value))
		{
			head.reset(head->next.release());
			return true;
		}

		auto* cur{ &head->next};
		auto* prev{ &head };
		while (*cur)
		{
			if (pred((*cur)->value))
			{
				(*prev)->next.reset((*cur)->next.release());
				return true;
			}
			prev = cur;
			cur = &(*cur)->next;
		}

		return false;
	}
};

static_assert([](){ // crude tests
	const auto sum {
		[](ctlinkedlist<int>& l) -> int {
			int n = 0;
			auto* cursor = l.head.get();
			while (cursor != nullptr)
			{
				n += cursor->value;
				cursor = cursor->next.get();
			}
			return n;
		}
	};

	ctlinkedlist<int> l;
	assert(l.count() == 0);

	// appending
	l.append(1);
	assert(l.count() == 1);
	assert(sum(l) == 1);
	l.append(2);
	assert(l.count() == 2);
	assert(sum(l) == 3);
	l.append(4);
	assert(l.count() == 3);
	assert(sum(l) == 7);

	// searching
	assert(!l.search([](int i){ return i == 0; }));
	assert(l.search([](int i){ return i == 1; }));
	assert(l.search([](int i){ return i == 2; }));
	assert(l.search([](int i){ return i == 4; }));
	assert(!l.search([](int i){ return i == 5; }));

	// removing middle
	assert(l.remove([](int i){ return i == 2; }));
	assert(l.count() == 2);
	assert(l.search([](int i){ return i == 1; }));
	assert(!l.search([](int i){ return i == 2; }));
	assert(l.search([](int i){ return i == 4; }));

	// removing first
	assert(l.remove([](int i){ return i == 1; }));
	assert(l.count() == 1);
	assert(!l.search([](int i){ return i == 1; }));
	assert(l.search([](int i){ return i == 4; }));

	// double remove
	assert(!l.remove([](int i){ return i == 1; }));
	assert(l.count() == 1);
	assert(l.search([](int i){ return i == 4; }));

	return true;
}());

template <class T_key, class T_value, class T_hasher>
struct ctumap
{
	struct kvpair_t { T_key key; T_value value; };
	using kvll_t = ctlinkedlist<kvpair_t>;
	struct hlpair_t { std::size_t hash; kvll_t list; };

	std::vector<hlpair_t> buckets{};

	[[nodiscard]] constexpr auto find_bucket(const std::size_t hash)
	{
		return std::find_if(buckets.begin(), buckets.end(),
			[hash](const hlpair_t& pair){ return pair.hash == hash; }
		);
	}

	[[nodiscard]] constexpr std::optional<std::reference_wrapper<kvll_t>>
	find_list(const std::size_t hash)
	{
		if (const auto it{ find_bucket(hash) }; it != buckets.end())
			return { it->list };
		else return std::nullopt;
	}

	[[nodiscard]] constexpr std::optional<std::reference_wrapper<kvpair_t>>
	find_kvpair(kvll_t& ll, const T_key& key)
	{ return ll.search([&key](const kvpair_t& r){ return r.key == key; }); }

	[[nodiscard]] constexpr std::optional<std::reference_wrapper<T_value>>
	find(const T_key& key)
	{
		if (const auto& llo{ find_list(T_hasher{}(key)) }; llo)
		{
			if (const auto& kvo{ find_kvpair(llo->get(), key) }; kvo)
				return { kvo->get().value };
		}
		return std::nullopt;
	}

	constexpr T_value& insert(T_key key, T_value value)
	{
		const std::size_t hash{ T_hasher{}(key) };
		if (const auto& llo{ find_list(hash) }; llo) // hash exists
		{
			auto& ll{ llo->get() };
			if (const auto& kvo{ find_kvpair(ll, key) }; kvo)
				throw std::invalid_argument("key already exists");
			return ll.append(kvpair_t{std::move(key), std::move(value)}).value;
		}
		else // hash doesn't exist
		{
			return buckets.emplace_back(hash, kvll_t{}).list.append(
				kvpair_t{ std::move(key), std::move(value) }
			).value;
		}
	}

	constexpr bool remove(const T_key& key)
	{
		if (const auto it{ find_bucket(T_hasher{}(key)) }; it != buckets.end())
		{ // hash found
			if (it->list.remove([&key](const kvpair_t& p){ return p.key == key; }))
			{ // removed element matching key
				if (it->list.count() == 0)
					buckets.erase(it); // also remove bucket if empty
				return true;
			}
		}
		return false;
	}

	[[nodiscard]] constexpr T_value& at(const T_key& key)
	{
		if (const auto& vo{ find(key) }; vo)
			return vo->get();
		throw std::invalid_argument("key not found");
	}

	[[nodiscard]] constexpr T_value& operator[](const T_key& key)
	{
		if (const auto& vo{ find(key) }; vo)
			return vo->get();

		return insert(key, T_value{});
	}
};

static_assert([](){// perfect hashes
	using hasher = decltype([](int i){ return static_cast<std::size_t>(i); });
	ctumap<int, int, hasher> map;
	// insert
	map.insert(1, 1);
	map.insert(2, 2);
	// find
	assert(map.find(1));
	assert(map.find(1).value().get() == 1);
	assert(map.find(2));
	assert(map.find(2).value().get() == 2);
	// []
	assert(map[2] == 2);
	assert(!map.find(3));
	map[3] = 3;
	assert(map.find(3));
	assert(map[3] == 3);
	// remove
	assert(map.remove(2));
	assert(map.find(1));
	assert(!map.find(2));
	assert(map.find(3));

	return true;
}());

static_assert([](){ // colliding hashes
	using collider = decltype([](int){ return size_t{100}; });
	ctumap<int, int, collider> map;
	// insert
	map.insert(3, 3);
	map.insert(4, 4);
	// find
	assert(map.find(3));
	assert(map.find(3).value().get() == 3);
	assert(map.find(4));
	assert(map.find(4).value().get() == 4);
	// []
	assert(map[3] == 3);
	assert(!map.find(5));
	map[5] = 5;
	assert(map.find(5));
	assert(map[5] == 5);
	// remove
	assert(map.remove(4));
	assert(map.find(3));
	assert(!map.find(4));
	assert(map.find(5));
	return true;
}());
