#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <new>


template<typename T>
auto VarGen(){
}

template<typename T>
auto FunGen(){
}


auto& v0 = std::cout;
        


auto f0 = static_cast<const int & (std::vector<int>::*)(unsigned long) const>(&std::vector<int>::at);
        

auto f1 = static_cast<__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>> (std::vector<int>::*)() const noexcept>(&std::vector<int>::end);
        

auto f2 = static_cast<const int & (std::vector<int>::*)() const noexcept>(&std::vector<int>::back);
        

auto f3 = static_cast<__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>> (std::vector<int>::*)() const noexcept>(&std::vector<int>::cend);
        

auto f4 = static_cast<const int * (std::vector<int>::*)() const noexcept>(&std::vector<int>::data);
        

auto f5 = static_cast<std::reverse_iterator<__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>>> (std::vector<int>::*)() const noexcept>(&std::vector<int>::rend);
        

auto f6 = static_cast<unsigned long (std::vector<int>::*)() const noexcept>(&std::vector<int>::size);
        

auto f7 = static_cast<__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>> (std::vector<int>::*)() const noexcept>(&std::vector<int>::begin);
        

auto f8 = static_cast<std::reverse_iterator<__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>>> (std::vector<int>::*)() const noexcept>(&std::vector<int>::crend);
        

auto f9 = static_cast<bool (std::vector<int>::*)() const noexcept>(&std::vector<int>::empty);
        

auto f10 = static_cast<const int & (std::vector<int>::*)() const noexcept>(&std::vector<int>::front);
        

auto f11 = static_cast<__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>> (std::vector<int>::*)() const noexcept>(&std::vector<int>::cbegin);
        

auto f12 = static_cast<std::reverse_iterator<__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>>> (std::vector<int>::*)() const noexcept>(&std::vector<int>::rbegin);
        

auto f13 = static_cast<std::reverse_iterator<__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>>> (std::vector<int>::*)() const noexcept>(&std::vector<int>::crbegin);
        

auto f14 = static_cast<unsigned long (std::vector<int>::*)() const noexcept>(&std::vector<int>::capacity);
        

auto f15 = static_cast<unsigned long (std::vector<int>::*)() const noexcept>(&std::vector<int>::max_size);
        

auto f16 = static_cast<const int & (std::vector<int>::*)(unsigned long) const noexcept>(&std::vector<int>::operator[]);
        

auto f17 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(char)>(&std::basic_ostream<char>::put);
        

auto f18 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)()>(&std::basic_ostream<char>::flush);
        

auto f19 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(std::fpos<__mbstate_t>)>(&std::basic_ostream<char>::seekp);
        

auto f20 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(long, std::_Ios_Seekdir)>(&std::basic_ostream<char>::seekp);
        

auto f21 = static_cast<std::fpos<__mbstate_t> (std::basic_ostream<char>::*)()>(&std::basic_ostream<char>::tellp);
        

auto f22 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(const char *, long)>(&std::basic_ostream<char>::write);
        

auto f23 = static_cast<void (std::basic_ostream<char>::*)(const char *, long)>(&std::basic_ostream<char>::_M_write);
        
using t24 = std::basic_ostream<char>;
using t25 = std::basic_ostream<char>;

auto f26 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(std::nullptr_t)>(&std::basic_ostream<char>::operator<<);
        

auto f27 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(std::basic_ostream<char, std::char_traits<char>> &(*)(std::basic_ostream<char, std::char_traits<char>> &))>(&std::basic_ostream<char>::operator<<);
        

auto f28 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(std::ios_base &(*)(std::ios_base &))>(&std::basic_ostream<char>::operator<<);
        

auto f29 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(std::basic_ios<char, std::char_traits<char>> &(*)(std::basic_ios<char, std::char_traits<char>> &))>(&std::basic_ostream<char>::operator<<);
        

auto f30 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(const void *)>(&std::basic_ostream<char>::operator<<);
        

auto f31 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(std::basic_streambuf<char, std::char_traits<char>> *)>(&std::basic_ostream<char>::operator<<);
        

auto f32 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(bool)>(&std::basic_ostream<char>::operator<<);
        

auto f33 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(double)>(&std::basic_ostream<char>::operator<<);
        

auto f34 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(long double)>(&std::basic_ostream<char>::operator<<);
        

auto f35 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(float)>(&std::basic_ostream<char>::operator<<);
        

