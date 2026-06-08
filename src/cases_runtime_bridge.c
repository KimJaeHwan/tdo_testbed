#include "dfbench_sources_sinks.h"

/* DFB091: TLS value — supported on both Windows and POSIX */
#if defined(_MSC_VER)
__declspec(thread) static int g_tls_value = 0;
#else
static __thread int g_tls_value = 0;
#endif

DFB_CASE void case_DFB091_tls_value(void) {
    int a = dfb_source_A();
    g_tls_value = a;
    dfb_sink_int(g_tls_value);
}

/* DFB090, DFB092: POSIX-only — guarded by _WIN32 */
#if !defined(_WIN32)
#include <pthread.h>

/* DFB090 */
static volatile int g_thread_value = 0;

static void *dfb_thread_worker(void *arg) {
    int *p = (int *)arg;
    g_thread_value = *p;
    return 0;
}

DFB_CASE void case_DFB090_thread_shared_memory(void) {
    int a = dfb_source_A();
    pthread_t tid;
    if (pthread_create(&tid, 0, dfb_thread_worker, &a) == 0) {
        pthread_join(tid, 0);
    }
    dfb_sink_int(g_thread_value);
}

/* DFB092 */
typedef struct DFBThreadDispatch {
    int selected;
    int ignored;
} DFBThreadDispatch;

typedef void (*dfb_thread_target_t)(DFBThreadDispatch *ctx);

static void dfb_thread_target_A(DFBThreadDispatch *ctx) {
    dfb_sink_int(ctx->selected);
}

static void dfb_thread_target_B(DFBThreadDispatch *ctx) {
    DFB_TOUCH_INT(ctx->ignored);
}

static void *dfb_thread_dispatch_worker(void *arg) {
    DFBThreadDispatch *ctx = (DFBThreadDispatch *)arg;
    dfb_thread_target_t table[2];
    int selector;

    table[0] = dfb_thread_target_A;
    table[1] = dfb_thread_target_B;

    if ((dfb_source_C() & 1) == 0) {
        selector = 0;
    } else {
        selector = 1;
    }

    table[selector](ctx);
    return 0;
}

DFB_CASE void case_DFB092_pthread_table_dispatch(void) {
    DFBThreadDispatch ctx;
    pthread_t tid;

    ctx.selected = dfb_source_A();
    ctx.ignored  = dfb_source_B();

    if (pthread_create(&tid, 0, dfb_thread_dispatch_worker, &ctx) == 0) {
        pthread_join(tid, 0);
    }
}

#endif /* !_WIN32 */
