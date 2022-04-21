// parser_RT.cpp: to parse a scene file
//		- parse_first(): generates a seq of sections. 
//		  in case of an error: 
//			- returns the sequence as is: possible analisys or,
//			- throws std::range_error() if abrupt ...
//		- iterators:
//			- scope: constructor, operator++
//			- data: construced to be used within cl_SceneDescr -> all checks and resources necessary are
//					to be provided before calling
//		- auxiliary: 
//			- move_to() family: read from the current position until eof or a delimiter found
//

#include <utility>

#include "parser_RT.h"

using std::cout ;
using std::endl ;

																		// cl_sectionUnit
std::ostream&
operator <<(std::ostream& os, const cl_sectionUnit& su)
{
	os << "(" << su._name << "[" << su._addr_start << ", " << su._addr_end << "])" ;
	return(os) ;
} // cl_sectionUnit operator <<
																		// eoc cl_sectionUnit
					
																		// iterators
_scope_iter::_scope_iter(const std::vector<cl_sectionUnit>& ss, const std::string& scope)
							: _scn{scope}, _it{cbegin(ss)}, _it_end{cend(ss)}
{
	size_t	pos ;
	for ( ; _it != cend(ss) ; ++_it) {
		if ((pos = _it->_name.find(scope)) != std::string::npos) {
			// check for further no-named levels of inheritance: only 1 '/' that's it, more of '/'
			if ((_it->_name.find("/", pos + scope.size() + 1)) == std::string::npos)	break ;
		}
	} 
	if (_it == _it_end)	return ;	// no match
											// cout << endl << "___ _it->" << *_it << " for scope: " << scope ;
	for (--_it_end ; _it_end != _it ; --_it_end)  {
		if ((_it_end->_name.find(scope)) != std::string::npos) 		break ; // the outer will be last
	}
											// cout << endl << "___ _it_end->" << *_it_end  ;
	// _it: first occurence or cedn(ss)
	// _it_end: after the last matching or cend(ss)
	return ;
} // _scope_iterator ()

_scope_iter&
_scope_iter::operator ++()
{
	size_t   pos ;
	for ( ++_it ; _it != _it_end ; ++_it) {
		if ((pos = _it->_name.find(_scn)) != std::string::npos) {
			if ((_it->_name.find('/', pos + _scn.size() + 1)) == std::string::npos)	break ;
		}
	}
	return(*this) ;
} // _scope_iter operator ++


_data_iter::_data_iter(int fh, const cl_sectionUnit& su, int ss)
			: _fh{fh}, _addr_c{su._addr_start + 1L}, _s_unit{su}, _data{}, _step_size{ss}
{
	if (_fh < 0)			return ;		// that's it

	if (_s_unit != cl_sectionUnit{}) {
		assert(_addr_c < su._addr_end) ;
		if (_lseek(_fh, _addr_c, SEEK_SET) <= 0)	throw std::runtime_error("___ descriptor: () position error") ;
	}
} // _data_iter()

_data_iter&
_data_iter::operator ++() // move _fh after defined _step_size (in delimiters)
{
	if (_addr_c >= _s_unit._addr_end)			throw std::runtime_error("___ position out of scope") ;

	unsigned char   c_buff{} ;
	int				count{0} ;

	long addr = _lseek(_fh, _addr_c, SEEK_SET) ;
	for ( ; addr < _s_unit._addr_end ; addr++) {
		if (_read(_fh, &c_buff, 1) != 1)		throw std::runtime_error("___ descriptor: read error") ;
		if (c_buff == _delim && (++count == _step_size))	break ;
	}

	_addr_c = (addr < _s_unit._addr_end) ? addr + 1 : addr ; // where _fh is
	return(*this) ;
} // _data_iter operator ++

_data_iter::value_type
_data_iter::operator *() 
{
	long			addr{-1L} ;
	unsigned char	c_buff{} ;

	if (!(_s_unit != cl_sectionUnit{}))     return(value_type{}) ;

	if (_lseek(_fh, _addr_c, SEEK_SET) < 0L)	throw std::runtime_error("___ descriptor: * position error") ;
	for (addr = _addr_c ; addr < _s_unit._addr_end ; addr++) {
		if (_read(_fh, &c_buff, 1) != 1)		throw std::runtime_error("___ descriptor: read error") ;
		if (std::isgraph(c_buff))   break ;
	}
	_data.clear(), _data.push_back(c_buff) ;

	int				count{0} ;
	for (addr++ ; addr < _s_unit._addr_end ; addr++) {
		if (_f_read(_fh, addr, &c_buff, 1) != 1)		throw std::runtime_error("___ descriptor: read error") ;
		if (std::isgraph(c_buff))		_data.push_back(c_buff) ;

		if (c_buff == _delim && (++count == _step_size))  break ;
	}

	// move _fh back to the iteration point
	if (_lseek(_fh, _addr_c, SEEK_SET) <= 0)	throw std::runtime_error("___ descriptor: ** position error") ;
	return (std::move(_data)) ; 
} // _data_iter operator *
																		// eoc iterators
																		
																		// class cl_SceneDescr
