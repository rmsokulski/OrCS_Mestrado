// noclone3: run a command with the clone3 syscall disabled.
//
// WORKAROUND for Pin 3.11 (2019): since glibc 2.34, pthread_create uses
// clone3, which Pin 3.11 does not support, so any multi-threaded program
// crashes under Pin. A seccomp filter makes clone3 fail with ENOSYS and
// glibc falls back to clone, which Pin handles.
// Remove this launcher once Pin is upgraded to a release with clone3 support.
//
// Usage: noclone3 <command> [args...]
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

int main(int argc, char **argv) {
    struct sock_filter filter[] = {
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_clone3, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | ENOSYS),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
    };
    struct sock_fprog prog = { sizeof(filter) / sizeof(filter[0]), filter };

    if (argc < 2) {
        fprintf(stderr, "usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0 ||
        prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog) != 0) {
        perror("noclone3: seccomp");
        return 1;
    }
    execvp(argv[1], argv + 1);
    perror("noclone3: execvp");
    return 1;
}
