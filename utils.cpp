
#include "utils.h"

namespace ucltp {

// get an utf8 char
bool get_utf8_char(const char *u8text, uint32 &len, uint32 &code)
{
  const unsigned char	*p = (unsigned char*)u8text;
  if (!p || !p[0]) return false;
  else if (p[0] <= 0x7F) {
    len = 1;
    code = p[0];
    return true;
  }
  else if (p[0]>=0xC0 && p[0]<=0xDF) len = 2;
  else if (p[0]>=0xE0 && p[0]<=0xEF) len = 3;
  else if (p[0]>=0xF0 && p[0]<=0xF7) len = 4;
  else if (p[0]>=0xF8 && p[0]<=0xFB) len = 5;
  else if (p[0]>=0xFC && p[0]<=0xFD) len = 6;
  else return false;
  code = (p[0] & (0xFF>>(1+len))) << (len-1)*6;
  for (int i=len-1,j=0; i>0; i-=1,j+=6)
    code += (p[i]&0x3F)<<j;
  return true;
}

// read utf8 text to chars
bool read_utf8_text(const char *u8text, vector<char_t>& chars)
{
  uint32 len, code, i;
  if (!u8text) return false;
  chars.clear();
  for (i=0; get_utf8_char(u8text+i, len, code); i+=len) {
    code = u2l(d2s(code));
  if (code <= 0x7F) chars.push_back(char_t(char(code), code)); 
  else chars.push_back(char_t(u8text+i, len, code));
  }
  return !u8text[i];
}

// read file to lines
bool read_lines(const char* file, vector<string>& lines)
{
  ifstream fi(file);
  string line;

  if (!fi) return false;
  lines.clear();
  while (getline(fi, line))
    lines.push_back(line);
  fi.close();

  return true;
}

// split string by space
int split (const string& str, vector<string>& parts)
{
  parts.clear();
  for (int i=0,s=str.length(),j; i<s; i=j) {
    for (; i<s && isspace(str[i]); i+=1);
    for (j=i; j<s && !isspace(str[j]); j+=1);
    if (j > i) parts.push_back(str.substr(i, j-i));
  }
  return parts.size();
}

} // namespace

