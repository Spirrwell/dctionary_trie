#ifndef SPL_TRIE_DICTIONARY
#define SPL_TRIE_DICTIONARY

#include <string>
#include <string_view>
#include <array>
#include <memory>
#include <stdexcept>
#include <vector>

namespace spl
{
	namespace internal
	{
		static constexpr std::string_view characterSet = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

		static constexpr std::array<unsigned char, 256> initialize_characterset_map()
		{
			std::array<unsigned char, 256> arr{};

			for (std::size_t i = 0; i < characterSet.size(); ++i)
				arr[characterSet[i]] = static_cast<unsigned char>(i);

			return arr;
		}

		static constexpr std::array<unsigned char, 256> characterSetMap = initialize_characterset_map();
	}

	struct trie
	{
		void insert(std::string_view str)
		{
			trie *node = this;
			for (const unsigned char c : str)
			{
				if (c < static_cast<unsigned char>(internal::characterSet[0]) || c > static_cast<unsigned char>(internal::characterSet[internal::characterSet.size() - 1]))
					throw std::runtime_error("Invalid character in string");

				const std::size_t index = character_to_index(c);

				if (!node->child[index])
					node->child[index] = std::make_unique<trie>();
				
				node = node->child[index].get();
			}

			node->isStringEnd = true;
		}

		bool contains(std::string_view key) const
		{
			const trie *node = this;

			for (const unsigned char c : key)
			{
				const std::size_t index = character_to_index(c);

				if (!node->child[index])
					return false;
				
				node = node->child[index].get();
			}

			return node->isStringEnd;
		}

		void auto_complete(std::string_view prefix, std::vector<std::string> &out, std::size_t limit) const
		{
			if (limit == 0)
				return;

			const trie *node = this;

			for (const unsigned char c : prefix)
			{
				if (c < static_cast<unsigned char>(internal::characterSet[0]) || c > static_cast<unsigned char>(internal::characterSet[internal::characterSet.size() - 1]))
					return;

				const std::size_t index = character_to_index(c);

				if (!node->child[index])
					return;
				
				node = node->child[index].get();
			}

			if (node->is_last())
				return;
			
			std::string working_string(prefix);

			out.reserve(limit);
			recursive_auto_complete(node, working_string, out, limit);
		}

	private:
		bool is_last() const
		{
			const trie *node = this;
			for (auto &child : node->child)
			{
				if (child)
					return false;
			}

			return true;
		}

		void recursive_auto_complete(const trie *node, std::string &working_string, std::vector<std::string> &out, std::size_t limit) const
		{
			for (std::size_t index = 0; index < node->child.size(); ++index)
			{
				if (out.size() == limit)
					return;
				
				auto &child = node->child[index];
				
				if (!child.get())
					continue;
				
				working_string += internal::characterSet[index];
				
				if (child->isStringEnd)
				{
					out.emplace_back(working_string);

					if (child->is_last())
					{
						working_string.pop_back();
						continue;
					}
				}

				recursive_auto_complete(child.get(), working_string, out, limit);
				working_string.pop_back();
			}
		}

		static std::size_t character_to_index(const unsigned char c) { return internal::characterSetMap[c]; }

		bool isStringEnd = false;
		std::array<std::unique_ptr<trie>, internal::characterSet.size()> child;
	};

}

#endif // SPL_TREE_DICTIONARY
