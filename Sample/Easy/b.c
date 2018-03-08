#include <module.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>

MODULE(B);

/*
 * Initializes this module's state;
 * returns a valid fd to be polled.
 */
static int init(void) {
    sigset_t mask;
    
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    int fd = signalfd(-1, &mask, 0);
    return fd;
}

/* 
 * Whether this module should be actually created:
 * 0 means OK -> start this module
 * !0 means do not start.
 * 
 * Use this function as a starting filter: 
 * you may desire that a module is not started in certain conditions.
 */
static int check(void) {
    return 0;
}

/* 
 * Should return not-0 value when module can be actually started (and thus polled).
 * Use this to check intra-modules dependencies or any other env variable.
 * 
 * Eg: you can evaluate your global state to make this module start right after
 * certain conditions are met.
 */
static int evaluate(void) {
    return 1;
}

/*
 * Destroyer function, called at module unload (at end of program).
 */
static void destroy(void) {
    
}

/*
 * Our default poll callback.
 * This function can be called from poll or because another MODULE
 * sent a message to this module through m_tell().
 */
static void recv(message_t *msg, const void *userdata) {
    if (!msg->message) {
        struct signalfd_siginfo fdsi;    
        ssize_t s = read(msg->fd, &fdsi, sizeof(struct signalfd_siginfo));
        if (s != sizeof(struct signalfd_siginfo)) {
            m_log("an error occurred while getting signalfd data.\n");
        }
        m_log("received signal %d. Leaving.\n", fdsi.ssi_signo);
        modules_quit();
    }
}