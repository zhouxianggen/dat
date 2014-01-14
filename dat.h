/*
  Double-ARray Trie System
  make a little change from "http://chasen.org/~taku/software/darts/#download"
  Copyright(C) 2013-2014 <zhouxg@ucweb.org>
*/
#ifndef UCLTP_DATRIE_H_
#define UCLTP_DATRIE_H_

#include <vector>
using std::vector;
#include <cstring>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

namespace ucltp {

template <class T> inline T _max(T x, T y) { return(x > y) ? x : y; }
template <class T> inline T* _resize(T* ptr, size_t n, size_t l, T v) {
  T *tmp = new T[l];
  for (size_t i = 0; i < n; ++i) tmp[i] = ptr[i];
  for (size_t i = n; i < l; ++i) tmp[i] = v;
  delete [] ptr;
  return tmp;
}

template <class char_t>
class DoubleArrayImpl {
  
public:
  struct result_t {
    size_t  len;
    int     value;
  };
  
  inline result_t max_forward_match(vector<char_t> &key, int start) const {
    result_t result;
    result.len = 0;
    result.value = 0;

    if (!array_) return result;
      
    register int    b = array_[0].base;
    register size_t i, p, len;

    for (i = start, len = key.size(); i < len; ++i) {
      //printf("i=%d, c=%d, b=%d, check[b]=%d, base[b]=%d\n", i, key[i], b, array_[b].check, array_[b].base);
      if (b < size_ && array_[b].check == b && array_[b].base < 0) {
        result.len = i;
        result.value = -array_[b].base;
      }
      p = b +(size_t)(key[i]) + 1;
      //printf("p=%d, check[b]=%d\n", p, array_[p].check);
      if (p < size_ && b == array_[p].check)
        b = array_[p].base;
      else
        return result;
    }

    if (b < size_ && array_[b].check == b && array_[b].base < 0) {
      result.len = i;
      result.value = -array_[b].base;
    }

    return result;
  }
  
  int build(vector<vector<char_t> > &keys, vector<int> &values) {
    if (keys.size() != values.size()) return 0;
    clear();
    
    key_size_      = keys.size();
    progress_      = 0;

    resize(8192);

    array_[0].base  = 1;
    next_check_pos_ = 0;

    node_t root_node;
    root_node.left  = 0;
    root_node.right = key_size_;
    root_node.depth = 0;
    root_node.code = 0;

    std::vector<node_t> siblings;
    fetch(keys, root_node, siblings);
    insert(keys, values, siblings);

    //for (int i=0; i<size_; i+=1)
    //  printf("n%d: %d\t%d\t%d\n", i, array_[i].base, array_[i].check, used_[i]);
    delete [] used_;
    used_ = 0;

    return error_;
  }
  
  int load(const char *file) {
    int	fd = 0;
    off_t size = 0;
    void *data = 0;

    clear();
    
    if (!file || (fd = open(file, O_RDONLY)) == -1)
      return -1;

    if ((size = lseek(fd, 0, SEEK_END)) % sizeof(unit_t) != 0 || 
        !size ||
        (data = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0)) == (void*)-1) {
      close(fd);
      return -2;
    }
    
    size_ = size/sizeof(unit_t);  
    array_ = (unit_t*)data;
    mmaped_ = true;
    close(fd);
    
    return 0;
  }
  
  int save(const char *file) {
    if (!size_) return -1;
    std::FILE *fp = std::fopen(file, "wb");
    if (!fp) return -1;
    if (size_ != std::fwrite(array_, sizeof(unit_t), size_, fp))
      return -1;
    std::fclose(fp);
    return 0;
  }
  
  explicit DoubleArrayImpl(): size_(0), array_(0), used_(0),
                              error_(0), mmaped_(false) {}
  virtual ~DoubleArrayImpl() { clear(); }

private:
  struct node_t {
    char_t  code;
    size_t  depth;
    size_t  left;
    size_t  right;
  };

