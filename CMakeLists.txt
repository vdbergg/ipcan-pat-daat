cmake_minimum_required(VERSION 3.10)
project(ipcan-pat-daat )

set(CMAKE_CXX_STANDARD 11)

add_executable(ipcan-pat-daat main.cpp header/TrieNode.h cpp/Trie.cpp header/Trie.h cpp/utils.cpp header/utils.h cpp/Framework.cpp header/Framework.h header/C.h cpp/Experiment.cpp header/Experiment.h header/ShortVector.h header/Directives.h cpp/StaticString.cpp header/StaticString.h cpp/ActiveNodeIpcan.cpp header/ActiveNodeIpcan.h header/crow_all.h)
