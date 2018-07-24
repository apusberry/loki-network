#include "dnsc.hpp"
#include <llarp/dns.h>
#include "buffer.hpp"

#include <netdb.h>  /* getaddrinfo, getnameinfo */
#include <stdlib.h> /* exit */
#include <string.h> /* memset */
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> /* close */

#include <arpa/inet.h>
#include <netinet/in.h>

#include <llarp/dns.h>
#include "logger.hpp"
#include "net.hpp"  // for llarp::Addr

// FIXME: make configurable
#define SERVER "8.8.8.8"
#define PORT 53

#define DNC_BUF_SIZE 512
// a question to be asked remotely
// header, question
struct dns_query
{
  uint16_t length;
  //char *url;
  unsigned char request[DNC_BUF_SIZE];
  //uint16_t reqType;
};

struct dns_query*
build_dns_packet(char *url, uint16_t id, uint16_t reqType)
{
  dns_query *dnsQuery = new dns_query;
  dnsQuery->length  = 12;
  // ID
  // buffer[0] = (value & 0xFF00) >> 8;
  // buffer[1] = value & 0xFF;
  llarp::LogDebug("building request ", id);

  dnsQuery->request[0] = (id & 0xFF00) >> 8;
  dnsQuery->request[1] = (id & 0x00FF) >> 0;
  // field
  dnsQuery->request[2] = 0x01;
  dnsQuery->request[3] = 0x00;
  // questions
  dnsQuery->request[4] = 0x00;
  dnsQuery->request[5] = 0x01;
  // answers
  dnsQuery->request[6] = 0x00;
  dnsQuery->request[7] = 0x00;
  // ns
  dnsQuery->request[8] = 0x00;
  dnsQuery->request[9] = 0x00;
  // ar
  dnsQuery->request[10] = 0x00;
  dnsQuery->request[11] = 0x00;

  char *word;
  // llarp::LogDebug("Asking DNS server %s about %s", SERVER, dnsQuery->url);

  char *strTemp = strdup(url);
  word          = strtok(strTemp, ".");
  while(word)
  {
    // llarp::LogDebug("parsing hostname: \"%s\" is %zu characters", word,
    // strlen(word));
    dnsQuery->request[dnsQuery->length++] = strlen(word);
    for(unsigned int i = 0; i < strlen(word); i++)
    {
      dnsQuery->request[dnsQuery->length++] = word[i];
    }
    word = strtok(NULL, ".");
  }

  dnsQuery->request[dnsQuery->length++] = 0x00;  // End of the host name
  dnsQuery->request[dnsQuery->length++] =
  0x00;  // 0x0001 - Query is a Type A query (host address)
  dnsQuery->request[dnsQuery->length++] = reqType;
  dnsQuery->request[dnsQuery->length++] =
  0x00;  // 0x0001 - Query is class IN (Internet address)
  dnsQuery->request[dnsQuery->length++] = 0x01;
  return dnsQuery;
}

