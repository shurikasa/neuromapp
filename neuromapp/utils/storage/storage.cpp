/*
 * Neuromapp - storage.cpp, Copyright (c), 2015,
 * Timothee Ewart - Swiss Federal Institute of technology in Lausanne,
 * Cremonesi Francesco - Swiss Federal Institute of technology in Lausanne,
 * Sam Yates - Swiss Federal Institute of technology in Lausanne,
 * timothee.ewart@epfl.ch,
 * francesco.cremonesi@epfl.ch
 * sam.yates@epfl.ch
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

/**
 * @file neuromapp/utils/storage/storage.cpp
 * \brief implementation of the storage class
 */

#include "utils/storage/storage.hpp"
extern "C" {
#include "utils/storage/storage.h"
}
#include "utils/storage/neuromapp_data.h"

#include <iostream>

/**
 * destructor of the storage class
 */
storage::~storage() {
    for (storage_map::iterator i=M.begin();i!=M.end();++i) i->second.destroy();
}

/**
 * clear item
 */
void storage::clear(std::string const &name) {
    storage_map::iterator it = M.find(name);
    if (it!=M.end()) {
        it->second.destroy();
        M.erase(it);
    }
}

/** \class ref_count_ptr
 * \brief Reference-counted managed pointer.
 *
 * ref_count_ptr is either in an 'empty' state, or owns a pointer and
 * positive reference count.
 *
 * Semantics are similar to C++11 shared_ptr<T>: copies increment the
 * reference count; deletions decrement the count and will call the
 * registered destructor function on the owned pointer when the count
 * reaches zero.
 *
 * Not thread-safe.
 */
class ref_count_ptr {
public:
    ref_count_ptr(): ptr(0), dtor(0), k(0) {}

    ref_count_ptr(const ref_count_ptr &r): ptr(r.ptr), dtor(r.dtor), k(r.k) {
        if (*this) ++*k;
        assert_invariant();
    }

    ref_count_ptr(void *ptr_, void (*dtor_)(void *)): ptr(ptr_), dtor(dtor_), k(0) {
        if (ptr_) k=new size_t(1);
        assert_invariant();
    }

    ref_count_ptr &operator=(const ref_count_ptr &r) {
        if (this==&r) return *this;
        reset();
        ptr=r.ptr;
        dtor=r.dtor;
        k=r.k;
        if (*this) ++*k; // call operator bool() test if k != 0
        assert_invariant();
        return *this;
    }

    // true if not in empty state.
    operator bool() const { return k; }

    // return 0 if empty
    void *get() {
        assert_invariant();
        return (*this)?ptr:0;
    }

    void reset() {
        // If I am empty return
        if (!*this) return;

        if (!--*k) { /* --*k == 0 */
            dtor(ptr); // clean up memory
            delete k;
            k=0;
            ptr=0;
        }
        assert_invariant();
    }

    ~ref_count_ptr() {
        reset();
        assert_invariant();
    }

private:
    void assert_invariant() {
        // must be either in empty state, with k zero,
        // or k must be non-zero and *k>0, and ptr!=0.
        if (k) {
            //(test *k==0)
            if (!*k) throw std::logic_error("ref_count_ptr: k!=0 but *k==0");
            if (!ptr) throw std::logic_error("ref_count_ptr: k!=0 but ptr==0");
        }
    }

    void *ptr;
    void (*dtor)(void *);
    size_t *k;
};


/** \struct storage_ctor_wrapper
 * \brief Functional object to construct ref_count_ptr owning the
 * result of calling the provided maker function.
 */
struct storage_ctor_wrapper {
    storage_ctor ctor;
    storage_ctor_context context;
    storage_dtor dtor;

    ref_count_ptr operator()() {
        return ref_count_ptr(ctor(context),dtor);
    }
};

/**
 * Gets a pointer to the data. If data has been already loaded, returns a pointer to the existing data (not a clone).
 * @param name keyword referring to the data (user-defined)
 * @param maker pointer to function that loads the data, if not internally existing
 * @param context parameters to pass to the maker function above (if function not called, not used, NULL can be past)
 * @param destroyer function pointer that will delete the data;
 * @return pointer to the data
 */
void *storage_get(const char *name, storage_ctor maker,
                  storage_ctor_context context, storage_dtor dtor)
{
   storage_ctor_wrapper mk = {maker, context, dtor};
   return neuromapp_data.get<ref_count_ptr>(name, mk).get();
};

/**
 * Put new data to a given key
 * @param name keyword referring to the data (user-defined)
 * @param item
 * @param destroyer function pointer that will delete the data;
 */
void storage_put(const char *name, void *item, storage_dtor dtor) {
    neuromapp_data.put_copy(name, ref_count_ptr(item,dtor));
}

/** cleaning the library */
void storage_clear(const char *name) {
    neuromapp_data.clear(name);
}
