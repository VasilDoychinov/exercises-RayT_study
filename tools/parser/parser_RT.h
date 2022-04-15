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
        int             _fh ;           // To be atomic

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
        
        const std::string& name() const& { return(_fname) ; }
        const size_t       size() const { return(_sections.size()) ; }

        std::ostream& show_sections(std::ostream& os) const ;

        // Data scopes
        cl_sectionUnit data(const std::string& scope, const std::string& name) const ;
        cl_sectionUnit data(_scope_iter it, const std::string& name) const ;
        iterator begin_scope(const std::string& s_name) const { return(_scope_iter(_sections, s_name)) ; }
        //iterator end_scope(const std::string& s_name) const { _scope_iter i(_sections,s_name);return(i.limit());}

        data_iterator begin_data(const cl_sectionUnit& su, int ss) const { return(_data_iter(_fh, su, ss)) ; }
        data_iterator end_data(const cl_sectionUnit& su) const { _data_iter t(-1, su, 0) ; return(t.limit()) ; }
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

// miscellaneous functions

long _f_read(int fh, long pos, void *buff, size_t count) ;
long move_to(int fh, const std::string& list_delim, unsigned char& delim) ;
long move_to(int fh, const unsigned char delim) ;


#endif
// eof parser_RT.h