auto f36 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(int)>(&std::basic_ostream<char>::operator<<);
        

auto f37 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(unsigned int)>(&std::basic_ostream<char>::operator<<);
        

auto f38 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(long)>(&std::basic_ostream<char>::operator<<);
        

auto f39 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(unsigned long)>(&std::basic_ostream<char>::operator<<);
        

auto f40 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(short)>(&std::basic_ostream<char>::operator<<);
        

auto f41 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(unsigned short)>(&std::basic_ostream<char>::operator<<);
        

auto f42 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(long long)>(&std::basic_ostream<char>::operator<<);
        

auto f43 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (std::basic_ostream<char>::*)(unsigned long long)>(&std::basic_ostream<char>::operator<<);
        

auto f44 = static_cast<void (std::vector<int>::*)()>(&std::vector<int>::shrink_to_fit);
        

auto f45 = static_cast<int & (std::vector<int>::*)(unsigned long)>(&std::vector<int>::at);
        

auto f46 = static_cast<__gnu_cxx::__normal_iterator<int *, std::vector<int, std::allocator<int>>> (std::vector<int>::*)() noexcept>(&std::vector<int>::end);
        

auto f47 = static_cast<int & (std::vector<int>::*)() noexcept>(&std::vector<int>::back);
        

auto f48 = static_cast<int * (std::vector<int>::*)() noexcept>(&std::vector<int>::data);
        

auto f49 = static_cast<std::reverse_iterator<__gnu_cxx::__normal_iterator<int *, std::vector<int, std::allocator<int>>>> (std::vector<int>::*)() noexcept>(&std::vector<int>::rend);
        

auto f50 = static_cast<void (std::vector<int>::*)(std::vector<int, std::allocator<int>> &) noexcept>(&std::vector<int>::swap);
        

auto f51 = static_cast<__gnu_cxx::__normal_iterator<int *, std::vector<int, std::allocator<int>>> (std::vector<int>::*)() noexcept>(&std::vector<int>::begin);
        

auto f52 = static_cast<void (std::vector<int>::*)() noexcept>(&std::vector<int>::clear);
        

auto f53 = static_cast<__gnu_cxx::__normal_iterator<int *, std::vector<int, std::allocator<int>>> (std::vector<int>::*)(__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>>)>(&std::vector<int>::erase);
        

auto f54 = static_cast<__gnu_cxx::__normal_iterator<int *, std::vector<int, std::allocator<int>>> (std::vector<int>::*)(__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>>, __gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>>)>(&std::vector<int>::erase);
        

auto f55 = static_cast<int & (std::vector<int>::*)() noexcept>(&std::vector<int>::front);
        

auto f56 = static_cast<void (std::vector<int>::*)(std::initializer_list<int>)>(&std::vector<int>::assign);
        

auto f57 = static_cast<void (std::vector<int>::*)(unsigned long, const int &)>(&std::vector<int>::assign);
        

auto f58 = static_cast<__gnu_cxx::__normal_iterator<int *, std::vector<int, std::allocator<int>>> (std::vector<int>::*)(__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>>, int &&)>(&std::vector<int>::insert);
        

auto f59 = static_cast<__gnu_cxx::__normal_iterator<int *, std::vector<int, std::allocator<int>>> (std::vector<int>::*)(__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>>, const int &)>(&std::vector<int>::insert);
        

auto f60 = static_cast<__gnu_cxx::__normal_iterator<int *, std::vector<int, std::allocator<int>>> (std::vector<int>::*)(__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>>, std::initializer_list<int>)>(&std::vector<int>::insert);
        

auto f61 = static_cast<__gnu_cxx::__normal_iterator<int *, std::vector<int, std::allocator<int>>> (std::vector<int>::*)(__gnu_cxx::__normal_iterator<const int *, std::vector<int, std::allocator<int>>>, unsigned long, const int &)>(&std::vector<int>::insert);
        

auto f62 = static_cast<std::reverse_iterator<__gnu_cxx::__normal_iterator<int *, std::vector<int, std::allocator<int>>>> (std::vector<int>::*)() noexcept>(&std::vector<int>::rbegin);
        

auto f63 = static_cast<void (std::vector<int>::*)(unsigned long)>(&std::vector<int>::resize);
        

auto f64 = static_cast<void (std::vector<int>::*)(unsigned long, const int &)>(&std::vector<int>::resize);
        

