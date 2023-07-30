// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "Lang.h"

using namespace ni;

tUUID __stdcall cLang::CreateLocalUUID() {
  return ni::UUIDGenLocal();
}
tUUID __stdcall cLang::CreateGlobalUUID() {
  return ni::UUIDGenGlobal();
}

//---------------------------------------------------------------------------
//
// Win32
//
//---------------------------------------------------------------------------
#ifdef niWin32

#  include <rpc.h>  // UuidCreate*
#  pragma comment(lib,"Rpcrt4.lib")

namespace ni {

niExportFunc(tUUID) UUIDGenLocal() {
  UUID uuid;
  UuidCreateSequential(&uuid);
  return *(tUUID*)&uuid;
}

niExportFunc(tUUID) UUIDGenGlobal() {
  UUID uuid;
  UuidCreate(&uuid);
  return *(tUUID*)&uuid;
}

}

//---------------------------------------------------------------------------
//
// OSX & iOS
//
//---------------------------------------------------------------------------
#elif defined niOSX || defined niIOS

#define Ptr _OSXPtr
#include <CoreFoundation/CFUUID.h>
#undef Ptr

namespace ni {

niExportFunc(tUUID) UUIDGenLocal() {
  CFUUIDRef theUUID = CFUUIDCreate(NULL);
  CFUUIDBytes bytes = CFUUIDGetUUIDBytes(theUUID);
  CFRelease(theUUID);
  niCAssert(sizeof(tUUID) == sizeof(bytes));
  return *(tUUID*)&bytes;
}

niExportFunc(tUUID) UUIDGenGlobal() {
  return UUIDGenLocal();
}

}

//---------------------------------------------------------------------------
//
// Posix
//
//---------------------------------------------------------------------------
#elif defined niPosix

#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

struct uuid {
  tU32   time_low;
  tU16   time_mid;
  tU16   time_hi_and_version;
  tU16   clock_seq;
  tU8    node[6];
};

typedef unsigned char uuid_t[16];

static void uuid_pack(const struct uuid *uu, uuid_t ptr);

#if !defined niJSCC
static int get_random_fd(void)
{
  struct timeval  tv;
  static int  fd = -2;
  int   i;

  if (fd == -2) {
    gettimeofday(&tv, 0);
    fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1)
      fd = open("/dev/random", O_RDONLY | O_NONBLOCK);
    srand((getpid() << 16) ^ getuid() ^ tv.tv_sec ^ tv.tv_usec);
  }
  /* Crank the random number generator a few times */
  gettimeofday(&tv, 0);
  for (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
    rand();
  return fd;
}

#endif

//
// Generate a series of random bytes.  Use /dev/urandom if possible,
// and if not, use srandom/random.
//
static void get_random_bytes(void *buf, int nbytes)
{
  int i, n = nbytes;
#if !defined niJSCC
  int fd = get_random_fd();
#endif
  int lose_counter = 0;
  unsigned char *cp = (unsigned char *) buf;

#if !defined niJSCC
  if (fd >= 0)
#endif
  {
    while (n > 0) {
#if defined niJSCC
      *(int*)cp = rand();
      i = sizeof(int);
#else
      i = read(fd, cp, n);
#endif
      if (i <= 0) {
        if (lose_counter++ > 16)
          break;
        continue;
      }
      n -= i;
      cp += i;
      lose_counter = 0;
    }
  }

  // We do this all the time, but this is the only source of
  // randomness if /dev/random/urandom is out to lunch.
  for (cp = (tU8*)buf, i = 0; i < nbytes; i++)
    *cp++ ^= (rand() >> 7) & 0xFF;
  return;
}