cl_sectionUnit
cl_SceneDescr::data(const std::string& scope, const std::string& vn) const
{
	auto iter = begin_scope(scope) ;
	auto iter_e = iter.limit() ; //end_scope(scope) ;

	for ( ; iter != iter_e ; ++iter)   {																											
		if ((iter->_name).find(vn) != std::string::npos)   return(*iter) ;
	}
	return(cl_sectionUnit{}) ;
} // cl_SceneDescr data()

cl_sectionUnit
cl_SceneDescr::data(_scope_iter s_iter, const std::string& vn) const
{
	return(data(s_iter.id(), vn)) ;
} // cl_SceneDescr data()

std::vector<long>
cl_SceneDescr::index_data(data_iterator iter)
{
	std::vector<long>		v_addr{} ;

	for (auto iter_e = iter.limit() ; iter != iter_e ; ++iter) {
		v_addr.push_back(std::move(iter.addr())) ;
	}

	return(v_addr) ;
} // cl_SceneDescr data()


bool 
cl_SceneDescr::activate()
{
	if (_fh == -1) {
		errno_t err = _sopen_s(&_fh, _fname.c_str(), _O_BINARY | _O_RDONLY,
							   _SH_DENYRW, _S_IREAD | _S_IWRITE) ;
		if (err != 0) { _fh = -1 ; return(false) ; }
		if (_sections.empty())		_sections = this->_check(0L, "") ;
		if (_sections.empty())	{ _close(_fh), _fh = -1 ; return(false) ; }
	}
	return(true) ;
} // cl_SceneDescr activate()

std::ostream&
cl_SceneDescr::show_sections(std::ostream& os) const //,const std::vector<cl_sectionUnit>& ss)
{
	if (_fh == -1)    { os << endl << "--- <" << _fname << "> not active" ; return(os) ;}

	unsigned char open_ch{} ;			// consistency check: must match and be either '[]' or '{}'
	unsigned char close_ch{} ;			// except for _data fields(scopes): " and ,/}/]

	os << endl << "- # of sections found: " << _sections.size() ;
	for (auto s : _sections) {
		os << endl << "--- " << s._name ;
		_f_read(_fh, s._addr_start, &open_ch, 1), _f_read(_fh, s._addr_end, &close_ch, 1) ;
		os << open_ch << s._addr_start << ", " << s._addr_end << close_ch ;
	}

	return(os) ;
} // cl_SceneDescr show_sections()