auto f65 = static_cast<void (std::vector<int>::*)(unsigned long)>(&std::vector<int>::reserve);
        

auto f66 = static_cast<void (std::vector<int>::*)() noexcept>(&std::vector<int>::pop_back);
        

auto f67 = static_cast<void (std::vector<int>::*)(int &&)>(&std::vector<int>::push_back);
        

auto f68 = static_cast<void (std::vector<int>::*)(const int &)>(&std::vector<int>::push_back);
        
using t69 = std::vector<int>;
using t70 = std::vector<int>;
using t71 = std::vector<int>;
using t72 = std::vector<int>;
using t73 = std::vector<int>;
using t74 = std::vector<int>;
using t75 = std::vector<int>;
using t76 = std::vector<int>;
using t77 = std::vector<int>;
using t78 = std::vector<int>;
using t79 = std::vector<int>;
using t80 = std::vector<int>;
using t81 = std::vector<int>;
using t82 = std::vector<int>;
using t83 = std::vector<int>;
using t84 = std::vector<int>;
using t85 = std::vector<int>;
using t86 = std::vector<int>;

auto f87 = static_cast<std::vector<int, std::allocator<int>> & (std::vector<int>::*)(std::vector<int, std::allocator<int>> &&)>(&std::vector<int>::operator=);
        

auto f88 = static_cast<std::vector<int, std::allocator<int>> & (std::vector<int>::*)(const std::vector<int, std::allocator<int>> &)>(&std::vector<int>::operator=);
        

auto f89 = static_cast<std::vector<int, std::allocator<int>> & (std::vector<int>::*)(std::initializer_list<int>)>(&std::vector<int>::operator=);
        

auto f90 = static_cast<int & (std::vector<int>::*)(unsigned long) noexcept>(&std::vector<int>::operator[]);
        

auto f91 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (*)(std::basic_ostream<char, std::char_traits<char>> &)>(&std::endl);
        

auto f92 = static_cast<std::basic_ostream<char, std::char_traits<char>> & (*)(std::basic_ostream<char, std::char_traits<char>> &, const char *)>(&std::operator<<);
        


namespace t86_ {
	t86* res;
	uint8_t addr[sizeof(t86)];
	auto t86_instance = [](){
		res = new(addr) t86();
	};
}

namespace f68_ {
	int  p0;
	auto f68_instance = [](){
		(t86_::res->*f68)(p0);
	};
}

namespace f92_ {
	char * p0 = new char();
	std::basic_ostream<char, std::char_traits<char>>  *res;
	const auto f92_instance = [](){
		res = &f92(v0, p0);
	};
}

namespace f39_ {
	unsigned long p0;
	std::basic_ostream<char, std::char_traits<char>>  * res;
	const auto f39_instance = [](){
		res = &(f92_::res->*f39)(p0);
	};
}

namespace f90_ {
	unsigned long p0;
	int* res;
	const auto f90_instance = [](){
		res = &(t86_::res->*f90)(p0);
	};
}

namespace f36_ {
	int p0;
	std::basic_ostream<char, std::char_traits<char>>  * res;
	const auto f36_instance = [](){
		res = &(f92_::res->*f36)(p0);
	};
}

namespace f27_ {
	std::basic_ostream<char, std::char_traits<char>>  * res;
	const auto f27_instance = [](){
		res = &(f36_::res->*f27)(f91);
	};
}

std::unordered_map<std::string, void*> var_map = {
	{"t86_::addr", t86_::addr},
	{"f39_::p0", &f39_::p0},
	{"f39_::res", reinterpret_cast<void*>(f39_::res)},
	{"f92_::p0", f92_::p0},
	{"f92_::res", reinterpret_cast<void*>(f92_::res)},
	{"f27_::res", reinterpret_cast<void*>(f27_::res)},
	{"f90_::p0", &f90_::p0},
	{"f90_::res", reinterpret_cast<void*>(f90_::res)},
	{"f68_::p0", &f68_::p0},
	{"f36_::p0", &f36_::p0},
	{"f36_::res", reinterpret_cast<void*>(f36_::res)},
};
std::unordered_map<std::string, std::function<void()>> func_map = {
	{"t86", t86_::t86_instance},
	{"f39", f39_::f39_instance},
	{"f92", f92_::f92_instance},
	{"f27", f27_::f27_instance},
	{"f90", f90_::f90_instance},
	{"f68", f68_::f68_instance},
	{"f36", f36_::f36_instance},
};
