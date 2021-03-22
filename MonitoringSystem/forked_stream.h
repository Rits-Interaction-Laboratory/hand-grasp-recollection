#ifndef __FORKED_STREAM_HEADER
#define __FORKED_STREAM_HEADER

#include <ostream>

template <class Ch, class Tr=std::char_traits<Ch> >
class basic_forked_ostreambuf : public std::basic_streambuf<Ch,Tr> {
private:
  typedef basic_forked_ostreambuf<Ch, Tr> self_t;
  std::ostream &out1, &out2;

  basic_forked_ostreambuf() {};
  basic_forked_ostreambuf(const self_t &source) {};
  self_t& operator=(const self_t &source) { return *this; };
  std::streampos current_position(const std::ios_base::openmode open_mode) {
    if(out1 && out2) {
      return out1.rdbuf()->pubseekoff(0, std::ios_base::cur, open_mode);
    }
    else {
      return -1;
    }
  };

public:
  basic_forked_ostreambuf(std::ostream &o1, std::ostream &o2):
    out1(o1), out2(o2) {
  };

  virtual ~basic_forked_ostreambuf(void) {
    out1.flush();
    out2.flush();
  };

protected:
  std::streampos seekoff
  (const std::streamoff off, const typename std::ios_base::seekdir dir,
   const typename std::ios_base::openmode open_mode = std::ios_base::out) {
    out2.seekp(off, dir);
    out1.seekp(off, dir);
    return this->current_position(open_mode);
  }

  std::streampos seekpos
  (const std::streampos pos,
   const typename std::ios_base::openmode open_mode = std::ios::out) {
    out2.seekp(pos);
    out1.seekp(pos);
    return this->current_position(open_mode);
  }

  int overflow(int c = Tr::eof()) {
    if(c != Tr::eof()) {
      out1.put(c);
      out2.put(c);
      if(out1 && out2) {
        return 0;
      }
      return Tr::eof();
    }
    return Tr::eof();
  }

  int underflow(void) {
    return Tr::eof();
  }

  int sync() {
    out1.flush();
    out2.flush();
    return 0;
  };
};

template <class Ch,class Tr=std::char_traits<Ch> >
class basic_forked_ostream: public std::basic_ostream<Ch,Tr> {
private:
  typedef basic_forked_ostream<Ch, Tr> self_t;
  basic_forked_ostream() {};
  basic_forked_ostream(const self_t &source) {};
  self_t& operator=(const self_t &source) { return *this; };

public:
  basic_forked_ostream(std::ostream &out1, std::ostream &out2):
    std::basic_ostream<Ch,Tr>(new basic_forked_ostreambuf<Ch,Tr>(out1, out2))
  {};

  ~basic_forked_ostream(void) {
    delete this->rdbuf();
  };
};


typedef basic_forked_ostreambuf<char> forked_ostreambuf;
typedef basic_forked_ostream<char> forked_ostream;

class forked_ostream_holder {
private:
  bool attached;
  std::ostream copied_out;
  std::ostream *original;
  forked_ostream forked_out;

public:
  forked_ostream_holder
  (std::ostream *base, std::ostream &additional_out):
    attached(true),
    copied_out(base->rdbuf()), original(base),
    forked_out(copied_out, additional_out) {
    // Replace the destination of original
    // with the forked output stream.
    original->rdbuf(forked_out.rdbuf());
  };

  ~forked_ostream_holder() {
    if(this->isAttached()) {
      this->detach();
    }
  };

  void detach() {
    if(this->isAttached()) {
      // Restore rdbuf of the original stream.
      original->rdbuf(copied_out.rdbuf());
    }
  };

  bool isAttached() const {
    return attached;
  };
};

#endif
