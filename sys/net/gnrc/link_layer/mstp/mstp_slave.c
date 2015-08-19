
int mstp_slave_intialize(mstp_t *ctx)
{
    if (ctx->initialized) {
        return 0;
    }

    uart_init(ctx->uart, ctx->baudrate, mstp_receive_frame, NULL, (void *) ctx);
    /* do initialization here */
    ctx->initialized = 1;
}

int mstp_slave_handle_ev(mstp_slave_t *ctx, uint16_t ev)
{
    switch (ctx->state) {
        case MSTP_SLAVE_IDLE:
            if (ev == MSTP_EV_RECEIVED_VALID_FRAME) {
                if (mstp_slave_frame_unwanted(ctx) == FALSE) {
                    /* handle wanted frame */
                    if (mstp_slave_frame_needs_reply(ctx)) {
                        /* handle for reply */
                        ctx->state = MSTP_SLAVE_ANSWER_DATA;
                        /* TODO: set timer to T_reply_delay */
                        mstp_slave_msg.type = MSTP_EV_SUCCESSFULL_RECEPTION;
                        msg_send_to_self(mstp_slave_msg);
                    }
                }
                else {
                    /* no reply */
                    ctx->frame->valid = 0;
                    mstp_slave_msg.type = MSTP_EV_SUCCESSFULL_RECEPTION;
                    msg_send_to_self(mstp_slave_msg);
                }
            }
        case MSTP_SLAVE_ANSWER_DATA:
            if (ev == MSTP_EV_SEND_FRAME) {
                /* TODO: disarm timer for T_reply_delay */
                mstp_send_frame(ctx);
                ctx->state = MSTP_SLAVE_IDLE;
            }
        default:
    }
}

int mstp_send_frame(ctx)
{
    /* mstp header */
    uart_write_blocking(ctx->uart, MSTP_DATA_PREAMBLE_1);
    uart_write_blocking(ctx->uart, MSTP_DATA_PREAMBLE_2);
    uart_write_blocking(ctx->uart, ctx->frame->type);
    uart_write_blocking(ctx->uart, ctx->frame->dst_addr);
    uart_write_blocking(ctx->uart, ctx->frame->src_addr);
    uart_write_blocking(ctx->uart, (ctx->frame->length>>8));
    uart_write_blocking(ctx->uart, (ctx->frame->length&0xff));
    uart_write_blocking(ctx->uart, ctx->frame->header_crc);

    /* mstp data */
    for (uint16_t i = 0; i < ctx->frame->length; i++) {
        uart_write_blocking(ctx->uart, ctx->frame->data[i]);
    }
    uart_write_blocking(ctx->uart, (ctx->frame->data_crc>>8));
    uart_write_blocking(ctx->uart, (ctx->frame->data_crc&0xff));
}

int mstp_slave_frame_needs_reply(mstp_slave_t *ctx)
{
    /* don't reply to broadcast frames */
    if ((ctx->frame->dst == ctx->addr)
        &&(ctx->frame->type == MSTP_FRAME_TYPE_DATA_EXPECTING_REPLY
           || ctx->frame->type == MSTP_FRAME_TYPE_TEST_REQUEST)) 
    {
        return 1;
    }
    return 0;
}

int mstp_slave_frame_unwanted(mstp_slave_t *ctx)
{
    /* not for us */
    if (ctx->frame->dst != ctx->addr
        || ctx->frame->dst != MSTP_BROADCAST_ADDR) {
        return 1;
    }
    /* don't reply to broadcast frames */
    if (ctx->frame->dst == MSTP_BROADCAST_ADDR
        && ( ctx->frame->type != MSTP_FRAME_TYPE_DATA_EXPECTING_REPLY
            || ctx->frame->type != MSTP_FRAME_TYPE_TEST_REQUEST)) {
        return 1;
    }
    if (ctx->frame->type != MSTP_FRAME_TYPE_DATA_EXPECTING_REPLY
        || ctx->frame->type != MSTP_FRAME_TYPE_TEST_REQUEST
        || ctx->frame->type != MSTP_FRAME_TYPE_TEST_RESPONSE
        || ctx->frame->type != MSTP_FRAME_TYPE_DATA_NOT_EXPECTING_REPLY) {
        return 1;
    }

    return 0;
}

/**
 * @brief   Startup code and event loop of the MS/TP layer
 * 
 * @param[in] args          expects a pointer to the MS/TP context
 *
 * @return                  never returns
 */
