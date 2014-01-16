/*
  Double Array Trie
  change from "http://chasen.org/~taku/software/darts/#download"
  Copyright(C) 2013-2014 <zhouxg@ucweb.org>
*/
#ifndef UCLTP_DAT_H_
#define UCLTP_DAT_H_

#include <vector>
using std::vector;
#include "common.h"

namespace ucltp {

class Dat {
public:
  match_result_t match(const char* str);
  match_result_t match(const vector<char_t>& chars, int start);
  
  int build(const char* fdict);
  int load(const char *file);
  int save(const char *file);
  
  size_t nonzero_size() const {
    size_t result = 0;
    for (size_t i = 0; i < _size; ++i)
      if (_array[i].check) ++result;
    return result;
  }
  size_t size() { return _size; }

  explicit Dat(): _size(0), _array(0), _used(0),
    _error(0), _mmaped(false) {}
  virtual ~Dat() { clear(); }
  
private:
  struct entry_t {
    string key;
    int    value;
    bool operator <(const entry_t &e2) const {
      return key < e2.key;
    }
    entry_t(const string& k="", int v=0) :
      key(k), value(v) {}
  };
  
  struct node_t {
    uchar   code;
    size_t  depth;
    size_t  left;
    size_t  right;
  };

  struct unit_t {
    int     base;
    size_t  check;
  };

  unit_t*         _array;
  size_t          _size;
  uchar*          _used;
  size_t          _next_check_pos;
  size_t          _progress;
  int             _error;
  bool            _mmaped;
  
  void clear();
  int build(const vector<entry_t>& entries);
  size_t resize(const size_t new_size);
  size_t fetch(const vector<entry_t>& entries, const node_t& parent, vector<node_t>& siblings);
  size_t insert(const vector<entry_t>& entries, const vector<node_t>& siblings);
};

}
#endif
 
