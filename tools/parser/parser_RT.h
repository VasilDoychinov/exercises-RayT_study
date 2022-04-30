// parser_RT.h: detypes and declarations for: parsing a scene file
//      - file interface based on Low Level I/O
//      Major types:
//      - cl_sectionUnit: defines a section:: {name, addrS, addrE}
//      - cl_Scene Descr: link to a descriptor file
//      - scope_iterator: see iterators_RT.h
//      - data_interator: ...
//                         indexing of data iterators defined to speed up triangles constructing
//      NB: the interface implemented through the am iterators
//


#ifndef _DEFS_PARSER_RT_SCENE
#define _DEFS_PARSER_RT_SCENE

#include <iostream>
#include <assert.h>

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>

#include <string>
#include <vector>

#include "../geometry/vectors_RT.h"
#include "../geometry/triangles_RT.h"

struct cl_sectionUnit {
    using iterator = class _iterator ;

    // fields
        std::string     _name{} ;
        long            _addr_start{} ;     // the address of {/[
        long            _addr_end{} ;       // the address of }/]

    // methods
        cl_sectionUnit() : _name{}, _addr_start{-1L}, _addr_end{-1L} {}
        // all special members = default (for now)

        bool operator !=(const cl_sectionUnit& su) { 
            return _name != su._name || _addr_start != su._addr_start || _addr_end != su._addr_end ;
        }

    // Friends
        friend class _scope_iter ;
        friend class _data_iter ;

        friend std::ostream& operator <<(std::ostream& os, const cl_sectionUnit& su) ;
}; // struct cl_sectionUnit

#include "iterators_RT.h"

class cl_SceneDescr {
    public:
        using iterator = _scope_iter ;
        using data_iterator = _data_iter ;

    private:
        std::string     _fname{} ;
        int             _fh ; 

        std::vector<cl_sectionUnit> _sections{} ;
        char                        _wbuff[512] = {0,} ; // just a buffer to read into

    private:
        std::vector<cl_sectionUnit> _check(long fm, const std::string& name_parent, unsigned char sc = 0) ;

    public:
        
    public:
        virtual ~cl_SceneDescr() { deactivate() ; }
        explicit cl_SceneDescr(const std::string& name) : _fname{name}, _fh{-1L}, _sections{}, _wbuff{0,} {}

        // file operations:
        bool activate() ;
        void deactivate() { if (_fh != -1) { _close(_fh), _fh = -1L ; } }
        
        const std::string& name() const& { return _fname ; }
        const size_t       size() const { return _sections.size() ; }

        std::ostream& show_sections(std::ostream& os) const ;

        // Data scopes
        cl_sectionUnit data(const std::string& scope, const std::string& name) const ;
        cl_sectionUnit data(_scope_iter it, const std::string& name) const ;
        iterator begin_scope(const std::string& s_name) const { return _scope_iter(_sections, s_name) ; }
        //iterator end_scope(const std::string& s_name) const { _scope_iter i(_sections,s_name);return(i.limit());}

        data_iterator begin_data(const cl_sectionUnit& su, int ss) const { return _data_iter(_fh, su, ss) ; }
        data_iterator end_data(const cl_sectionUnit& su) const { _data_iter t(-1, su, 0) ; return t.limit() ; }
        std::vector<long> index_data(data_iterator) ;

        // Friends
        friend std::vector<std::string> extract_data(cl_SceneDescr& scene,
                                                     const std::string& scope, 
                                                     const std::string& name, 
                                                     int ss);

        friend class _scope_iter ;
        friend class _data_iter ;
}; // class cl_SceneDescr

// Friend
std::vector<std::string> extract_data(cl_SceneDescr& scene, 
                                      const std::string& scope, const std::string& name, 
                                      int ss);

// convertion functions
template <typename T> vector_RT<T>
vector_from_string(const std::string& str)  // there has to be two ',': defining three numbers 
{
    T       mark[3] = {{},} ;
                                                                            // cout << endl << endl 
                                                                            // << "___ in vector from_string("
                                                                            // << str << ")" ;
    try {
        size_t  i = 0 ;
        size_t  count = 0 ;
        for (size_t j = 0 ; j < str.size() && count < 2 ; j++) {
            if (str[j] == ',') {
                mark[count ++] = static_cast<T>(std::stod(str.substr(i, j - i))),
                    i = j + 1 ;
            }
        }
        if (count < 2)      throw std::runtime_error("___ bad vector descriptor") ;
        mark[2] = static_cast<T>(std::stod(str.substr(i))) ;
    } catch (...) { throw ; }

    return vector_RT<T>(mark[0], mark[1], mark[2]) ;
} // vector_from_string()