struct sockaddr *
raw_resolve_host(const char *url)
{
  //char *sUrl = strdup(url);
  //struct dns_query dnsQuery;
  dns_query *dns_packet = build_dns_packet((char *)url, 0xDB42, 1);

  /*
  dnsQuery.length  = 12;
  dnsQuery.url     = sUrl;
  dnsQuery.reqType = 0x01;
  // dnsQuery.request  = { 0xDB, 0x42, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  // 0x00, 0x00, 0x00 };
  dnsQuery.request[0]  = 0xDB;
  dnsQuery.request[1]  = 0x42;
  dnsQuery.request[2]  = 0x01;
  dnsQuery.request[3]  = 0x00;
  dnsQuery.request[4]  = 0x00;
  dnsQuery.request[5]  = 0x01;
  dnsQuery.request[6]  = 0x00;
  dnsQuery.request[7]  = 0x00;
  dnsQuery.request[8]  = 0x00;
  dnsQuery.request[9]  = 0x00;
  dnsQuery.request[10] = 0x00;
  dnsQuery.request[11] = 0x00;
  */

  char *word;
  unsigned int i;
  llarp::LogDebug("Asking DNS server ", SERVER, " about ", url);
  // dnsQuery.reqType = 0x01;
  /*
  word = strtok(sUrl, ".");
  while(word)
  {
    llarp::LogDebug("parsing hostname: \"%s\" is %zu characters\n", word,
                    strlen(word));
    dnsQuery.request[dnsQuery.length++] = strlen(word);
    for(i = 0; i < strlen(word); i++)
    {
      dnsQuery.request[dnsQuery.length++] = word[i];
    }
    word = strtok(NULL, ".");
  }

  dnsQuery.request[dnsQuery.length++] = 0x00;  // End of the host name
  dnsQuery.request[dnsQuery.length++] =
      0x00;  // 0x0001 - Query is a Type A query (host address)
  dnsQuery.request[dnsQuery.length++] = dnsQuery.reqType;
  dnsQuery.request[dnsQuery.length++] =
      0x00;  // 0x0001 - Query is class IN (Internet address)
  dnsQuery.request[dnsQuery.length++] = 0x01;
  */

  struct sockaddr_in addr;
  // int socket;
  ssize_t ret;
  int rcode;
  socklen_t size;
  int ip = 0;
  int length;
  unsigned char buffer[DNC_BUF_SIZE];
  // unsigned char tempBuf[3];
  uint16_t QDCOUNT;   // No. of items in Question Section
  uint16_t ANCOUNT;   // No. of items in Answer Section
  uint16_t NSCOUNT;   // No. of items in Authority Section
  uint16_t ARCOUNT;   // No. of items in Additional Section
  uint16_t QCLASS;    // Specifies the class of the query
  uint16_t ATYPE;     // Specifies the meaning of the data in the RDATA field
  uint16_t ACLASS;    // Specifies the class of the data in the RDATA field
  uint32_t TTL;       // The number of seconds the results can be cached
  uint16_t RDLENGTH;  // The length of the RDATA field
  uint16_t MSGID;

  int sockfd;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0)
  {
    llarp::LogWarn("Error creating socket!\n");
    return nullptr;
  }
  // socket = sockfd;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = inet_addr(SERVER);
  addr.sin_port        = htons(PORT);
  size                 = sizeof(addr);

  // hexdump("sending packet", &dnsQuery.request, dnsQuery.length);
  ret = sendto(sockfd, dns_packet->request, dns_packet->length, 0,
               (struct sockaddr *)&addr, size);
  delete dns_packet;
  if(ret < 0)
  {
    llarp::LogWarn("Error Sending Request");
    return nullptr;
  }
  // llarp::LogInfo("Sent\n");

  memset(&buffer, 0, DNC_BUF_SIZE);
  ret = recvfrom(sockfd, buffer, DNC_BUF_SIZE, 0, (struct sockaddr *)&addr,
                 &size);
  if(ret < 0)
  {
    llarp::LogWarn("Error Receiving Response");
    return nullptr;
  }

  // hexdump("received packet", &buffer, ret);

  close(sockfd);

  rcode = (buffer[3] & 0x0F);

  // tempBuf[0] = buffer[4];
  // tempBuf[1] = buffer[5];
  // tempBuf[2] = '\0';

  // printf("%0x %0x %0x %0x\n", buffer[4], buffer[5], tempBuf[0], tempBuf[1]);

  // QDCOUNT = (uint16_t) strtol(tempBuf, NULL, 16);
  QDCOUNT = (uint16_t)buffer[4] * 0x100 + buffer[5];
  llarp::LogDebug("entries in question section: %u\n", QDCOUNT);
  ANCOUNT = (uint16_t)buffer[6] * 0x100 + buffer[7];
  llarp::LogDebug("records in answer section: %u\n", ANCOUNT);
  NSCOUNT = (uint16_t)buffer[8] * 0x100 + buffer[9];
  llarp::LogDebug("name server resource record count: %u\n", NSCOUNT);
  ARCOUNT = (uint16_t)buffer[10] * 0x100 + buffer[11];
  llarp::LogDebug("additional records count: %u\n", ARCOUNT);

  /*
  llarp::LogDebug("query type: %u\n", dnsQuery.reqType);
  QCLASS = (uint16_t)dnsQuery.request[dnsQuery.length - 2] * 0x100
      + dnsQuery.request[dnsQuery.length - 1];
  llarp::LogDebug("query class: %u\n", QCLASS);
  length = dnsQuery.length + 1;  // to skip 0xc00c
  ATYPE  = (uint16_t)buffer[length + 1] * 0x100 + buffer[length + 2];
  llarp::LogDebug("answer type: %u\n", ATYPE);
  ACLASS = (uint16_t)buffer[length + 3] * 0x100 + buffer[length + 4];
  llarp::LogDebug("answer class: %u\n", ACLASS);
  TTL = (uint32_t)buffer[length + 5] * 0x1000000 + buffer[length + 6] * 0x10000
      + buffer[length + 7] * 0x100 + buffer[length + 8];
  llarp::LogDebug("seconds to cache: %u\n", TTL);
  RDLENGTH = (uint16_t)buffer[length + 9] * 0x100 + buffer[length + 10];
  llarp::LogDebug("bytes in answer: %u\n", RDLENGTH);
  MSGID = (uint16_t)buffer[0] * 0x100 + buffer[1];
  llarp::LogDebug("answer msg id: %u\n", MSGID);
  */

  if(rcode == 2)
  {
    llarp::LogWarn("nameserver %s returned SERVFAIL:\n", SERVER);
    llarp::LogWarn(
        "  the name server was unable to process this query due to a\n  "
        "problem with the name server.\n");
    return nullptr;
  }
  else if(rcode == 3)
  {
    llarp::LogWarn("nameserver %s returned NXDOMAIN for ", SERVER);
    llarp::LogWarn(
        "  the domain name referenced in the query does not exist\n");
    return nullptr;
  }

  /* search for and print IPv4 addresses */
  //if(dnsQuery.reqType == 0x01)
  if (1)
  {
    llarp::LogDebug("DNS server's answer is: (type#=%u):", ATYPE);
    // printf("IPv4 address(es) for %s:\n", dnsQuery.url);
    for(i = 0; i < ret; i++)
    {
      if(buffer[i] == 0xC0 && buffer[i + 3] == 0x01)
      {
        ip++;
        i += 12; /* ! += buf[i+1]; */
        llarp::LogDebug(" %u.%u.%u.%u\n", buffer[i], buffer[i + 1],
                        buffer[i + 2], buffer[i + 3]);
        struct sockaddr *g_addr = new sockaddr;
        g_addr->sa_family       = AF_INET;
#if ((__APPLE__ && __MACH__) || __FreeBSD__)
        g_addr->sa_len          = sizeof(in_addr);
#endif
        struct in_addr *addr    = &((struct sockaddr_in *)g_addr)->sin_addr;
        unsigned char *ip;

        // have ip point to s_addr
        ip = (unsigned char *)&(addr->s_addr);

        ip[0] = buffer[i + 0];
        ip[1] = buffer[i + 1];
        ip[2] = buffer[i + 2];
        ip[3] = buffer[i + 3];

        return g_addr;
      }
    }

    if(!ip)
    {
      llarp::LogWarn("  No IPv4 address found in the DNS response!\n");
      return nullptr;
    }
  }
  return nullptr;
}