//
// Get the ethernet hardware address, if we can find it...
//
static int get_node_id(unsigned char *node_id)
{
#ifdef HAVE_NET_IF_H
  int     sd;
  struct ifreq  ifr, *ifrp;
  struct ifconf   ifc;
  char buf[1024];
  int   n, i;
  unsigned char   *a;
#ifdef HAVE_NET_IF_DL_H
  struct sockaddr_dl *sdlp;
#endif

  /*
   * BSD 4.4 defines the size of an ifreq to be
   * max(sizeof(ifreq), sizeof(ifreq.ifr_name)+ifreq.ifr_addr.sa_len
   * However, under earlier systems, sa_len isn't present, so the size is
   * just sizeof(struct ifreq)
   */
#ifdef HAVE_SA_LEN
#define ifreq_size(i) ni::Max(sizeof(struct ifreq),sizeof((i).ifr_name)+(i).ifr_addr.sa_len)
#else
#define ifreq_size(i) sizeof(struct ifreq)
#endif

  sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sd < 0) {
    return -1;
  }
  memset(buf, 0, sizeof(buf));
  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl (sd, SIOCGIFCONF, (char *)&ifc) < 0) {
    close(sd);
    return -1;
  }
  n = ifc.ifc_len;
  for (i = 0; i < n; i+= ifreq_size(*ifrp) ) {
    ifrp = (struct ifreq *)((char *) ifc.ifc_buf+i);
    strncpy(ifr.ifr_name, ifrp->ifr_name, IFNAMSIZ);
#ifdef SIOCGIFHWADDR
    if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
      continue;
    a = (unsigned char *) &ifr.ifr_hwaddr.sa_data;
#else
#ifdef SIOCGENADDR
    if (ioctl(sd, SIOCGENADDR, &ifr) < 0)
      continue;
    a = (unsigned char *) ifr.ifr_enaddr;
#else
#ifdef HAVE_NET_IF_DL_H
    sdlp = (struct sockaddr_dl *) &ifrp->ifr_addr;
    if ((sdlp->sdl_family != AF_LINK) || (sdlp->sdl_alen != 6))
      continue;
    a = (unsigned char *) &sdlp->sdl_data[sdlp->sdl_nlen];
#else
    /*
     * XXX we don't have a way of getting the hardware
     * address
     */
    close(sd);
    return 0;
#endif /* HAVE_NET_IF_DL_H */
#endif /* SIOCGENADDR */
#endif /* SIOCGIFHWADDR */
    if (!a[0] && !a[1] && !a[2] && !a[3] && !a[4] && !a[5])
      continue;
    if (node_id) {
      memcpy(node_id, a, 6);
      close(sd);
      return 1;
    }
  }
  close(sd);
#endif
  return 0;
}

/* Assume that the gettimeofday() has microsecond granularity */
#define MAX_ADJUSTMENT 10

static int get_clock(uint32_t *clock_high, uint32_t *clock_low, uint16_t *ret_clock_seq)
{
  static int      adjustment = 0;
  static struct timeval   last = {0, 0};
  static uint16_t     clock_seq;
  struct timeval      tv;
  unsigned long long    clock_reg;

try_again:
  gettimeofday(&tv, 0);
  if ((last.tv_sec == 0) && (last.tv_usec == 0)) {
    get_random_bytes(&clock_seq, sizeof(clock_seq));
    clock_seq &= 0x3FFF;
    last = tv;
    last.tv_sec--;
  }
  if ((tv.tv_sec < last.tv_sec) ||
      ((tv.tv_sec == last.tv_sec) &&
       (tv.tv_usec < last.tv_usec))) {
    clock_seq = (clock_seq+1) & 0x3FFF;
    adjustment = 0;
    last = tv;
  } else if ((tv.tv_sec == last.tv_sec) &&
             (tv.tv_usec == last.tv_usec)) {
    if (adjustment >= MAX_ADJUSTMENT)
      goto try_again;
    adjustment++;
  } else {
    adjustment = 0;
    last = tv;
  }

  clock_reg = tv.tv_usec*10 + adjustment;
  clock_reg += ((unsigned long long) tv.tv_sec)*10000000;
  clock_reg += (((unsigned long long) 0x01B21DD2) << 32) + 0x13814000;

  *clock_high = clock_reg >> 32;
  *clock_low = clock_reg;
  *ret_clock_seq = clock_seq;
  return 0;
}

