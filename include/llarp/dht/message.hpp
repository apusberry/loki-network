#ifndef LLARP_DHT_MESSAGE_HPP
#define LLARP_DHT_MESSAGE_HPP

#include <llarp/dht.h>
#include <llarp/bencode.hpp>
#include <llarp/dht/key.hpp>
#include <llarp/path_types.hpp>
#include <vector>

namespace llarp
{
  namespace dht
  {
    constexpr size_t MAX_MSG_SIZE = 2048;

    struct IMessage : public llarp::IBEncodeMessage
    {
      virtual ~IMessage(){};

      /// construct
      IMessage(const Key_t& from) : From(from)
      {
      }

      virtual bool
      HandleMessage(
          llarp_dht_context* dht,
          std::vector< std::unique_ptr< IMessage > >& replies) const = 0;

      Key_t From;
      PathID_t pathID;
    };

    std::unique_ptr< IMessage >
    DecodeMessage(const Key_t& from, llarp_buffer_t* buf, bool relayed = false);

    bool
    DecodeMesssageList(Key_t from, llarp_buffer_t* buf,
                       std::vector< std::unique_ptr< IMessage > >& dst,
                       bool relayed = false);
  }  // namespace dht
}  // namespace llarp

#endif
