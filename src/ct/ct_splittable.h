/*
 * ct_splittable.h
 *
 * Copyright 2009-2020
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
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

#include <glibmm.h>
#include <string>
#include <vector>
#include <utility>

namespace {

template<typename T>
std::vector<T> split_items(const T& items) {
    std::vector<T> items_vect(items.size());
    for (const auto& item : items) {
        items_vect.emplace_back(item);
    }
    return items_vect;
}

template<>
std::vector<Glib::ustring> split_items(const Glib::ustring& str) {
    std::vector<Glib::ustring> items_vect;
    for (const auto& ch : str) {
        items_vect.emplace_back(1, ch);
    }
    return items_vect;
}

}
template<typename ITEM_T, typename ITEM_T_SPLITTER = decltype(&split_items<ITEM_T>)>
class CtSplittable
{
    using vect_t = std::vector<ITEM_T>;
public:
    using size_type = typename vect_t::size_type;

    /**
     * @brief Swap two CtVectorProxy objects
     * @param first
     * @param second
     */
    friend void swap(CtSplittable& first, CtSplittable& second) noexcept
    {
        using std::swap;

        swap(first._internal_vec, second._internal_vec);
        swap(first._item_cache, second._item_cache);
        swap(first._set_cache, second._set_cache);
        swap(first._set_vec_cache, second._set_vec_cache);
    }

    /**
     * @brief Construct a new proxy out of iterators compatable with vect_t's ctor
     * @param begin
     * @param end
     */
    template<class Iterator>
    CtSplittable(Iterator begin, Iterator end, ITEM_T_SPLITTER item_splitter = &split_items<ITEM_T>) : _item_splitter(item_splitter), _internal_vec(begin, end) {}

    typename vect_t::reference operator[](typename vect_t::size_type index) {
        _load_vec_cache();
        return _internal_vec[index];
    }

    typename vect_t::const_reference operator[](typename vect_t::size_type index) const {
        _load_vec_cache();
        return _internal_vec[index];
    }

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
        _load_vec_cache();
        return _internal_vec;
    }

    CtSplittable(const CtSplittable&) = default;
    CtSplittable(CtSplittable&&) noexcept = default;
    CtSplittable(ITEM_T item, ITEM_T_SPLITTER item_splitter = &split_items<ITEM_T>) : _item_splitter(item_splitter), _item_cache(std::move(item)), _set_vec_cache(false) {}

    CtSplittable& operator=(CtSplittable other)
    {
        swap(*this, other);
        return *this;
    }

    template<class ITEM_LIKE_T, std::enable_if_t<std::is_convertible_v<ITEM_LIKE_T, ITEM_T>>>
    CtSplittable& operator=(ITEM_LIKE_T&& item) {
        _load_cache();
        if (item != _item_cache) {
            _set_vec_cache = false;
            _set_cache = true;
            _item_cache = std::forward<ITEM_T>(item);
        }
        return *this;
    }

    /**
     * @brief Reset the CtVectorProxy's internal vector
     *
     * @tparam Iterator
     * @param begin
     * @param end
     */
    template<class Iterator>
    void reset(Iterator begin, Iterator end) {
        vect_t tmp(begin, end);
        _internal_vec.swap(tmp);
        _set_cache = false;
        _item_cache.reset(0);
    }

    template<typename T>
    constexpr typename vect_t::const_iterator find(const T& item) const
    {
        _load_vec_cache();
        return std::find(_internal_vec.begin(), _internal_vec.end(), item);
    }
    template<typename T>
    constexpr typename vect_t::iterator find(const T& item)
    {
        _load_vec_cache();
        return std::find(_internal_vec.begin(), _internal_vec.end(), item);
    }

    [[nodiscard]] constexpr typename vect_t::size_type size() const {
        _load_vec_cache();
        return _internal_vec.size();
    }

    [[nodiscard]] constexpr typename vect_t::const_iterator end() const noexcept {
        _load_vec_cache();
        return _internal_vec.end();
    }

    [[nodiscard]] constexpr typename vect_t::const_iterator begin() const noexcept {
        _load_vec_cache();
        return _internal_vec.begin();
    }

    template<typename T>
    constexpr bool contains(const T& item) const
    {
        _load_vec_cache();
        return std::find(_internal_vec.begin(), _internal_vec.end(), item) != _internal_vec.end();
    }

    const ITEM_T& item() const
    {
        _load_cache();
        return _item_cache;
    }

private:
    ITEM_T_SPLITTER _item_splitter;
    mutable vect_t _internal_vec;
    mutable ITEM_T _item_cache;
    mutable bool _set_cache = false;
    mutable bool _set_vec_cache = true;

    constexpr void _load_cache() const
    {
        if (!_set_cache) {
            for (const auto &item : _internal_vec) {
                _item_cache += item;
            }
            _set_cache = true;
        }
    }
    constexpr void _load_vec_cache() const {
        if (!_set_vec_cache) {
            _internal_vec = _item_splitter(_item_cache);
            _set_vec_cache = true;
        }
    }
};

using CtStringSplittable = CtSplittable<Glib::ustring>;
