#ifndef XGBOOST_STREAM_H
#define XGBOOST_STREAM_H

//#include <cstdio>
#include <string>
/*!
 * \file xgboost_stream.h
 * \brief general stream interface for serialization
 * \author Tianqi Chen: tianqi.tchen@gmail.com
 */
namespace xgboost{
namespace utils {
/*!
 * \brief interface of stream I/O, used to serialize model
 */
class IStream {
public:
  /*!
   * \brief read data from stream
   * \param ptr pointer to memory buffer
   * \param size size of block
   * \return usually is the size of data readed
   */
  virtual size_t Read(void *ptr, size_t size) = 0;
  /*!
   * \brief write data to stream
   * \param ptr pointer to memory buffer
   * \param size size of block
   */
  virtual void Write(const void *ptr, size_t size) = 0;
  /*! \brief virtual destructor */
  virtual ~IStream(void) {}
};

/*! \brief implementation of file i/o stream */
class FileStream : public IStream {
private:
  // FILE *fp;
  // void *fp;
  std::string m_mem;
  size_t m_offset;

public:
  virtual size_t Read(void *ptr, size_t size) {
    if (m_offset + size > m_mem.size()) {
      return 0;
    }
    memcpy(ptr, &m_mem[0] + m_offset, size);
    m_offset += size;
    return size;
    // return fread(ptr, size, 1, fp);
  }
  virtual void Write(const void *ptr, size_t size) {
    if (size == 0) {
      return;
    }
    m_mem += std::string((const char *)ptr, size);
    // fwrite(ptr, size, 1, fp);
  }
  inline const std::string &get() const { return m_mem; }
  inline void set(const std::string &mem) { m_mem = mem; }
};
}; // namespace utils
}; // namespace xgboost
#endif