template <typename T> T
number_from_string(const std::string& str)  // there has to be a ':' - the number starts after it
{
    T       number{0} ;
    size_t  pos = 0 ;

    if ((pos = str.find(":")) != std::string::npos) {
        number = static_cast<T>(std::stod(str.substr(pos + 1))) ;
    }
    return number ;
} // number_from_string()

template <typename T, typename RGB> RGB
color_from_string(const std::string& str)
{
    vector_RT<T> temp{vector_from_string<T>(str)} ; // get the indexes
    // temp holds the indexes of triangle's vertices in indexes
    return RGB{temp[0], temp[1], temp[2]} ;
} // color_from_string()


#ifdef NO_INDEXING

#include <algorithm>

template <typename T> triangle_RT<T>
triangle_from_iterator(_data_iter& it_tr, _data_iter& begin_vertices)  // it: assumed to be (ui,ui,ui)
{
    // cout << endl << "--- in triangle_from_ITER (i(" << *it_tr << "), at v0{" << *begin_vertices << "})" ;
    vector_RT<unsigned int> temp{vector_from_string<unsigned int>(*it_tr)} ; // get the indexes

    std::pair<size_t, size_t>    vvv[3] = {std::pair<size_t,size_t>(0, temp[0]),
                                            std::pair<size_t,size_t>(1, temp[1]),
                                            std::pair<size_t,size_t>(2, temp[2])
    } ;
    std::sort(begin(vvv), end(vvv), [](std::pair<size_t, size_t> i, std::pair<size_t, size_t> j)->bool
                {return i.second < j.second ; }) ;
    // vvv holds the indexes in "vertices" in ascending order 

    // The vertice indexes are supposedly: sorted here.
    auto            iter = begin_vertices ; auto iter_end = iter.limit() ;
    size_t          ind_vertice = 0 ;
    vector_RT<T>    vertices[3] = {vector_RT<T>{}, vector_RT<T>{}, vector_RT<T>{}} ;

    for (int i = 0 ; i < _countof(vvv) ; i++) {
        // cout << endl << "--- loading vertice:" 
        // << vvv[i].second << 
        // " for triangle descr.: " << temp ;
        // fwd_iterator: position it to the current vertice index
        for (; ind_vertice < (vvv[i]).second && iter != iter_end ; ++iter, ind_vertice++) ;
        // load the vector      
        vertices[(vvv[i]).first] = std::move(vector_RT<T>(vector_from_string<T>(*iter))) ;
    }

    return triangle_RT<T>{vertices[0], vertices[1], vertices[2]} ;
} // triangle_from_iterator()
#endif

template <typename T> std::pair<triangle_RT<T>, vector_RT<unsigned int>>
triangle_from_indexes(_data_iter& it_tr, std::vector<long>& indexes)  // it_tr: assumed to be (ui,ui,ui)
{
    // cout << endl << "--- in triangle_from_ITER (i(" << *it_tr << "), at v0{" << *begin_vertices << "})" ;

    vector_RT<unsigned int> temp{vector_from_string<unsigned int>(*it_tr)} ; // get the indexes
    // temp holds the indexes of triangle's vertices in indexes

    vector_RT<T>    vertices[3] = {vector_RT<T>{}, vector_RT<T>{}, vector_RT<T>{}} ;

    for (int i = 0 ; i < 3 ; i++) {
        auto iter = std::move(it_tr + indexes[temp[i]]) ; // a bit of cheating but _iters are of same scope
        vertices[i] = std::move(vector_from_string<T>(*iter)) ;
    }

    return std::pair<triangle_RT<T>, vector_RT<unsigned int>>
                    (triangle_RT<T>{vertices[0], vertices[1], vertices[2]}, temp) ;
} // triangle_from_indexes()


// miscellaneous functions
long _f_read(int fh, long pos, void *buff, size_t count) ;
long move_to(int fh, const std::string& list_delim, unsigned char& delim) ;
long move_to(int fh, const unsigned char delim) ;


#endif
// eof parser_RT.h