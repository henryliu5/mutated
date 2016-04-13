#ifndef MUTATED_SOCKET_BUF_HH
#define MUTATED_SOCKET_BUF_HH

/**
 * socket_buf.hh - async socket I/O support. A variant that uses circular
 * buffers internally for memory management of the rx and tx queues.
 */

#include <cstdint>
#include <cstring>
#include <functional>
#include <utility>

#include <limits.h>

#include "buffer.hh"

class Sock;

/**
 * An IO operation.
 */
class ioop
{
  public:
    using ioop_cb =
      std::function<void(Sock *, void *, char *, size_t, char *, size_t, int)>;

    size_t len;
    void *cb_data;
    ioop_cb cb;

    ioop(void) noexcept : len{0}, cb_data{nullptr}, cb{} {}

    ioop(size_t len_, void *data_, ioop_cb complete_) noexcept
      : len{len_},
        cb_data{data_},
        cb{complete_}
    {
    }

    ioop(const ioop &) = default;
    ioop &operator=(const ioop &) = default;
    ~ioop(void) noexcept {}
};

/**
 * Asynchronous socket (TCP only). Uses circular buffers internally for
 * managing rx and tx queues.
 */
class Sock
{
  public:
    /* Maximum number of outstanding read IO operations */
    using ioqueue = buffer<ioop, 1024>;

  private:
    int fd_;             /* the file descriptor */
    unsigned short port; /* port connected to */
    bool connected;      /* is the socket connected? */
    bool rx_rdy;         /* ready to read? */
    bool tx_rdy;         /* ready to write? */

    ioqueue rxcbs; /* read queue */
    charbuf rbuf;  /* read buffer */
    charbuf wbuf;  /* write buffer */

    void rx(void); /* receive handler */
    void tx(void); /* transmit handler */

  public:
    Sock(void) noexcept;
    ~Sock(void) noexcept;

    /* Disable copy and move */
    Sock(const Sock &) = delete;
    Sock(Sock &&) = delete;
    Sock operator=(const Sock &) = delete;
    Sock operator=(Sock &&) = delete;

    /* Access underlying file descriptor */
    int fd(void) const noexcept { return fd_; }

    /* Open a new remote connection */
    void connect(const char *addr, unsigned short port);

    /* Read queuing */
    void read(const ioop &);

    /* Write queuing */
    std::pair<char *, char *> write_prepare(size_t &len);
    void write_commit(const size_t len);

    /* Handle epoll events against this socket */
    void run_io(uint32_t events);
};

#endif /* MUTATED_SOCKET_BUF_HH */
