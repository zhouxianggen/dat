// common difinition

#ifndef UCLTP_COMMON_H_
#define UCLTP_COMMON_H_

#include <string>
using std::string;

namespace ucltp {

typedef  unsigned char  uchar;
typedef  unsigned short  uint16;
typedef  unsigned int  uint32;
typedef  unsigned long long  uint64;

// double char to single char
#define  d2s(u)  (((u)>=0xFF01 && (u)<=0xFF5D)? ((u)-0xFEE0) : (u))

#define  u2l(u)  (((u)>=0x41 && (u)<=0x5A)? ((u)+0x20) : (u))
#define  ischn(u)  ((u)>=0x4E00 && (u)<=0x9FA5)
#define  isnumber(u)  ((u)==0x4E00 || (u)==0x4E8C || (u)==0x4E09 || \
                       (u)==0x56DB || (u)==0x4E94 || (u)==0x516D || \
                       (u)==0x4E03 || (u)==0x516B || (u)==0x4E5D) 
#define  max2(x, y)  ((x)>(y)? (x) : (y))
#define  min2(x, y)  ((x)<(y)? (x) : (y))

// part-of-speech define
#define  POS_a  ((uint64)(0x01) << 0)
#define  POS_b  ((uint64)(0x01) << 2)
#define  POS_c  ((uint64)(0x01) << 4)
#define  POS_d  ((uint64)(0x01) << 6)
#define  POS_e  ((uint64)(0x01) << 8)
#define  POS_g  ((uint64)(0x01) << 10)
#define  POS_h  ((uint64)(0x01) << 12)
#define  POS_i  ((uint64)(0x01) << 14)
#define  POS_j  ((uint64)(0x01) << 16)
#define  POS_k  ((uint64)(0x01) << 18)
#define  POS_m  ((uint64)(0x01) << 20)
#define  POS_n  ((uint64)(0x01) << 22)
#define  POS_nd  ((uint64)(0x01) << 24)
#define  POS_nh  ((uint64)(0x01) << 26)
#define  POS_ni  ((uint64)(0x01) << 28)
#define  POS_nl  ((uint64)(0x01) << 30)
#define  POS_ns  ((uint64)(0x01) << 32)
#define  POS_nt  ((uint64)(0x01) << 34)
#define  POS_nz  ((uint64)(0x01) << 36)
#define  POS_o  ((uint64)(0x01) << 38)
#define  POS_p  ((uint64)(0x01) << 40)
#define  POS_q  ((uint64)(0x01) << 42)
#define  POS_r  ((uint64)(0x01) << 44)
#define  POS_u  ((uint64)(0x01) << 46)
#define  POS_v  ((uint64)(0x01) << 48)
#define  POS_wp  ((uint64)(0x01) << 50)
#define  POS_ws  ((uint64)(0x01) << 52)
#define  POS_x  ((uint64)(0x01) << 54)
// binary=0101010101010101010101010101010101010101010101010101010101010101
#define  POS_MASK  0x5555555555555555

struct char_t {
  string _name;
  uint32 _code;
  uint64 _tag;
  
  void set_tag(uint64 pos, int offset) {
    _tag |= (pos << offset);
  }
  
  char_t(const char *s, int len, uint32 c) : _code(c), _name(s, len) {}
  char_t(char ch, uint32 c) : _code(c), _name(1, ch) {}
  char_t() : _code(0) { _name[0] = '\0'; }
};

struct word_t {
  string _name;
  uint64 _pos;

  word_t(const string& n, uint64 t) : _name(n), _pos(t) {}
};

// convert chars to words
inline void make_words(const vector<char_t>& chars, vector<word_t>& words)
{
  words.clear();
  for (int i=0,j,s=chars.size(); i<s; i=j) {
    uint64 mask = POS_MASK & chars[i]._tag;
    string name = chars[i]._name;
    for (j=i+1; mask; j+=1) {
      mask = ((mask<<1) & chars[j]._tag) >> 1;
      name += chars[j]._name;
    }
    words.push_back(word_t(name, mask));
  }
}

struct match_result_t {
  int len;
  int value;

  match_result_t() : len(0), value(0) {}
};

}
#endif 
