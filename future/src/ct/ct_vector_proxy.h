/*
 * ct_vector_proxy.hpp
 *
 * Copyright 2017-2020 Giuseppe Penone <giuspen@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#pragma once

#include <string>
#include <vector>
#include <utility>

template<typename ITEM_T>
class CtVectorProxy
{
    using vect_t = std::vector<ITEM_T>;
public:
    /**
     * @brief Swap two CtVectorProxy objects
     * @param first
     * @param second
     */
    friend void swap(CtVectorProxy& first, CtVectorProxy& second) noexcept
    {
        using std::swap;
        
        swap(first._internal_vec, second._internal_vec);
        swap(first._item_cache, second._item_cache);
        swap(first._set_cache, second._set_cache);
    }
    
    /**
     * @brief Construct a new proxy out of iterators compatable with vect_t's ctor
     * @param begin
     * @param end
     */
    template<class Iterator>
    CtVectorProxy(Iterator begin, Iterator end) : _internal_vec(begin, end) {}
    
    template<typename... TYPE_T>
    explicit CtVectorProxy(TYPE_T... args) : _internal_vec{args...} {}
    
    typename vect_t::reference operator[](typename vect_t::size_type index) { return _internal_vec[index]; }
    
    typename vect_t::const_reference operator[](typename vect_t::size_type index) const { return _internal_vec[index]; }
    
    /**
     * @brief Implicit covnersion operator
     * @return
     */
    operator ITEM_T() const
    {
        _load_cache();
        return _item_cache;
    }
    operator vect_t() const
    {
        return _internal_vec;
    }
    
    CtVectorProxy(const CtVectorProxy&) = default;
    CtVectorProxy(CtVectorProxy&&) noexcept = default;
    
    CtVectorProxy& operator=(CtVectorProxy other)
    {
        swap(*this, other);
        return *this;
    }
    
    CtVectorProxy& operator=(ITEM_T&& item)
    {
        CtVectorProxy tmp(std::forward<ITEM_T>(item));
        swap(*this, tmp);
        return *this;
    }
    
    template<typename T>
    constexpr typename vect_t::const_iterator find(const T& item) const
    {
        return std::find(_internal_vec.begin(), _internal_vec.end(), item);
    }
    template<typename T>
    constexpr typename vect_t::iterator find(const T& item)
    {
        return std::find(_internal_vec.begin(), _internal_vec.end(), item);
    }
    
    [[nodiscard]] constexpr typename vect_t::size_type size() const { return _internal_vec.size(); }
    
    [[nodiscard]] constexpr typename vect_t::const_iterator end() const noexcept { return _internal_vec.end(); }
    
    [[nodiscard]] constexpr typename vect_t::const_iterator begin() const noexcept { return _internal_vec.begin(); }
    
    template<typename T>
    constexpr bool contains(const T& item) const
    {
        return std::find(_internal_vec.begin(), _internal_vec.end(), item) != _internal_vec.end();
    }
    
    const ITEM_T& item() const
    {
        _load_cache();
        return _item_cache;
    }

private:
    vect_t _internal_vec;
    mutable ITEM_T _item_cache;
    mutable bool _set_cache = false;
    
    void _load_cache() const
    {
        if (!_set_cache) {
            for (const auto &item : _internal_vec) {
                _item_cache += item;
            }
            _set_cache = true;
        }
    }
};

using CtStringVectorProxy = CtVectorProxy<std::string>;

