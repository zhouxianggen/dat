
#include <sys/time.h>
#include "utils.h"
#include "dat.h"
using namespace ucltp;

int main(int argc, char** argv)
{
  struct timeval sTime, eTime;
  gettimeofday(&sTime, NULL);

  Dat trie;
  int re = trie.build(argv[1]);
  if (re != 0) {
    printf("trie.build failed with %d.\n", re);
    return -1;
  }
  printf("Done!, Compression Ratio: %f\n", 100.0*trie.nonzero_size()/trie.size());
  trie.save("bin");
  trie.load("bin");

  vector<char_t> chars;
  read_utf8_text(argv[2], chars);
  
  match_result_t r = trie.match(chars, 0);
  //match_result_t r = trie.match(argv[2]);
  printf("match result is, len=%d, value=%d\n", r.len, r.value);

  gettimeofday(&eTime, NULL);
  //long exeTime = (eTime.tv_sec-sTime.tv_sec)*1000000+(eTime.tv_usec-sTime.tv_usec);
  long exeTime = (eTime.tv_sec-sTime.tv_sec)*1000+(eTime.tv_usec-sTime.tv_usec)/1000;
  printf("exeTime = %d ms\n", exeTime);
  
  return 0;
}
