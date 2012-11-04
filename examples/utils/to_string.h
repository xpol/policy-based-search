/*
    Copyright (C) 2012  Jeremy W. Murphy <jeremy.william.murphy@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <vector>
#include <string>
#include <set>
#include <unordered_set>

namespace jwm
{
	template <template <typename T, typename Alloc = std::allocator<T>> class Container, typename T>
	inline std::string open_bracket(Container<T> const &)
	{
		return "[";
	}
	
	
	template <template <typename T, typename Alloc = std::allocator<T>> class Container, typename T>
	inline std::string close_bracket(Container<T> const &)
	{
		return "]";
	}
	
	
	template <typename T>
	inline std::string open_bracket(std::vector<T> const &)
	{
		return "⟨";
	}
	
	
	template <typename T>
	inline std::string close_bracket(std::vector<T> const &)
	{
		return "⟩";
	}
	
	
	template <typename T>
	inline std::string open_bracket(std::set<T> const &)
	{
		return "{";
	}
	
	
	template <typename T>
	inline std::string close_bracket(std::set<T> const &)
	{
		return "}";
	}
	

	template <typename T>
	inline std::string open_bracket(std::unordered_set<T> const &)
	{
		return "{";
	}
	
	
	template <typename T>
	inline std::string close_bracket(std::unordered_set<T> const &)
	{
		return "}";
	}
	
	
	template <template <typename T, typename Alloc = std::allocator<T>> class Container, typename T>
	std::string to_string(Container<T> const &V)
	{
		std::string s(open_bracket(V));
		
		if(!V.empty())
		{
			std::for_each(std::begin(V), std::end(V) - 1, [&](typename Container<T>::const_reference E)
			{
				s += std::to_string(E) + ", ";
			});
			
			s += std::to_string(V.back());
		}
		
		s += close_bracket(V);
		
		return s;
	}
}
