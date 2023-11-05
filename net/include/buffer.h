#ifndef XFAST_BUFFER_H__
#define XFAST_BUFFER_H__

#include <cassert>
#include "netapi.h"
#include "xendian.h"
#include <string>
#include <vector>
#include <cstring>

using namespace std;

//FIXME: 这个版本，只会扩张， 不会缩小，需要谨慎使用，但是还是放入conction里面，用于网络缓存

namespace xfast
{


// a buffer implied from org.jboss.netty.buffer.ChannelBuffer that described in https://docs.jboss.org/netty/3.2/api/org/jboss/netty/buffer/ChannelBuffer.html
// the bytes in the buffer are net endian

// ChannelBuffer provides two pointer variables to support sequential read and write operations - readerIndex for a read operation and writerIndex for a write operation respectively. The following diagram shows how a buffer is segmented into three areas by the two pointers:
//       +-------------------+------------------+------------------+
//       | discardable bytes |  readable bytes  |  writable bytes  |
//       |                   |     (CONTENT)    |                  |
//       +-------------------+------------------+------------------+
//       |                   |                  |                  |
//       0      <=      readerIndex   <=   writerIndex    <=    capacity
 
// Readable bytes (the actual content)
// This segment is where the actual data is stored. Any operation whose name starts with read or skip will get or skip the data at the current readerIndex and increase it by the number of read bytes. If the argument of the read operation is also a ChannelBuffer and no destination index is specified, the specified buffer's readerIndex is increased together.
// If there's not enough content left, IndexOutOfBoundsException is raised. The default value of newly allocated, wrapped or copied buffer's readerIndex is 0.

//  // Iterates the readable bytes of a buffer.
//  ChannelBuffer buffer = ...;
//  while (buffer.readable()) {
//      System.out.println(buffer.readByte());
//  }
 
// Writable bytes
// This segment is a undefined space which needs to be filled. Any operation whose name ends with write will write the data at the current writerIndex and increase it by the number of written bytes. If the argument of the write operation is also a ChannelBuffer, and no source index is specified, the specified buffer's readerIndex is increased together.
// If there's not enough writable bytes left, IndexOutOfBoundsException is raised. The default value of newly allocated buffer's writerIndex is 0. The default value of wrapped or copied buffer's writerIndex is the capacity of the buffer.

//  // Fills the writable bytes of a buffer with random integers.
//  ChannelBuffer buffer = ...;
//  while (buffer.writableBytes() >= 4) {
//      buffer.writeInt(random.nextInt());
//  }
 
// Discardable bytes
// This segment contains the bytes which were read already by a read operation. Initially, the size of this segment is 0, but its size increases up to the writerIndex as read operations are executed. The read bytes can be discarded by calling discardReadBytes() to reclaim unused area as depicted by the following diagram:
//   BEFORE discardReadBytes()

//       +-------------------+------------------+------------------+
//       | discardable bytes |  readable bytes  |  writable bytes  |
//       +-------------------+------------------+------------------+
//       |                   |                  |                  |
//       0      <=      readerIndex   <=   writerIndex    <=    capacity


//   AFTER discardReadBytes()

//       +------------------+--------------------------------------+
//       |  readable bytes  |    writable bytes (got more space)   |
//       +------------------+--------------------------------------+
//       |                  |                                      |
//  readerIndex (0) <= writerIndex (decreased)        <=        capacity
 
// Please note that there is no guarantee about the content of writable bytes after calling discardReadBytes(). The writable bytes will not be moved in most cases and could even be filled with completely different data depending on the underlying buffer implementation.
// Clearing the buffer indexes
// You can set both readerIndex and writerIndex to 0 by calling clear(). It does not clear the buffer content (e.g. filling with 0) but just clears the two pointers. Please also note that the semantic of this operation is different from Buffer.clear().
//   BEFORE clear()

//       +-------------------+------------------+------------------+
//       | discardable bytes |  readable bytes  |  writable bytes  |
//       +-------------------+------------------+------------------+
//       |                   |                  |                  |
//       0      <=      readerIndex   <=   writerIndex    <=    capacity


//   AFTER clear()

//       +---------------------------------------------------------+
//       |             writable bytes (got more space)             |
//       +---------------------------------------------------------+
//       |                                                         |
//       0 = readerIndex = writerIndex            <=            capacity
class xBuffer  {
    public:
        static const size_t kDiscardableSize = sizeof(int);
        static const size_t kInitialSize = 1024-sizeof(int); // for a 1M space

        explicit xBuffer(size_t initialSize = kInitialSize)
            : buffer_(kDiscardableSize + initialSize),
              readerIndex_(kDiscardableSize),
              writerIndex_(kDiscardableSize)
        {
            assert(readableBytes() == 0);
            assert(writableBytes() == initialSize);
            assert(discardableBytes() == kDiscardableSize);
        }

        // implicit copy-ctor, move-ctor, dtor and assignment are fine
        // NOTE: implicit move-ctor is added in g++ 4.6

        void swap(xBuffer &rhs){
            buffer_.swap(rhs.buffer_);
            std::swap(readerIndex_, rhs.readerIndex_);
            std::swap(writerIndex_, rhs.writerIndex_);
        }

        size_t readableBytes() const
        {
            return writerIndex_ - readerIndex_;
        }

        size_t writableBytes() const
        {
            return buffer_.size() - writerIndex_;
        }

        size_t discardableBytes() const
        {
            return readerIndex_;
        }

        const char *peek() const
        {
            return begin() + readerIndex_;
        }

        // moveOver returns void, to prevent
        // string str(moveOver(readableBytes()), readableBytes());
        void moveOver(size_t len)
        {
            assert(len <= readableBytes());
            if (len < readableBytes())
            {
                readerIndex_ += len;
            }
            else
            {
                reset();
            }
        }

