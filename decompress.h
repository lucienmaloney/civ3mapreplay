#pragma once

#include <cmath>
#include <vector>
#include <iostream>

namespace Civ3 {
  #define BUFFER_LENGTH 4096

  /*
    This code is a copy of a file I wrote in JavaScript (https://civ3complete.com/script/decompress.js)
    which is itself a copy of someone else's Java program (https://forums.civfanatics.com/threads/cross-platform-sav-biq-bix-bic-decompressor.527613/)

    Despite rewriting this class twice in two different languages, I have absolutely no idea how or why it works
    Happy debugging!
  */
  class Decompress {
    char* buffer;
    int bufferlength;
    int elapsed = 0;

    int queue[BUFFER_LENGTH];
    int queuehead;
    int queuetail;
    int queuecount;

    int header;
    int dictionarybits;
    int dictionarysize;

    bool bitstream[BUFFER_LENGTH * 8];
    int inbuffer = 0;
    int bufferoffset = 0;

    int getbit(int, int);
    int setbit(int, int);
    void queueinit();
    void queuepop();
    void queuepush(int);
    int getbits(int);
    int getcopyoffsethighorder();
    int getcopylength();
    int getreversebits(int);
    void decompress();

    public:
      Decompress(char*, int);
      std::vector<char> output;
  };

  Decompress::Decompress(char* input, int length) {
    this->buffer = input - BUFFER_LENGTH;
    this->bufferlength = length;

    queueinit();
    header = getbits(8);
    dictionarybits = getbits(8);
    dictionarysize = 64 << dictionarybits;

    if (header != 0 || dictionarysize > BUFFER_LENGTH) {
      output.push_back('-');
    } else {
      decompress();
    }
  }

  void Decompress::decompress() {
    int x, length, offset, ch;

    for (;;) {
      x = getbits(1);

      if (x == 0) {
        ch = getbits(8);
        output.push_back((char)ch);
        queuepush(ch);
      } else {
        length = getcopylength();
        if (length == 519) {
          break;
        }

        if (length != 2) {
          offset = getcopyoffsethighorder() << dictionarybits;
          offset |= getbits(dictionarybits);
        } else {
          offset = getcopyoffsethighorder() << 2;
          offset |= getbits(2);
        }

        offset %= queuecount;

        if (queuetail >= offset) {
          offset = queuetail - offset;
        } else {
          offset = dictionarysize - (offset - queuetail);
        }

        while (length-- != 0) {
          output.push_back((char)queue[offset]);
          queuepush(queue[offset++]);

          if (offset == dictionarysize) {
            offset = 0;
          }
        }
      }
    }
  }

  int Decompress::getbit(int x, int n) {
    return ((x >> n) & 1);
  }

  int Decompress::setbit(int x, int n) {
    return (x |= (1 << n));
  }

  void Decompress::queueinit() {
    queuehead = 0;
    queuetail = -1;
    queuecount = 0;
  }

  void Decompress::queuepop() {
    queuehead++;
    queuecount--;

    if (queuehead == dictionarysize) {
      queuehead = 0;
    }
  }

  void Decompress::queuepush(int key) {
    if (queuecount == dictionarysize) {
      queuepop();
    }

    if (++queuetail == dictionarysize) {
      queuetail = 0;
    }

    queue[queuetail] = key;
    queuecount++;
  }

  int Decompress::getbits(int n) {
    if (inbuffer == 0) {
      buffer += BUFFER_LENGTH;
      int x = std::min(bufferlength - elapsed, BUFFER_LENGTH);

      if (x == 0) {
        return 0xFFFF;
      }

      for (int i = 0; i < x; i++) {
        for (int j = 0; j < 8; j++) {
          bitstream[(i << 3) + j] = buffer[i] & 1;
          buffer[i] >>= 1;
        }
      }

      inbuffer = (x << 3);
      bufferoffset = 0;
    } else if (inbuffer < n) {
      int i = inbuffer;
      int x = getbits(i);
      return x | (getbits(n - i) << i);
    }

    int x = 0;
    for (int i = 0; i < n; i++) {
      if (bitstream[bufferoffset + i] != 0) {
        x = setbit(x, i);
      }
    }

    inbuffer -= n;
    bufferoffset += n;
    return x;
  }

  int Decompress::getcopyoffsethighorder() {
    if (getbits(1) != 0) {
      if (getbits(1) != 0) {
        return 0x00;
      }

      if (getbits(1) != 0) {
        return (0x02 - getbits(1));
      } else {
        return (0x06 - getreversebits(2));
      }
    }

    if (getbits(1) != 0) {
      int x = getreversebits(4);

      if (x != 0) {
        return (0x16 - x);
      } else {
        return (0x17 - getbits(1));
      }
    }

    if (getbits(1) != 0) {
      return (0x27 - getreversebits(4));
    }

    if (getbits(1) != 0) {
      return (0x2F - getreversebits(3));
    }

    return (0x3F - getreversebits(4));
  }

  int Decompress::getcopylength() {
    if (getbits(1) != 0) {
      if (getbits(1) != 0) {
        return 3;
      } else {
        return ((getbits(1) != 0) ? 2 : 4);
      }
    }

    if (getbits(1) != 0) {
      if (getbits(1) != 0) {
        return 5;
      } else {
        return ((getbits(1) != 0) ? 6 : 7);
      }
    }

    if (getbits(1) != 0) {
      if (getbits(1) != 0) {
        return 8;
      } else {
        return ((getbits(1) != 0) ? 9 : 10 + getbits(1));
      }
    }

    if (getbits(1) != 0) {
      if (getbits(1) != 0) {
        return (12 + getbits(2));
      } else {
        return (16 + getbits(3));
      }
    }

    if (getbits(1) != 0) {
      if (getbits(1) != 0) {
        return (24 + getbits(4));
      } else {
        return (40 + getbits(5));
      }
    }

    if (getbits(1) != 0) {
      return (72 + getbits(6));
    }

    if (getbits(1) != 0) {
      return (136 + getbits(7));
    }

    return (264 + getbits(8));
  }

  int Decompress::getreversebits(int n) {
    int r = 0;
    int x = getbits(n);
    for (int i = 0; i < n; i++) {
      if (getbit(x, n - i - 1) != 0) {
        r = setbit(r, i);
      }
    }

    return r;
  }

};
