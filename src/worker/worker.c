#include "util_glb.h"
#include "engine_glb.h"
#include "worker_glb.h"
#include "log_glb.h"
#include "worker.h"


void start_other_worker(GLB_VARS *glb_vars)
{
    return;
}

int process_mesg(GLB_VARS *glb_vars, PKT *pkt)
{
    if (pkt == NULL || glb_vars == NULL) {
        SDNS_LOG_ERR("param err");
        return RET_ERR;
    }

    return RET_OK;
}

void start_worker(GLB_VARS *glb_vars)
{
    /**
     * 多进程同时监听某个UDP插口, 需要在一个进程建立socket, 然后通过
     * fork()的方式建立其他进程, 这样直接继承建立的socket; 当然多个
     * 进程通过阻塞读方式会有"惊群"现象.
     *
     * 如果在多个进程分别调用socket/bind()建立插口, 则只有第一个进程
     * 能接收到报文, 其他进程不响应!!!
     *
     * 如果性能问题, 后续采用epoll事件分发模型, 主worker收, 其他worker
     * 处理!
     */
    pid_t child_pid = 0;

    /* 启动第一个worker进程 */
    child_pid = fork();
    if (child_pid) {
        SDNS_LOG_DEBUG("In parent, fork worker[%d]", child_pid);
        return;
    }

    /* 引擎第二阶段初始化 */
    if (pkt_engine_init_2() == RET_ERR) {
        SDNS_LOG_DEBUG("In worker[%d], engine init failed", getpid());
        return;
    }

    /* 启动其他worker进程 */
    start_other_worker(glb_vars);

    /* 主处理流程 */
    for (;;) {
        PKT *pkt = NULL;
    
        /* 接收报文 */
        if (receive_pkt(&pkt) == RET_ERR) {
            SDNS_LOG_WARN("receive pkt failed");
            continue;
        }

        /* 处理数据 */
        if (process_mesg(glb_vars, pkt) == RET_ERR) {
            SDNS_LOG_WARN("process failed");
            continue;
        }

        /* 忽略发送失败 */
        (void)send_pkt(pkt);
    }
}