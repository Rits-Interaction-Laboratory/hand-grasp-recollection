#ifndef __PREFIXED_STREAM_HEADER
#define __PREFIXED_STREAM_HEADER

#include <ostream>
#include <string>

template <class Ch, class Tr=std::char_traits<Ch> >
class basic_prefixed_ostreambuf : public std::basic_streambuf<Ch,Tr> {
private:
  std::ostream &out;
  std::string prefix;
  bool beginning_of_line_flag;

public:
  basic_prefixed_ostreambuf
  (std::ostream &dest, const std::string &initial_prefix):
    out(dest), prefix(initial_prefix), beginning_of_line_flag(true) {
  };

  virtual ~basic_prefixed_ostreambuf(void) {
    out.flush();
  };

protected:
  std::streampos seekoff
  (const std::streamoff off, const typename std::ios_base::seekdir dir,
   const typename std::ios_base::openmode open_mode = std::ios_base::out) {
    return out.rdbuf()->pubseekoff(off, dir, open_mode);
  }

  std::streampos seekpos
  (const std::streampos pos,
   const typename std::ios_base::openmode open_mode = std::ios::out) {
    return out.rdbuf()->pubseekpos(pos, open_mode);
  }

  int overflow(int c = Tr::eof()) {
    if(c != Tr::eof()) {
      if(out && beginning_of_line_flag) {
        out << prefix;
        beginning_of_line_flag = false;
      }

      if(c == '\n') {
        beginning_of_line_flag = true;
      }

      if(out) {
        out.put(c);
      }

      if(out) {
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
    out.flush();
    return 0;
  };
};

template <class Ch,class Tr=std::char_traits<Ch> >
class basic_prefixed_ostream: public std::basic_ostream<Ch,Tr> {
public:
  basic_prefixed_ostream(std::ostream &out, const std::string &prefix):
    std::basic_ostream<Ch,Tr>
    (new basic_prefixed_ostreambuf<Ch,Tr>(out, prefix))
  {};

  ~basic_prefixed_ostream(void) {
    delete this->rdbuf();
  };
};


typedef basic_prefixed_ostreambuf<char> prefixed_ostreambuf;
typedef basic_prefixed_ostream<char> prefixed_ostream;

class prefixed_ostream_holder {
private:
  bool attached;
  std::ostream *original;
  std::ostream copied_out;
  prefixed_ostream prefixed_out;

public:
  prefixed_ostream_holder
  (std::ostream *base, const std::string &prefix):
    attached(true), original(base), copied_out(base->rdbuf()),
    prefixed_out(copied_out, prefix) {
    // Replace the destination of original
    // with the prefixed output stream.
    original->rdbuf(prefixed_out.rdbuf());
  };

  ~prefixed_ostream_holder() {
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
