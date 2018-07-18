#ifndef LLARP_SERVICE_TAG_HPP
#define LLARP_SERVICE_TAG_HPP

#include <sodium/crypto_generichash.h>
#include <llarp/aligned.hpp>
#include <llarp/dht/key.hpp>

namespace llarp
{
  namespace service
  {
    struct Tag : public llarp::AlignedBuffer< 16 >
    {
      Tag() : llarp::AlignedBuffer< 16 >()
      {
        Zero();
      }

      Tag(const byte_t* d) : llarp::AlignedBuffer< 16 >(d)
      {
      }

      Tag(const std::string& str) : Tag()
      {
        memcpy(data(), str.c_str(), std::min(16UL, str.size()));
      }

      operator llarp::dht::Key_t() const
      {
        llarp::dht::Key_t k;
        crypto_generichash(k, 32, data(), 16, nullptr, 0);
        return k;
      }

      std::string
      ToString() const;

      struct Hash
      {
        std::size_t
        operator()(const Tag& t) const
        {
          return *t.data_l();
        }
      };
    };
  }  // namespace service
}  // namespace llarp

#endif