void
llarp_handle_dnsc_recvfrom(struct llarp_udp_io *udp,
                                const struct sockaddr *saddr, const void *buf,
                                ssize_t sz)
{
  //lock_t lock(m_dnsc_Mutex);
  // llarp::LogInfo("got a response, udp user is ", udp->user);

  unsigned char *castBuf = (unsigned char *)buf;
  auto buffer            = llarp::StackBuffer< decltype(castBuf) >(castBuf);
  dns_msg_header *hdr    = decode_hdr((const char *)castBuf);

  llarp::LogDebug("Header got client responses for id: ", hdr->id);

  // if we sent this out, then there's an id
  struct dns_tracker *tracker        = (struct dns_tracker *)udp->user;
  struct dnsc_answer_request *request = tracker->client_request[hdr->id];

  if(!request)
  {
    llarp::LogError(
        "User data to DNS Client response not a dnsc_answer_request");
    // we can't call back the hook
    return;
  }
  // llarp_dnsc_unbind(request);

  if(sz < 0)
  {
    llarp::LogWarn("Error Receiving DNS Client Response");
    request->resolved(request);
    return;
  }

  // unsigned char *castBuf = (unsigned char *)buf;
  // auto buffer = llarp::StackBuffer< decltype(castBuf) >(castBuf);

  // hexdump("received packet", &buffer, ret);
  /*
  uint16_t QDCOUNT;   // No. of items in Question Section
  uint16_t ANCOUNT;   // No. of items in Answer Section
  uint16_t NSCOUNT;   // No. of items in Authority Section
  uint16_t ARCOUNT;   // No. of items in Additional Section
  uint16_t QCLASS;    // Specifies the class of the query
  uint16_t ATYPE;     // Specifies the meaning of the data in the RDATA field
  uint16_t ACLASS;    // Specifies the class of the data in the RDATA field
  uint32_t TTL;       // The number of seconds the results can be cached
  uint16_t RDLENGTH;  // The length of the RDATA field
  uint16_t MSGID;
  */
  uint8_t rcode;
  //int length;

  //struct dns_query *dnsQuery = &request->query;

  //rcode = (buffer[3] & 0x0F);
  //llarp::LogInfo("dnsc rcode ", rcode);

  dns_msg_header *msg = decode_hdr((const char *)castBuf);
  castBuf += 12;
  llarp::LogDebug("msg id ", msg->id);
  uint8_t qr = msg->qr;
  llarp::LogDebug("msg qr ", qr);
  uint8_t opcode = msg->opcode;
  llarp::LogDebug("msg op ", opcode);
  rcode = msg->rcode;
  llarp::LogDebug("msg rc ", rcode);

  llarp::LogDebug("msg qdc ", msg->qdCount);
  llarp::LogDebug("msg anc ", msg->anCount);
  llarp::LogDebug("msg nsc ", msg->nsCount);
  llarp::LogDebug("msg arc ", msg->arCount);

  // we may need to parse question first

  /*
  dns_msg_question *question = decode_question((const char *)castBuf);
  llarp::LogInfo("que name  ", question->name);
  castBuf += question->name.length() + 8;

  dns_msg_answer *answer = decode_answer((const char *)castBuf);
  castBuf += answer->name.length() + 4 + 4 + 4 + answer->rdLen;
  */


  // FIXME: only handling one atm
  dns_msg_question *question = nullptr;
  for(uint i = 0; i < hdr->qdCount; i++)
  {
    question = decode_question((const char*)castBuf);
    llarp::LogDebug("Read a question");
    castBuf += question->name.length() + 8;
  }

  // FIXME: only handling one atm
  dns_msg_answer *answer = nullptr;
  for(uint i = 0; i < hdr->anCount; i++)
  {
    answer = decode_answer((const char*)castBuf);
    llarp::LogDebug("Read an answer");
    castBuf += answer->name.length() + 4 + 4 + 4 + answer->rdLen;
  }
  // handle authority records (usually no answers with these, so we'll just stomp)
  // usually NS records tho
  for(uint i = 0; i < hdr->nsCount; i++)
  {
    answer = decode_answer((const char*)castBuf);
    llarp::LogDebug("Read an authority");
    castBuf += answer->name.length() + 4 + 4 + 4 + answer->rdLen;
  }

  // dns_msg_answer *answer2 = decode_answer((const char*)castBuf);
  // castBuf += answer->name.length() + 4 + 4 + 4 + answer->rdLen;

  // llarp::LogDebug("query type: %u\n", dnsQuery->reqType);
  /*
  QCLASS = (uint16_t)dnsQuery->request[dnsQuery->length - 2] * 0x100
      + dnsQuery->request[dnsQuery->length - 1];
  llarp::LogInfo("query class: ", QCLASS);

  length = dnsQuery->length + 1;  // to skip 0xc00c
  // printf("length [%d] from [%d]\n", length, buffer.base);
  ATYPE = (uint16_t)buffer[length + 1] * 0x100 + buffer[length + 2];
  llarp::LogInfo("answer type: ", ATYPE);
  ACLASS = (uint16_t)buffer[length + 3] * 0x100 + buffer[length + 4];
  llarp::LogInfo("answer class: ", ACLASS);
  TTL = (uint32_t)buffer[length + 5] * 0x1000000 + buffer[length + 6] * 0x10000
      + buffer[length + 7] * 0x100 + buffer[length + 8];
  llarp::LogInfo("seconds to cache: ", TTL);
  RDLENGTH = (uint16_t)buffer[length + 9] * 0x100 + buffer[length + 10];
  llarp::LogInfo("bytes in answer: ", RDLENGTH);

  MSGID = (uint16_t)buffer[0] * 0x100 + buffer[1];
  // llarp::LogDebug("answer msg id: %u\n", MSGID);
  */

  if(answer == nullptr)
  {
    llarp::LogWarn("nameserver ", SERVER, " didnt return any answers:");
    request->resolved(request);
    return;
  }

  llarp::LogDebug("ans class ", answer->aClass);
  llarp::LogDebug("ans type  ", answer->type);
  llarp::LogDebug("ans ttl   ", answer->ttl);
  llarp::LogDebug("ans rdlen ", answer->rdLen);

  /*
  llarp::LogInfo("ans2 class ", answer2->aClass);
  llarp::LogInfo("ans2 type  ", answer2->type);
  llarp::LogInfo("ans2 ttl   ", answer2->ttl);
  llarp::LogInfo("ans2 rdlen ", answer2->rdLen);
  */

  if(rcode == 2)
  {
    llarp::LogWarn("nameserver ", SERVER, " returned SERVFAIL:");
    llarp::LogWarn(
        "  the name server was unable to process this query due to a problem "
        "with the name server.");
    request->resolved(request);
    return;
  }
  else if(rcode == 3)
  {
    llarp::LogWarn("nameserver ", SERVER,
                   " returned NXDOMAIN for: ", request->question.name);
    llarp::LogWarn("  the domain name referenced in the query does not exist");
    request->resolved(request);
    return;
  }

  int ip = 0;

  /* search for and print IPv4 addresses */
  //if(dnsQuery->reqType == 0x01)
  if(request->question.type == 1)
  {
    //llarp::LogInfo("DNS server's answer is: (type#=", ATYPE, "):");
    llarp::LogDebug("IPv4 address(es) for ", request->question.name, ":");

    if (answer->rdLen == 4)
    {
      request->result.sa_family = AF_INET;
#if ((__APPLE__ && __MACH__) || __FreeBSD__)
      request->result.sa_len    = sizeof(in_addr);
#endif
      struct in_addr *addr =
      &((struct sockaddr_in *)&request->result)->sin_addr;

      unsigned char *ip = (unsigned char *)&(addr->s_addr);
      ip[0] = answer->rData[0];
      ip[1] = answer->rData[1];
      ip[2] = answer->rData[2];
      ip[3] = answer->rData[3];

      llarp::Addr test(request->result);
      llarp::LogDebug(test);
      request->found = true;
      request->resolved(request);
      return;
    }

    if(!ip)
    {
      llarp::LogWarn("  No IPv4 address found in the DNS answer!");
      request->resolved(request);
      return;
    }
  }
}

