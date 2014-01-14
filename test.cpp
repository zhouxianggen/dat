
#include <stdio.h>
#include <fstream>
#include <string>
using namespace std;
#include <time.h>
#include "common.h"
#include "datrie.h"
using namespace ucltp;

DATrie trie;

void read(const string &ln, vector<uint16> &codes, vector<string> &chars)
{
  const char *text = ln.c_str();
  uint32 len, code, i;
  codes.clear();
  chars.clear();
  for (i=0; getu8char(text+i, len, code); i+=len) {
    codes.push_back(code);
    chars.push_back(string(text+i, len));
  }
}

string join(const vector<string> &chars, int start, int end)
{
  string re;
  for (int i=start; i<end; i+=1) re += chars[i];
  return re;
}

void test(const string &ln)
{
  vector<uint16> codes;
  vector<string> chars;
  read(ln, codes, chars);
  printf("%s\n", join(chars, 0, chars.size()).c_str());
  for (int i=0,s=codes.size(); i<s; i+=1) {
    DATrie::result_t r = trie.max_forward_match(codes, i);
    if (r.len)
      printf("%s\n\n", join(chars, i, r.len).c_str());
    else
      printf("match none\n\n");
  }
}

int main (int argc, char **argv)
{
    const  char *usage = "argv: dict, test";
    if (argc != 3) {
      printf("%s\n", usage);
      return -1;
    }
    
    vector<vector<uint16> > words;
    vector<int> values;
    ifstream fi(argv[1]);
    string line;
    int idx = 1;
    while (getline(fi, line)) {
        uint32 len, code, i;
        vector<uint16> codes;
        vector<string> chars;
        read(line, codes, chars);
        words.push_back(codes);
        values.push_back(idx++);
    }
    fi.close();

    clock_t start=clock(), finish;
	int re = trie.build(words, values);
	if (re != 0) {
		printf("trie.build failed with %d\n", re);
		return -1;
	}
    finish = clock();
    double duration = (double)(finish-start)/CLOCKS_PER_SEC;
    printf("build ok with %f seconds\n", duration);

    //test(argv[2]);
    return 0;
	
    re = trie.save("bin");
	if (re != 0) {
		printf("trie.save failed with %d\n", re);
		return -1;
	}
    
	re = trie.load("bin");
	if (re != 0) {
		printf("trie.load failed with %d\n", re);
		return -1;
	}

    test(argv[2]);
    return 0;
}


