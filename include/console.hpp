#pragma once
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <iterator>
#include <ranges>
#include <set>
#include <source_location>
#include <string_view>
#include <type_traits>
#include <vector>

namespace
{

template <std::ranges::input_range R>
inline std::string repr(R &&);
template <std::ranges::random_access_range R>
inline std::string repr(R &&);
template <typename T, typename U>
inline std::string repr(const std::pair<T, U> &);

using namespace std::literals;
inline std::string repr(const std::string &val)
{
    return "\33[36m" + val + "\33[39m";
}
inline std::string repr(const std::string_view val)
{
    return "\33[36m" + std::string(val) + "\33[39m";
}
inline std::string repr(const std::filesystem::path &val)
{
    return "\33[32m" + val.string() + "\33[39m";
}
template <typename T>
requires std::is_arithmetic_v<T>
inline std::string repr(const T val)
{
    return "\33[35m" + std::to_string(val) + "\33[39m";
}
template <typename... Args>
inline std::string repr(const std::chrono::duration<Args...> &val)
{
    return "\33[34m" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(val).count()) + " ms" +
           "\33[39m";
}
inline std::string repr(const std::source_location &value)
{
    return "\33[31m" + std::string{value.file_name()} + ":" + std::to_string(value.line()) + "\33[39m";
}
template <typename... Args>
inline std::string repr(const std::tuple<Args...> &val)
{
    return std::apply(
        [&](auto &&...args) {
            std::string res = "(";
            size_t n = 0;
            ((res+=repr(args)+((++n)!=sizeof...(args)?", ":"")),...);
            res+=")";
            return res;
        },
        val);
}
template <typename T, typename U>
inline std::string repr(const std::pair<T, U> &val)
{
    return "<" + repr(val.first) + ", " + repr(val.second) + ">";
}
template <std::ranges::input_range R>
inline std::string repr(R &&rng)
{
    std::string res = "{";
    for (std::string delim = ""; auto &&val : rng)
        res += delim + repr(val), delim = ", ";
    return std::move(res) + "}";
}
template <std::ranges::random_access_range R>
inline std::string repr(R &&rng)
{
    std::string res = "[";
    for (std::string delim = ""; auto &&val : rng)
        res += delim + repr(val), delim = ", ";
    return std::move(res) + "]";
}
}; // namespace

// output
inline void print(std::ostream&os ,std::string_view context)
{
    os << context << std::endl;
}
void print(std::ostream& os,std::string_view context, auto &&first_arg, auto &&...args)
{
    for (size_t i = 0; i < context.size(); i++)
    {
        if (context[i] == '{' && i + 1 < context.size() && context[i + 1] == '}')
        {
            os << repr(first_arg);
            print(os,context.substr(i + 2),std::forward<decltype(args)>(args)...);
            return;
        }
        os << context[i];
    }
    os << std::endl;
}
void print(std::string_view context, auto &&...args){
    print(std::cout,context,std::forward<decltype(args)>(args)...);
}

// input from stream
template <typename T, typename U>
std::istream &operator>>(std::istream &, std::pair<T, U> &);
template <typename... Args>
std::istream &operator>>(std::istream &, std::tuple<Args...> &);

template <typename T, typename U>
std::istream &operator>>(std::istream &is, std::pair<T, U> &value)
{
    return is >> value.first >> value.second;
}
template <typename... Args>
std::istream &operator>>(std::istream &is, std::tuple<Args...> &value)
{
    std::apply([&](auto &...args) { (is >> ... >> args); }, value);
    return is;
}
template <typename T, size_t N>
std::istream &operator>>(std::istream &is, std::array<T, N> &value)
{
    for (size_t i = 0; i < N; i++)
        is >> value[i];
    return is;
}
template <typename... Args>
std::istream &operator>>(std::istream &is, std::vector<Args...> &value)
{
    size_t size;
    is >> size;
    value.resize(size);
    for (auto &val : value)
        is >> val;
    return is;
}

// bench
template <std::invocable Fn>
requires std::is_same_v<std::invoke_result_t<Fn>, void>
auto bench(Fn &&func, std::string_view description = "function call", std::source_location location = std::source_location::current())
{
    auto start_time = std::chrono::steady_clock::now();
    std::invoke(func);
    auto time_cost = std::chrono::steady_clock::now() - start_time;
    print("{} at {} takes {}", description, location, time_cost);
}
template <std::invocable Fn>
requires(!std::is_same_v<std::invoke_result_t<Fn>, void>)
auto bench(Fn &&func, std::string_view description = "function call", std::source_location location = std::source_location::current())
{
    auto start_time = std::chrono::steady_clock::now();
    auto res = std::invoke(func);
    auto time_cost = std::chrono::steady_clock::now() - start_time;
    print("{} at {} takes {}", description, location, time_cost);
    return res;
}