  struct unit_t {
    int     base;
    size_t  check;
  };

  unit_t          *array_;
  size_t          size_;
  unsigned char   *used_;
  size_t          next_check_pos_;
  size_t          key_size_;
  size_t          progress_;
  int             error_;
  bool            mmaped_;

  void clear() {
    if (array_) {
      if (mmaped_) munmap(array_, size_*sizeof(unit_t));
      else delete [] array_;
    }
    array_ = 0;
    used_ = 0;
    size_ = 0;
    mmaped_ = false;
  }
  size_t resize(const size_t new_size) {
    unit_t tmp;
    tmp.base = 0;
    tmp.check = 0;
    //printf("progress %d/%d, resize from %d to %d\n", progress_, key_size_, size_, new_size);
    array_ = _resize(array_, size_, new_size, tmp);
    used_  = _resize(used_, size_, new_size, static_cast<unsigned char>(0));
    size_ = new_size;
    return new_size;
  }

  size_t fetch(const vector<vector<char_t> > &keys,
               const node_t &parent,
               vector <node_t> &siblings) {
    if (error_) return 0;

    char_t prev = 0;

    for (size_t i = parent.left; i < parent.right; ++i) {
      if (keys[i].size() < parent.depth) continue;

      char_t cur = (keys[i].size() != parent.depth)? keys[i][parent.depth]+1 : 0;

      if (prev > cur) {
        error_ = -3;
        return 0;
      }

      if (cur != prev || siblings.empty()) {
        node_t tmp_node;
        tmp_node.depth = parent.depth + 1;
        tmp_node.code  = cur;
        tmp_node.left  = i;
        if (!siblings.empty()) siblings[siblings.size()-1].right = i;

        siblings.push_back(tmp_node);
      }

      prev = cur;
    }

    if (!siblings.empty())
      siblings[siblings.size()-1].right = parent.right;

    return siblings.size();
  }

  size_t insert(const vector<vector<char_t> > &keys, 
                const vector<int> &values,
                const vector <node_t> &siblings) {
    if (error_) return 0;

    size_t begin = 0;
    size_t pos   = _max((size_t)siblings[0].code + 1, next_check_pos_) - 1;
    size_t nonzero_num = 0;
    int    first = 0;

    while (true) {
    next:
      ++pos;
      begin = pos - siblings[0].code;
      
      if (size_ <= (begin + (1 << 8 * sizeof(char_t)))) {
        resize(begin + (1 << 8 * sizeof(char_t) + 1) + 1);
      }

      if (array_[pos].check) {
        ++nonzero_num;
        continue;
      } else if (!first) {
        next_check_pos_ = pos;
        first = 1;
      }

      if (used_[begin]) continue;

      for (size_t i = 1; i < siblings.size(); ++i)
        if (array_[begin + siblings[i].code].check != 0) goto next;

      break;
    }

    // -- Simple heuristics --
    // if the percentage of non-empty contents in check between the index
    // 'next_check_pos' and 'check' is greater than some constant
    // value(e.g. 0.9),
    // new 'next_check_pos' index is written by 'check'.
    if (1.0 * nonzero_num/(pos - next_check_pos_ + 1) >= 0.95)
      next_check_pos_ = pos;

    used_[begin] = 1;

    for (size_t i = 0; i < siblings.size(); ++i)
      array_[begin + siblings[i].code].check = begin;

    for (size_t i = 0; i < siblings.size(); ++i) {
      std::vector <node_t> new_siblings;

      if (!fetch(keys, siblings[i], new_siblings)) {
        if (values[siblings[i].left] <= 0) {
          error_ = -2;
          return 0;
        }
        
        array_[begin + siblings[i].code].base = -values[siblings[i].left];
        ++progress_;

      } else {
        size_t h = insert(keys, values, new_siblings);
        array_[begin + siblings[i].code].base = (int)h;
      }
    }

    return begin;
  }
};

typedef  DoubleArrayImpl<unsigned short> DATrie;

}
#endif
 