        void moveTo(const char *end)
        {
            assert(peek() <= end);
            assert(end <= beginWrite());
            moveOver(end - peek());
        }

        void moveInt64()
        {
            moveOver(sizeof(int64_t));
        }

        void moveInt32()
        {
            moveOver(sizeof(int32_t));
        }

        void moveInt16()
        {
            moveOver(sizeof(int16_t));
        }

        void moveInt8()
        {
            moveOver(sizeof(int8_t));
        }

        void reset()
        {
            readerIndex_ = kDiscardableSize;
            writerIndex_ = kDiscardableSize;
        }

        string readAll()
        {
            return readStr(readableBytes());
        }

        string readStr(size_t len)
        {
            assert(len <= readableBytes());
            string result(peek(), len);
            moveOver(len);
            return result;
        }

        string toString() const
        {
            return string(peek(), readableBytes());
        }

        void append(const string &str)
        {
            append(str.data(), str.size());
        }

        void append(const char *data, size_t len)
        {
            ensureWritableBytes(len);
            std::copy(data, data + len, beginWrite());
            hasWritten(len);
        }

        void append(const void *data, size_t len)
        {
            append(static_cast<const char *>(data), len);
        }

        void ensureWritableBytes(size_t len)
        {
            if (writableBytes() < len)
            {
                makeSpace(len);
            }
            assert(writableBytes() >= len);
        }

        char *beginWrite()
        {
            return begin() + writerIndex_;
        }

        const char *beginWrite() const
        {
            return begin() + writerIndex_;
        }

        void hasWritten(size_t len)
        {
            assert(len <= writableBytes());
            writerIndex_ += len;
        }

        void unwrite(size_t len)
        {
            assert(len <= readableBytes());
            writerIndex_ -= len;
        }

        void appendInt64(int64_t x)
        {
            int64_t be64 = xNetApi::hostToNetwork64(x);
            append(&be64, sizeof be64);
        }

        void appendInt32(int32_t x)
        {
            int32_t be32 = xNetApi::hostToNetwork32(x);
            append(&be32, sizeof be32);
        }

        void appendInt16(int16_t x)
        {
            int16_t be16 = xNetApi::hostToNetwork16(x);
            append(&be16, sizeof be16);
        }

        void appendInt8(int8_t x)
        {
            append(&x, sizeof x);
        }

        int64_t readInt64()
        {
            int64_t result = peekInt64();
            moveInt64();
            return result;
        }

        int32_t readInt32()
        {
            int32_t result = peekInt32();
            moveInt32();
            return result;
        }

        int16_t readInt16()
        {
            int16_t result = peekInt16();
            moveInt16();
            return result;
        }

        int8_t readInt8()
        {
            int8_t result = peekInt8();
            moveInt8();
            return result;
        }

        int64_t peekInt64() const
        {
            assert(readableBytes() >= sizeof(int64_t));
            int64_t be64 = 0;
            ::memcpy(&be64, peek(), sizeof be64);
            return xNetApi::networkToHost64(be64);
        }

        int32_t peekInt32() const
        {
            assert(readableBytes() >= sizeof(int32_t));
            int32_t be32 = 0;
            ::memcpy(&be32, peek(), sizeof be32);
            return xNetApi::networkToHost32(be32);
        }

        int16_t peekInt16() const
        {
            assert(readableBytes() >= sizeof(int16_t));
            int16_t be16 = 0;
            ::memcpy(&be16, peek(), sizeof be16);
            return xNetApi::networkToHost16(be16);
        }

        int8_t peekInt8() const
        {
            assert(readableBytes() >= sizeof(int8_t));
            int8_t x = *peek();
            return x;
        }

        void prependInt64(int64_t x)
        {
            int64_t be64 = xNetApi::hostToNetwork64(x);
            prepend(&be64, sizeof be64);
        }

        void prependInt32(int32_t x)
        {
            int32_t be32 = xNetApi::hostToNetwork32(x);
            prepend(&be32, sizeof be32);
        }

        void prependInt16(int16_t x)
        {
            int16_t be16 = xNetApi::hostToNetwork16(x);
            prepend(&be16, sizeof be16);
        }

        void prependInt8(int8_t x)
        {
            prepend(&x, sizeof x);
        }

        void prepend(const void * /*restrict*/ data, size_t len)
        {
            assert(len <= discardableBytes());
            readerIndex_ -= len;
            const char *d = static_cast<const char *>(data);
            std::copy(d, d + len, begin() + readerIndex_);
        }

        size_t bufferTotalCapacity() const
        {
            return buffer_.capacity();
        }

        int fillFromFd(int fd);

    private:
        char *begin()
        {
            return &*buffer_.begin();
        }

        const char *begin() const
        {
            return &*buffer_.begin();
        }

        void makeSpace(size_t len)
        {
            if (writableBytes() + discardableBytes() < len + kDiscardableSize)
            {
                buffer_.resize(writerIndex_ + len);
            }
            else
            {
                // move readable data to the front, make space inside buffer
                assert(kDiscardableSize < readerIndex_);
                size_t readable = readableBytes();
                std::copy(begin() + readerIndex_,
                          begin() + writerIndex_,
                          begin() + kDiscardableSize);
                readerIndex_ = kDiscardableSize;
                writerIndex_ = readerIndex_ + readable;
                assert(readable == readableBytes());
            }
        }

    private:
        std::vector<char> buffer_;
        size_t readerIndex_;
        size_t writerIndex_;
    };

} // namespace xfast

#endif //XFAST_BUFFER_H__