static void uuid_generate_time(uuid_t out)
{
  static unsigned char node_id[6];
  static int has_init = 0;
  struct uuid uu;
  uint32_t  clock_mid;

  if (!has_init) {
    if (get_node_id(node_id) <= 0) {
      get_random_bytes(node_id, 6);
      /*
       * Set multicast bit, to prevent conflicts
       * with IEEE 802 addresses obtained from
       * network cards
       */
      node_id[0] |= 0x01;
    }
    has_init = 1;
  }
  niCAssert(sizeof(uu.time_low) == sizeof(uint32_t));
  get_clock(&clock_mid, (uint32_t*)&uu.time_low, &uu.clock_seq);
  uu.clock_seq |= 0x8000;
  uu.time_mid = (uint16_t) clock_mid;
  uu.time_hi_and_version = ((clock_mid >> 16) & 0x0FFF) | 0x1000;
  memcpy(uu.node, node_id, 6);
  uuid_pack(&uu, out);
}

static void uuid_pack(const struct uuid *uu, uuid_t ptr)
{
  tU32  tmp;
  unsigned char *out = ptr;

  tmp = uu->time_low;
  out[3] = (unsigned char) tmp;
  tmp >>= 8;
  out[2] = (unsigned char) tmp;
  tmp >>= 8;
  out[1] = (unsigned char) tmp;
  tmp >>= 8;
  out[0] = (unsigned char) tmp;

  tmp = uu->time_mid;
  out[5] = (unsigned char) tmp;
  tmp >>= 8;
  out[4] = (unsigned char) tmp;

  tmp = uu->time_hi_and_version;
  out[7] = (unsigned char) tmp;
  tmp >>= 8;
  out[6] = (unsigned char) tmp;

  tmp = uu->clock_seq;
  out[9] = (unsigned char) tmp;
  tmp >>= 8;
  out[8] = (unsigned char) tmp;

  memcpy(out+10, uu->node, 6);
}

#if 0 // Unused
static void uuid_unpack(const uuid_t in, struct uuid *uu)
{
  const tU8 *ptr = in;
  tU32    tmp;

  tmp = *ptr++;
  tmp = (tmp << 8) | *ptr++;
  tmp = (tmp << 8) | *ptr++;
  tmp = (tmp << 8) | *ptr++;
  uu->time_low = tmp;

  tmp = *ptr++;
  tmp = (tmp << 8) | *ptr++;
  uu->time_mid = tmp;

  tmp = *ptr++;
  tmp = (tmp << 8) | *ptr++;
  uu->time_hi_and_version = tmp;

  tmp = *ptr++;
  tmp = (tmp << 8) | *ptr++;
  uu->clock_seq = tmp;

  memcpy(uu->node, ptr, 6);
}

static void uuid_generate_random(uuid_t out)
{
  uuid_t  buf;
  struct uuid uu;

  get_random_bytes(buf, sizeof(buf));
  uuid_unpack(buf, &uu);

  uu.clock_seq = (uu.clock_seq & 0x3FFF) | 0x8000;
  uu.time_hi_and_version = (uu.time_hi_and_version & 0x0FFF) | 0x4000;
  uuid_pack(&uu, out);
}
#endif

namespace ni {

niExportFunc(tUUID) UUIDGenLocal() {
  uuid_t uuid;
  uuid_generate_time(uuid);
  return *(tUUID*)&uuid;
}

niExportFunc(tUUID) UUIDGenGlobal() {
  uuid_t uuid;
  uuid_generate_time(uuid);
  return *(tUUID*)&uuid;
}

}

//---------------------------------------------------------------------------
//
// Unknown
//
//---------------------------------------------------------------------------
#else

#  error ("Not Implemented.")

#endif