std::vector<cl_sectionUnit>
cl_SceneDescr::_check(long from_pos, const std::string& name_parent, unsigned char start_ch)
{
	if (_fh == -1L)		throw std::runtime_error("___ <" + _fname + "> not active") ;

	unsigned char	c_buff{} ;			// to read a byte into

	std::vector<cl_sectionUnit>		sections{} ;		// fill it
	cl_sectionUnit					w_sec{} ;			// section processed

	bool							fl_name{false} ;	// if a name's been found

	long	addr{from_pos} ;
	long	addr_eof{_lseek(_fh, 0L, SEEK_END)} ;
	
	if (addr_eof <= 1L)					return(sections) ;
	// assert(addr < addr_eof) ;
																		// cout << endl << "___ <" << _fname 
																		// << "> within: [" << addr << ", " 
																		// << addr_eof << "]" ;
	if (_lseek(_fh, addr, SEEK_SET) == -1L) {
		sprintf_s(_wbuff, "___ file id: %d<%s>: position error", _fh, _fname.c_str()) ;
		throw std::runtime_error(_wbuff) ;
	}
	
	static int noname_counter{0} ;
	if (addr == 0L)   noname_counter = 0 ;

	for ( ; addr < addr_eof ; addr++) {
		if (_read(_fh, static_cast<unsigned char *>(&c_buff), 1) != 1)	break ;

		if (c_buff == '\"') {		// delimiter linked to a name
			long	w_addr = addr ;
			// move to end of name 
			if ((addr = move_to(_fh, '\"')) == -1L)		throw std::runtime_error("___ parse: missing \"") ;
			if (addr + 1 - w_addr >= sizeof(_wbuff))	throw std::runtime_error("___ parse: name_size > 512b") ;

			long	name_len = (addr + 1) - w_addr ;
			if (_f_read(_fh, w_addr, _wbuff, name_len) != name_len)
														throw std::runtime_error("___ parse: read error") ;
			_wbuff[name_len] = 0 ;  // the terminating '\0'
			fl_name = true ;		// indicate: straight away might be a data section ending with closing ]/}
			// move to opening of a new section or, to closing of an old one
			if ((addr = move_to(_fh, "{[]},", c_buff)) == -1L)
														throw std::runtime_error("___ parse: wrong format") ;
			if (c_buff == '{' || c_buff == '[') {
				fl_name = false ;  // clear
				w_sec._name = name_parent + '/' + std::string(_wbuff) ;

				w_sec._addr_start = addr ;

				// recursion
				std::vector<cl_sectionUnit>		sect_ancest{this->_check(++addr, w_sec._name, c_buff)} ;
				// back from recursion
				sections.insert(end(sections), begin(sect_ancest), end(sect_ancest)) ;
				addr = _lseek(_fh, 0L, SEEK_CUR) - 1L ; // just after the closing '} or ]'
				if (addr < 0L)							throw std::runtime_error("___ parse: position error") ;

				w_sec._addr_end = addr ;
				sections.push_back(std::move(w_sec)), w_sec = cl_sectionUnit{} ;
			} else if ((c_buff == ']' && start_ch == '[') || (c_buff == '}' && start_ch == '{')) {
				if (fl_name) {
					fl_name = false ;
					w_sec._name = name_parent + '/' + std::string(_wbuff) ;
					w_sec._addr_start = w_addr, w_sec._addr_end = addr ;
					sections.push_back(std::move(w_sec)), w_sec = cl_sectionUnit{} ;
					// now we break UP
				}
				break ;
			} else if (c_buff == ',') {
				fl_name = false ;
				w_sec._name = name_parent + '/' + std::string(_wbuff) ;
				w_sec._addr_start = w_addr, w_sec._addr_end = addr ;
				sections.push_back(std::move(w_sec)), w_sec = cl_sectionUnit{} ;
			}
		} else if (c_buff == '{' || c_buff == '[') {		// opening: go DOWN - but NO NAME														
			w_sec._name = name_parent + '/' + std::to_string(noname_counter++) ; // + "" ;
			w_sec._addr_start = addr ;

			std::vector<cl_sectionUnit>		sect_ancest{this->_check(++addr, w_sec._name, c_buff)} ;
			sections.insert(end(sections), begin(sect_ancest), end(sect_ancest)) ;
			addr = _lseek(_fh, 0L, SEEK_CUR) - 1L ; // just after the closing '} or ]'
			if (addr < 0L)							throw std::runtime_error("___ parse: position error") ;
			w_sec._addr_end = addr ;
			sections.push_back(std::move(w_sec)), w_sec = cl_sectionUnit{} ;
		} else if (c_buff == '}')	{   // if closing: go UP
			if (start_ch == '{')	break ;   // go up
		} else if (c_buff == ']') {   // closing: go UP
			if (start_ch == '[')	break ;   // go up
		}
	}
	
	return(sections) ;	// the so far initiated -> could use it in case of an error as well
} // cl_SceneDescr:: _check()


std::vector<std::string>
extract_data(cl_SceneDescr& scene,              // the Scene
			 const std::string& scope,          // the Scope
			 const std::string& name,           // variable(s) section
			 int ss)                            // step size of ... in ','s
{
	std::vector<std::string>    values{} ;
	cl_sectionUnit              s_unit{scene.data(scope,name)} ;

	if (s_unit != cl_sectionUnit{})	{
		auto iter = scene.begin_data(s_unit, ss) ;
		auto iter_e = iter.limit() ;

		for (int i = 0 ; iter != iter_e ; ++iter, i++) {
			// cout << endl << "> scope<" << iter_b.scope() << "> --- section #" << i << ": " << *iter_b ;
			// cout << endl << "--- found #" << i << ": " << *iter ;
			values.push_back(std::move(*iter)) ;
		}
	}
	return(values) ;
} // friend cl_SceneDescr() extract_data()
																		// eoc cl_SceneDescr

																		// Miscellaneous
long
_f_read(int fh, long pos, void *buff, size_t count)
{
	if (_lseek(fh, pos, SEEK_SET) == -1L)			return(-1L) ;

	int   bw = _read(fh, static_cast<char *>(buff), count) ;
	if (bw == count)								return(count) ;

	return(-1L) ;
} // cl_SceneDescr:: _f_read()

// all move_to() family: read from the current position until eof or a delimiter found
long move_to(int fh, const unsigned char ch)   // read from the current position
{
	char   c_buff{} ;
	for (long addr = _lseek(fh, 0L, SEEK_CUR) ; ; addr++) {
		if (_read(fh, static_cast<char *>(&c_buff), 1) != 1)	break ;
		if (c_buff == ch)   return(addr) ;
	}
	return(-1L) ;
} // move_to()

long move_to(int fh, const std::string& list_delim, unsigned char& delim )
{ // place the found in delim
	char   c_buff{} ;
	for (long addr = _lseek(fh, 0L, SEEK_CUR) ; ; addr++) {
		if (_read(fh, static_cast<char *>(&c_buff), 1) != 1)	break ;
		if (list_delim.find(c_buff) != std::string::npos) {	delim = c_buff ; return(addr) ; }
	}
	return(-1L) ;
} // move_to(list)
																		// end of Miscellaneous

// eof parser_RT.cpp