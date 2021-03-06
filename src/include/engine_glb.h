#ifndef ENGINE_GLB_H
#define ENGINE_GLB_H

/**
 * 定义数据报文格式
 *
 * 注意, 不同的引擎可能包含不同的值
 *  1) 普通引擎
 *      不关注PKT_INFO->eth_hdr/ip_hdr/udp_hdr
 *      PKT->data/data_len对应DNS数据
 *
 *  2) DPDK引擎
 *      关注PKT_INFO->eth_hdr/ip_hdr/udp_hdr
 *      PKT->data/data_len对应L2-L4 + DNS数据
 */
typedef struct st_pkt_info {
    /* 底层引擎关注的字段 */
    void *eth_hdr;
    void *ip_hdr;
    void *udp_hdr;

    /* 高层包处理 */
    void *dns_hdr;
    char *cur_pos;              /* 当前处理位置 */

    /* acl规则 */
    union{
        uint32_t ip4;
    }src_ip;                    /* 源IP, 网络字节序 */

    /* 查询条件 */
    char domain[DOMAIN_LEN_MAX];
    uint16_t q_type;
    uint16_t q_class;

    /* 记录结果 */
    union {                     /* A记录, 网络字节序 */
        uint32_t ip4;
    }rr_res[RR_PER_TYPE_MAX];
    uint32_t rr_res_ttl[RR_PER_TYPE_MAX];
    int rr_res_cnt;
}PKT_INFO;
typedef struct st_pkt {
    PKT_INFO info;              /* 信息区, 存放解析中间结果, PKT_INFO */
    char *data;                 /* 报文正文 */
    int data_len;               /* 报文长度 */
}PKT;

/**
 * 初始化报文收发引擎, 一般在主进程调用, 如DPDK环境等
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int pkt_engine_init(void);

/**
 * 第二阶段初始化报文收发引擎, 一般在主worker进程调用,
 * 如普通的SOCKET插口操作等
 */
int pkt_engine_init_2(void);

/**
 * 启动报文收发引擎, 如DPDK环境等
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int start_pkt_engine(void);

/**
 * 收/发报文
 * @param pkt: [in][out], 报文指针
 * @retval: RET_OK/RET_ERR
 *
 * @note
 *  1) 普通引擎报文为串行处理, 因此这两个函数应该成对出现, 并且顺序为
 *      先收后发
 */
int receive_pkt(PKT **pkt);
int send_pkt(PKT *pkt);

#endif