static void *_mstp_slave_thread(void *args)
{
    mstp_slave_t *ctx = (mstp_slave_t *)args;
    ng_netapi_opt_t *opt;
    int res;
    msg_t msg, reply, msg_queue[MSTP_SLAVE_MSG_QUEUE_SIZE];

    /* setup the MAC layers message queue */
    msg_init_queue(msg_queue, NG_NOMAC_MSG_QUEUE_SIZE);
    /* save the PID to the device descriptor and register the device */
    ctx->mac_pid = thread_getpid();
    ng_netif_add(ctx->mac_pid);
    /* register the event callback with the device driver */
    mstp_slave_intialize(ctx);

    /* start the event loop */
    while (1) {
        DEBUG("mstp_slave: waiting for incoming messages\n");
        msg_receive(&msg);
        /* dispatch NETDEV and NETAPI messages */
        switch (msg.type) {
            case NG_NETAPI_MSG_TYPE_SND:
                DEBUG("mstp slave: NG_NETAPI_MSG_TYPE_SND received (NOT IMPLEMENTED)\n");
                // dev->driver->send_data(dev, (ng_pktsnip_t *)msg.content.ptr);
                break;
            case NG_NETAPI_MSG_TYPE_SET:
                /* TODO: filter out MAC layer options -> for now forward
                         everything to the device driver */
                DEBUG("mstp slave: NG_NETAPI_MSG_TYPE_SET received\n");
                /* read incoming options */
                opt = (ng_netapi_opt_t *)msg.content.ptr;
                /* set option for device driver */
                res = _set(ctx, opt->opt, opt->data, opt->data_len);
                DEBUG("mstp slave: response of netdev->set: %i\n", res);
                /* send reply to calling thread */
                reply.type = NG_NETAPI_MSG_TYPE_ACK;
                reply.content.value = (uint32_t)res;
                msg_reply(&msg, &reply);
                break;
            case NG_NETAPI_MSG_TYPE_GET:
                /* TODO: filter out MAC layer options -> for now forward
                         everything to the device driver */
                DEBUG("mstp slave: NG_NETAPI_MSG_TYPE_GET received\n");
                /* read incoming options */
                opt = (ng_netapi_opt_t *)msg.content.ptr;
                /* get option from device driver */
                res = _get(ctx, opt->opt, opt->data, opt->data_len);
                DEBUG("mstp slave: response of netdev->get: %i\n", res);
                /* send reply to calling thread */
                reply.type = NG_NETAPI_MSG_TYPE_ACK;
                reply.content.value = (uint32_t)res;
                msg_reply(&msg, &reply);
                break;
            case MSTP_EV_RECEIVED_VALID_FRAME:
                DEBUG("mstp slave: MSTP_EV_RECEIVED_VALID_FRAME received\n");
                mstp_slave_handle_ev(ctx, MSTP_EV_RECEIVED_VALID_FRAME);
                break;
            case MSTP_EV_SUCCESSFULL_RECEPTION:
                DEBUG("mstp slave: MSTP_EV_SUCCESSFULL_RECEPTION received\n");
                ng_pktsnip_t *pkt = ng_pktbuf_add(NULL, NULL, ctx->frame->length + 5, NG_NETTYPE_UNDEF);
                mstp_snipify_pkt(ctx, pkt);
                /* send the packet to everyone interested in it's type */
                if (!ng_netapi_dispatch_receive(pkt->type, NG_NETREG_DEMUX_CTX_ALL, pkt)) {
                    DEBUG("mstp slave: unable to forward packet of type %i\n", pkt->type);
                    ng_pktbuf_release(pkt);
                }

            default:
                DEBUG("mstp slave: Unknown command %" PRIu16 "\n", msg.type);
                break;
        }
    }
    /* never reached */
    return NULL;
}

static int _set(mstp_slave_t *ctx, netopt_t opt, void *val, size_t max_len)
{
    if (device == NULL) {
        return -ENODEV;
    }

    switch (opt) {
        default:
            return -ENOTSUP;
    }
}

static int _get(mstp_slave_t *ctx, netopt_t opt, void *val, size_t max_len)
{
    if (device == NULL) {
        return -ENODEV;
    }

    switch (opt) {
        default:
            return -ENOTSUP;
    }
}

kernel_pid_t ng_mstp_slave_init(char *stack, int stacksize, char priority,
                           const char *name, mstp_slave_t *ctx)
{
    kernel_pid_t res;

    /* check if given mstp context is defined and the frame to use is set */
    if (ctx == NULL || dev->frame == NULL) {
        return -ENODEV;
    }
    /* create new NOMAC thread */
    res = thread_create(stack, stacksize, priority, CREATE_STACKTEST,
                        _mstp_slave_thread, (void *)ctx, name);
    if (res <= 0) {
        return -EINVAL;
    }
    return res;
}
