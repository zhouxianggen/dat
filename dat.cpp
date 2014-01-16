
#include <algorithm>
#include <string>
using std::string;
using std::sort;
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "utils.h"
#include "dat.h"

namespace ucltp {

template <class T> inline T _max(T x, T y) { return(x > y) ? x : y; }
template <class T> inline T* _resize(T* ptr, size_t n, size_t l, T v) {
  T *tmp = new T[l];
  for (size_t i = 0; i < n; ++i) tmp[i] = ptr[i];
  for (size_t i = n; i < l; ++i) tmp[i] = v;
  delete [] ptr;
  return tmp;
}

match_result_t Dat::match(const char* key)
{
  match_result_t r;
  r.len = r.value = 0;

  if (!key) return r;
    
  register int b = _array[0].base;
  register size_t i = 0, len = strlen(key), p;

  for (; i < len; ++i) {
    //printf("i=%d, c=%d, b=%d, check[b]=%d, base[b]=%d\n", i, key[i], b, _array[b].check, _array[b].base);
    if (_array[b].check == b && _array[b].base < 0) {
      r.len = i;
      r.value = -_array[b].base;
    }
    p = b +(size_t)(uchar(key[i])) + 1;
    //printf("p=%d, check[b]=%d\n", p, _array[p].check);
    if (b == _array[p].check)
      b = _array[p].base;
    else
      return r;
  }

  if (_array[b].check == b && _array[b].base < 0) {
    r.len = i;
    r.value = -_array[b].base;
  }

  return r;
}

match_result_t Dat::match(const vector<char_t>& chars, int start)
{
  match_result_t r;
  r.len = r.value = 0;
    
  register int b = _array[0].base;
  register size_t i = start, len = chars.size(), p;

  for (; i < len; ++i) { 
    for (int j = 0; chars[i]._name[j]; ++j) {
      if (j == 0 && _array[b].check == b && _array[b].base < 0) {
        r.len = i;
        r.value = -_array[b].base;
      }
      p = b +(size_t)((uchar)(chars[i]._name[j])) + 1;
      if (b == _array[p].check)
        b = _array[p].base;
      else
        return r;
    }
  }
  if (_array[b].check == b && _array[b].base < 0) {
    r.len = i;
    r.value = -_array[b].base;
  }

  return r;
}

int Dat::build(const char* fdict)
{
  vector<string> lines;
  read_lines(fdict, lines);
  
  vector<entry_t> entries;
  vector<string> parts;
  for (int i=0; i<lines.size(); i+=1) {
    if (split(lines[i], parts) == 2) {
      entries.push_back(entry_t(parts[0], atoi(parts[1].c_str())));
    }
  }

  if (entries.size() <= 0) return 0;
  sort(entries.begin(), entries.end());
  return build(entries);
}
  
int Dat::build(const vector<entry_t>& entries)
{
  clear();
  
  resize(8192);
  _progress      = 0;
  _array[0].base  = 1;
  _next_check_pos = 0;

  node_t root_node;
  root_node.left  = 0;
  root_node.right = entries.size();
  root_node.depth = 0;
  root_node.code = 0;

  std::vector<node_t> siblings;
  fetch(entries, root_node, siblings);
  insert(entries, siblings);

  return _error;
}

size_t Dat::fetch(const vector<entry_t>& entries, const node_t& parent, vector<node_t> &siblings)
{
  if (_error) return 0;

  uchar prev = 0;

  for (size_t i = parent.left; i < parent.right; ++i) {
    if (entries[i].key.length() < parent.depth) continue;

    uchar cur = (entries[i].key.length() != parent.depth)? entries[i].key[parent.depth]+1 : 0;

    if (prev > cur) {
      _error = -3;
      return 0;
    }

    if (cur != prev || siblings.empty()) {
      node_t tmp_node;
      tmp_node.depth = parent.depth + 1;
      tmp_node.code  = cur;
      tmp_node.left  = i;
      if (!siblings.empty())
        siblings[siblings.size()-1].right = i;

      siblings.push_back(tmp_node);
    }
    prev = cur;
  }

  if (!siblings.empty())
    siblings[siblings.size()-1].right = parent.right;

  return siblings.size();
}

size_t Dat::insert(const vector<entry_t>& entries, const vector<node_t>& siblings)
{
  if (_error) return 0;

  size_t begin = 0;
  size_t pos   = _max((size_t)siblings[0].code + 1, _next_check_pos) - 1;
  size_t nonzero_num = 0;
  int    first = 0;

  while (true) {
  next:
    ++pos;
    begin = pos - siblings[0].code;
    
    if (_size <= (begin + (1 << 8 * sizeof(uchar)))) {
      size_t size1 = begin + (1 << 8 * sizeof(uchar) + 1) + 1;
      size_t size2 = (size_t)_max(1.05, 1.0 * entries.size() / _progress) * _size;
      resize(_max(size1, size2));
    }

    if (_array[pos].check) {
      ++nonzero_num;
      continue;
    } else if (!first) {
      _next_check_pos = pos;
      first = 1;
    }

    if (_used[begin]) continue;

    for (size_t i = 1; i < siblings.size(); ++i)
      if (_array[begin + siblings[i].code].check != 0) goto next;

    break;
  }

  // -- Simple heuristics --
  // if the percentage of non-empty contents in check between the index
  // 'next_check_pos' and 'check' is greater than some constant
  // value(e.g. 0.9),
  // new 'next_check_pos' index is written by 'check'.
  if (1.0 * nonzero_num/(pos - _next_check_pos + 1) >= 0.95)
    _next_check_pos = pos;

  _used[begin] = 1;

  for (size_t i = 0; i < siblings.size(); ++i)
    _array[begin + siblings[i].code].check = begin;

  for (size_t i = 0; i < siblings.size(); ++i) {
    std::vector <node_t> new_siblings;

    if (!fetch(entries, siblings[i], new_siblings)) {
      if (entries[siblings[i].left].value <= 0) {
        _error = -2;
        return 0;
      }
      
      _array[begin + siblings[i].code].base = -entries[siblings[i].left].value;
      ++_progress;
      //printf("%d\r", _progress);fflush(stdout);

    } else {
      size_t h = insert(entries, new_siblings);
      _array[begin + siblings[i].code].base = (int)h;
    }
  }

  return begin;
}

int Dat::load(const char *file)
{
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
  
  _size = size/sizeof(unit_t);  
  _array = (unit_t*)data;
  _mmaped = true;
  close(fd);
  
  return 0;
}

int Dat::save(const char *file)
{
  if (!_size) return -1;
    
  std::FILE *fp = std::fopen(file, "wb");
  if (!fp) return -1;
    
  if (_size != std::fwrite(_array, sizeof(unit_t), _size, fp))
    return -1;
    
  std::fclose(fp);
  return 0;
}

void Dat::clear()
{
  if (_array) {
    if (_mmaped)
      munmap(_array, _size*sizeof(unit_t));
    else {
      delete [] _array;
      delete [] _used;
    }
  }
  _array = 0;
  _used = 0;
  _size = 0;
  _mmaped = false;
}

/*size_t Dat::resize(const size_t new_size)
{
  unit_t tmp;
  tmp.base = 0;
  tmp.check = 0;
  _array = _resize(_array, _size, new_size, tmp);
  _used  = _resize(_used, _size, new_size, static_cast<unsigned char>(0));
  _size = new_size;
  return new_size;
}*/

size_t Dat::resize(const size_t new_size)
{
  void *tmp = calloc(new_size, sizeof(unit_t));
  memcpy(tmp, _array, _size*sizeof(unit_t));
  free(_array);
  _array = (unit_t*)tmp;

  tmp = calloc(new_size, sizeof(uchar));
  memcpy(tmp, _used, _size*sizeof(uchar));
  free(_used);
  _used = (uchar*)tmp;
  
  _size = new_size;
  return new_size;
}

} // end of namespace
 
