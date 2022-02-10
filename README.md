# trie_dictionary
A very simple trie dictionary implementation for auto-completion in C++17.

This is quite likely not the fastest implementation. But it's simple and usable.

## Basic Usage

```cpp
spl::trie trie;
trie.insert("tes"); // will not be in auto complete list
trie.insert("test");
trie.insert("test String 3");
trie.insert("test String 4!");

std::vector<std::string> auto_complete_list;
trie.auto_complete("tes", auto_complete_list, 5); // auto complete with a limit of 5
```
