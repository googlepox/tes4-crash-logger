#pragma once
#include <string>

template <class T>
std::string LogClassLineByLine(T& obj)
{
    std::string out;
	std::vector<std::string> vec = LogClass(member);
	for (const auto& i : vec)
		output += i + '\n';
    return out;
}