// iterators_RT.h: iterators for: parsing a descriptor file
//                 NB: designed to be used within cl_SceneDescr class
//      - scope_iterator: forward_iterator_tag
//              - iterates from {} to {} of a same scope: defined by cl_sectionUnit
//              - operations: constructors, ++, !=, limit(), 
//                            *: returns std:vector<>::const_iterator
//      - data_interator: forward_iterator_tag+ 
//              - iterates within {} by a defined step of delimiters
//              - operations: constructors, ++, !=, limit(), + address: positions it over a field 
//                            *: returns std:string with the data field
//


#ifndef _DEFS_ITER_RT_SCENE
#define _DEFS_ITER_RT_SCENE

#include <string>


class _scope_iter {     // iterates with a scope: from {} to next {}
    public:
        using iterator_category = std::forward_iterator_tag ;
        using value_type = typename cl_sectionUnit ;
        using size_type = typename size_t ;

        using pointer = typename const cl_sectionUnit * ;
        using reference = typename const cl_sectionUnit & ;

    private:
        std::string _scn{} ;

        std::vector<cl_sectionUnit>::const_iterator _it ;
        std::vector<cl_sectionUnit>::const_iterator _it_end ;

    public:
        _scope_iter() : _it{}, _it_end{}, _scn{} { }						// default C.
        _scope_iter(const std::vector<cl_sectionUnit>& ss, const std::string& scope) ;
        // all special members = default

        _scope_iter limit() { _scope_iter temp(*this) ; temp._it = temp._it_end ; return(temp) ; }
        reference operator *() { return ((*_it)) ; }
        pointer   operator ->() { return(&(*_it)) ; }

        _scope_iter& operator ++() ;
        // const _scope_iter operator ++(int) { _scope_iter tmp(*this) ; ++(*this) ; return(tmp) ; }    

        bool operator !=(const _scope_iter& ri) {
            if (_scn != ri._scn)                throw std::range_error("___ scope mismatch") ;
            return(_it != ri._it) ;
        }

        const std::string& scope() const & { return(_scn) ; }
        const std::string& id() const & { return(_it->_name) ; }

        // Friends ...
}; // class _scope_iter


class _data_iter {     // iterates within {} by a defined step
    public:
        using iterator_category = std::forward_iterator_tag ;
        using value_type = typename std::string ;
        using size_type = typename size_t ;

        using reference = typename std::string& ;

    private:
        static constexpr char   _delim = ',' ;
        static constexpr char   _end1 = ']' ;
        static constexpr char   _end2 = '}' ;

    private:
        int                 _fh{-1} ;
        long                _addr_c{-1L} ;
      
        cl_sectionUnit      _s_unit{} ;
        std::string         _data{} ;

        int                 _step_size{0} ; // measured in # of _delim (until _end?)

    public:
        _data_iter() : _fh{-1}, _addr_c{-1L}, _s_unit{}, _data{}, _step_size{0} {} // default C.
        _data_iter(int fh, const cl_sectionUnit& su, int ss) ;
        // all special members = default

        _data_iter limit() { _data_iter temp(*this) ; temp._addr_c = _s_unit._addr_end ; return(temp) ; }

        value_type operator *() ;

        _data_iter& operator ++() ;
        _data_iter  operator +(long addr) { _data_iter t{*this} ; t._addr_c = addr ; return(t) ; }
        //const _data_iter operator ++(int) { _data_iter tmp(*this) ; ++(*this) ; return(tmp) ; }

        bool operator !=(const _data_iter& ri) { return(_addr_c != ri._addr_c) ; }

        long addr() const { return(_addr_c) ; } // &((static_cast<node_VAL<DTy>*>(p_node))->m_val)) ; }

        // Friends ...

}; // class _data_iter

#endif
// eof iterators_RT.h