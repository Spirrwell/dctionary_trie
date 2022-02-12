#ifndef SPL_DICTIONARY_TRIE
#define SPL_DICTIONARY_TRIE

#include <string>
#include <string_view>
#include <array>
#include <memory>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <functional>

namespace spl
{
	namespace internal
	{
		static constexpr std::string_view characterSet = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
		static constexpr char ascii_min = ' ';
		static constexpr char ascii_max = '~';

		static constexpr std::array<char, 256> initialize_characterset_map()
		{
			std::array<char, 256> arr{};

			for (std::size_t i = 0; i < characterSet.size(); ++i)
				arr[characterSet[i]] = i;

			return arr;
		}

		static constexpr std::array<char, 256> characterSetMap = initialize_characterset_map();
	}

	struct dict_trie
	{
		struct node
		{
			node(const char symbol = '\0') :
				symbol(symbol)
			{
			}

			const char symbol;
			bool isStringEnd = false;

			std::array<std::unique_ptr<node>, internal::characterSet.size()> children;
			node *firstChild = nullptr;
			node *next = nullptr;
		};

		node root;

		std::size_t maxKeyLength = 0;
		std::size_t numKeys = 0;

		bool is_valid_character(const char c) const
		{
			return (c >= internal::ascii_min && c <= internal::ascii_max);
		}

		void insert(std::string_view key)
		{
			node *parent = &root;

			for (const char c : key)
			{
				if (!is_valid_character(c))
					throw std::runtime_error("Bad characters in string");

				const std::size_t index = internal::characterSetMap[c];
				
				if (!parent->children[index])
				{
					parent->children[index] = std::make_unique<node>(c);

					if (!parent->firstChild || c < parent->firstChild->symbol)
					{
						parent->children[index]->next = parent->firstChild;
						parent->firstChild = parent->children[index].get();
					}
					else
					{
						for (node *current = parent->firstChild; current != nullptr; current = current->next)
						{
							if (!current->next)
							{
								current->next = parent->children[index].get();
								break;
							}
							else if (current->next->symbol > c)
							{
								parent->children[index]->next = current->next;
								current->next = parent->children[index].get();
								break;
							}
						}
					}
				}

				parent = parent->children[index].get();
			}

			parent->isStringEnd = true;

			maxKeyLength = (std::max)(maxKeyLength, key.size());
			++numKeys;
		}

		bool contains(std::string_view key) const
		{
			const node *parent = &root;

			for (const char c : key)
			{
				if (!is_valid_character(c))
					return false;
				
				const std::size_t index = internal::characterSetMap[c];

				parent = parent->children[index].get();
				if (!parent)
					return false;
			}

			return parent->isStringEnd;
		}

		void each(std::function<void(const std::string&)> callbackFn)
		{
			if (!callbackFn)
				return;

			const node *startNode = &root;
			if (!startNode->firstChild)
				return;
			
			std::string working_string;
			working_string.reserve(maxKeyLength);

			auto recursiveEach = [&working_string, &callbackFn](const node *startNode, auto &recursiveFn) -> void {
				for (const node *current = startNode; current != nullptr; current = current->next)
				{
					working_string += current->symbol;

					if (current->isStringEnd)
						callbackFn(working_string);
					
					if (current->firstChild)
						recursiveFn(current->firstChild, recursiveFn);
					else
						working_string.pop_back();
				}

				if (!working_string.empty())
					working_string.pop_back();
			};

			recursiveEach(startNode->firstChild, recursiveEach);
		}

		void each(std::string_view prefix, std::function<void(const std::string&)> callbackFn)
		{
			if (!callbackFn)
				return;
			
			const node *startNode = &root;
			for (const char c : prefix)
			{
				if (!is_valid_character(c))
					return;
				
				const std::size_t index = internal::characterSetMap[c];

				startNode = startNode->children[index].get();
				if (!startNode)
					return;
			}

			if (!startNode->firstChild)
				return;
			
			std::string working_string;
			working_string.reserve(maxKeyLength);
			working_string.append(prefix);

			auto recursiveEach = [&working_string, &callbackFn](const node *startNode, auto &recursiveFn) -> void {
				for (const node *current = startNode; current != nullptr; current = current->next)
				{
					working_string += current->symbol;

					if (current->isStringEnd)
						callbackFn(working_string);
					
					if (current->firstChild)
						recursiveFn(current->firstChild, recursiveFn);
					else
						working_string.pop_back();
				}

				if (!working_string.empty())
					working_string.pop_back();
			};

			recursiveEach(startNode->firstChild, recursiveEach);
		}

		void auto_complete(std::string_view prefix, std::vector<std::string> &out) const
		{
			const node *startNode = &root;
			for (const char c : prefix)
			{
				if (!is_valid_character(c))
					return;
				
				const std::size_t index = internal::characterSetMap[c];

				startNode = startNode->children[index].get();
				if (!startNode)
					return;
			}

			if (!startNode->firstChild)
				return;
			
			std::string working_string;
			working_string.reserve(maxKeyLength);
			working_string.append(prefix);

			auto recursiveComplete = [&working_string, &out](const node *startNode, auto &recursiveFn) -> void {
				for (const node *current = startNode; current != nullptr; current = current->next)
				{
					working_string += current->symbol;

					if (current->isStringEnd)
						out.emplace_back(working_string);
					
					if (current->firstChild)
						recursiveFn(current->firstChild, recursiveFn);
					else
						working_string.pop_back();
				}

				if (!working_string.empty())
					working_string.pop_back();
			};

			recursiveComplete(startNode->firstChild, recursiveComplete);
		}

		void auto_complete(std::string_view prefix, std::vector<std::string> &out, std::size_t limit) const
		{
			if (limit == 0)
				return;

			const node *startNode = &root;
			for (const char c : prefix)
			{
				if (!is_valid_character(c))
					return;
				
				const std::size_t index = internal::characterSetMap[c];

				startNode = startNode->children[index].get();
				if (!startNode)
					return;
			}

			if (!startNode->firstChild)
				return;
			
			std::string working_string;
			working_string.reserve(maxKeyLength);
			working_string.append(prefix);

			out.reserve(limit);

			auto recursiveComplete = [&working_string, &out, &limit](const node *startNode, auto &recursiveFn) -> void {
				for (const node *current = startNode; current != nullptr; current = current->next)
				{
					if (out.size() == limit)
						return;

					working_string += current->symbol;

					if (current->isStringEnd)
						out.emplace_back(working_string);
					
					if (current->firstChild)
						recursiveFn(current->firstChild, recursiveFn);
					else
						working_string.pop_back();
				}

				if (!working_string.empty())
					working_string.pop_back();
			};

			recursiveComplete(startNode->firstChild, recursiveComplete);
		}

		std::size_t size() const { return numKeys; }
	};
}

#endif // SPL_DICTIONARY_TRIE