bool
llarp_resolve_host(struct dnsc_context *dnsc, const char *url,
                   dnsc_answer_hook_func resolved, void *user)
{
  dnsc_answer_request *request = new dnsc_answer_request;
  request->sock     = (void *)&dnsc->udp;
  request->user     = user;
  request->resolved = resolved;
  request->found    = false;
  request->context  = dnsc;

  char *sUrl = strdup(url);
  request->question.name   = sUrl;
  request->question.type   = 1;
  request->question.qClass = 1;

  // register request with udp response tracker
  dns_tracker *tracker       = (dns_tracker *)dnsc->udp->user;

  /*
  uint16_t length = 0;
  dns_msg_header header;
  header.id         = htons(id);
  header.qr         = 0;
  header.opcode     = 0;
  header.aa         = 0;
  header.tc         = 0;
  header.rd         = 1;
  header.ra         = 0;
  header.rcode      = 0;
  header.qdCount    = htons(1);
  header.anCount    = 0;
  header.nsCount    = 0;
  header.arCount    = 0;
  length += 12;

  //request->question.name   = sUrl;
  request->question.type   = htons(1);
  request->question.qClass = htons(1);

  uint16_t qLen = request->question.name.length() + 8;
  length += qLen;

  unsigned char bytes[length];
  // memcpy isn't going to fix the network endian issue
  // encode header into bytes
  memcpy(bytes, &header, 12);
  // encode question into bytes
  memcpy(bytes + 12, &request->question, qLen);
  */

  uint16_t id = ++tracker->c_requests;
  tracker->client_request[id] = request;
  //llarp::LogInfo("Sending request #", tracker->c_requests, " ", length, " bytes");

  dns_query *dns_packet = build_dns_packet((char *)url, id, 1);
  //ssize_t ret = llarp_ev_udp_sendto(dnsc->udp, dnsc->server, bytes, length);
  ssize_t ret = llarp_ev_udp_sendto(dnsc->udp, dnsc->server, dns_packet->request, dns_packet->length);
  delete dns_packet;
  if(ret < 0)
  {
    llarp::LogWarn("Error Sending Request");
    return false;
  }

  return true;
}

void
llarp_host_resolved(dnsc_answer_request *request)
{
  delete request;
}

bool
llarp_dnsc_init(struct dnsc_context *dnsc, struct llarp_udp_io *udp,
                const char *dnsc_hostname, uint16_t dnsc_port)
{
  sockaddr_in *trgaddr     = new sockaddr_in;
  trgaddr->sin_addr.s_addr = inet_addr(dnsc_hostname);
  trgaddr->sin_port        = htons(dnsc_port);
  trgaddr->sin_family      = AF_INET;
  dnsc->server             = (sockaddr *)trgaddr;
  dnsc->udp                = udp;
  return true;
}

bool
llarp_dnsc_stop(struct dnsc_context *dnsc)
{
  delete(sockaddr_in *)dnsc->server;  // deallocation
  return true;
}
