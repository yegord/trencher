/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <trench/config.h>

#include <boost/mpl/int.hpp>

namespace trench { 
    /**
     * Compile-time mapping from class in some hierarchy to its kind.
     * 
     * \tparam T                       Class.
     * \tparam Base                    Base class of the hierarchy that this class belongs to.
     */
    template<class T, class Base>
    class class_kind;
}

/**
 * Macro defining a 'kind' property, its getter, is() and as() template methods.
 * 
 * Must appear in private section of a class.
 * 
 * \param CLASS                                Class this macro is being used in.
 * \param KIND_PROPERTY                        Class.
 */
#define TRENCH_CLASS_WITH_KINDS(CLASS, KIND_PROPERTY)                   \
    public:                                                             \
                                                                        \
    int KIND_PROPERTY() const { return KIND_PROPERTY##_; }              \
                                                                        \
    template<class Class>                                               \
    bool is() const {                                                   \
        return KIND_PROPERTY##_ == class_kind<Class, CLASS>::value;     \
    }                                                                   \
                                                                        \
    template<class Class>                                               \
    Class *as() {                                                       \
        if(!is<Class>()) {                                              \
            return NULL;                                                \
        } else {                                                        \
            return static_cast<Class *>(this);                          \
        }                                                               \
    }                                                                   \
                                                                        \
    template<class Class>                                               \
    const Class *as() const {                                           \
        if(!is<Class>()) {                                              \
            return NULL;                                                \
        } else {                                                        \
            return static_cast<const Class *>(this);                    \
        }                                                               \
    }                                                                   \
                                                                        \
    private:                                                            \
                                                                        \
    int KIND_PROPERTY##_;

/**
 * Defines a compile-time mapping from class to class kind.
 * 
 * Must be used at global namespace.
 * 
 * This macro MUST be invoked for each class that you wish to use with <tt>is</tt>
 * and <tt>as</tt> methods.
 * 
 * \param BASE                         Base class of the hierarchy that this class belongs to.
 * \param CLASS                        Class.
 * \param KIND                         Class kind.
 */
#define TRENCH_REGISTER_CLASS_KIND(BASE, CLASS, KIND)                   \
namespace trench {                                                      \
    template<>                                                          \
    class class_kind<CLASS, BASE>: public boost::mpl::int_<KIND> {};    \
}

/* vim:set et sts=4 sw=4: */
