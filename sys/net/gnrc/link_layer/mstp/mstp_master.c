
/**
 * @brief   Startup code and event loop of the MS/TP layer
 * 
 * @param[in] args          expects a pointer to the MS/TP context
 *
 * @return                  never returns
 */
static void *_mstp_master_thread(void *args)
{
    mstp_master_t *ctx = (mstp_master_t *)args;
    ng_netapi_opt_t *opt;
    int res;
    msg_t msg, reply, msg_queue[MSTP_MASTER_MSG_QUEUE_SIZE];

    /* setup the MAC layers message queue */
    msg_init_queue(msg_queue, NG_NOMAC_MSG_QUEUE_SIZE);
    /* save the PID to the device descriptor and register the device */
    ctx->mac_pid = thread_getpid();
    ng_netif_add(ctx->mac_pid);
    /* register the event callback with the device driver */
    mstp_master_intialize(ctx);

    /* start the event loop */
    while (1) {
        DEBUG("mstp master: waiting for incoming messages\n");
        msg_receive(&msg);
        /* dispatch NETDEV and NETAPI messages */
        switch (msg.type) {
            case NG_NETAPI_MSG_TYPE_SND:
                DEBUG("mstp master: NG_NETAPI_MSG_TYPE_SND received (NOT IMPLEMENTED)\n");
                // dev->driver->send_data(dev, (ng_pktsnip_t *)msg.content.ptr);
                break;
            case NG_NETAPI_MSG_TYPE_SET:
                /* TODO: filter out MAC layer options -> for now forward
                         everything to the device driver */
                DEBUG("mstp master: NG_NETAPI_MSG_TYPE_SET received\n");
                /* read incoming options */
                opt = (ng_netapi_opt_t *)msg.content.ptr;
                /* set option for device driver */
                res = _set(ctx, opt->opt, opt->data, opt->data_len);
                DEBUG("mstp master: response of netdev->set: %i\n", res);
                /* send reply to calling thread */
                reply.type = NG_NETAPI_MSG_TYPE_ACK;
                reply.content.value = (uint32_t)res;
                msg_reply(&msg, &reply);
                break;
            case NG_NETAPI_MSG_TYPE_GET:
                /* TODO: filter out MAC layer options -> for now forward
                         everything to the device driver */
                DEBUG("mstp master: NG_NETAPI_MSG_TYPE_GET received\n");
                /* read incoming options */
                opt = (ng_netapi_opt_t *)msg.content.ptr;
                /* get option from device driver */
                res = _get(ctx, opt->opt, opt->data, opt->data_len);
                DEBUG("mstp master: response of netdev->get: %i\n", res);
                /* send reply to calling thread */
                reply.type = NG_NETAPI_MSG_TYPE_ACK;
                reply.content.value = (uint32_t)res;
                msg_reply(&msg, &reply);
                break;
            case MSTP_EV_RECEIVED_VALID_FRAME:
                DEBUG("mstp master: MSTP_EV_RECEIVED_VALID_FRAME received\n");
                mstp_master_handle_ev(ctx, MSTP_EV_RECEIVED_VALID_FRAME);
                break;
            case MSTP_EV_SUCCESSFULL_RECEPTION:
                DEBUG("mstp master: MSTP_EV_SUCCESSFULL_RECEPTION received\n");
                ng_pktsnip_t *pkt = ng_pktbuf_add(NULL, NULL, ctx->frame->length + 5, NG_NETTYPE_UNDEF);
                mstp_snipify_pkt(ctx, pkt);
                /* send the packet to everyone interested in it's type */
                if (!ng_netapi_dispatch_receive(pkt->type, NG_NETREG_DEMUX_CTX_ALL, pkt)) {
                    DEBUG("mstp master: unable to forward packet of type %i\n", pkt->type);
                    ng_pktbuf_release(pkt);
                }

            default:
                DEBUG("mstp master: Unknown command %" PRIu16 "\n", msg.type);
                break;
        }
    }
    /* never reached */
    return NULL;
}

static int _set(mstp_master_t *ctx, netopt_t opt, void *val, size_t max_len)
{
    if (device == NULL) {
        return -ENODEV;
    }

    switch (opt) {
        default:
            return -ENOTSUP;
    }
}

static int _get(mstp_master_t *ctx, netopt_t opt, void *val, size_t max_len)
{
    if (device == NULL) {
        return -ENODEV;
    }

    switch (opt) {
        default:
            return -ENOTSUP;
    }
}

kernel_pid_t ng_mstp_master_init(char *stack, int stacksize, char priority,
                                 const char *name, mstp_master_t *ctx)
{
    kernel_pid_t res;

    /* check if given mstp context is defined and the frame to use is set */
    if (ctx == NULL || dev->frame == NULL) {
        return -ENODEV;
    }
    /* create new mstp master thread */
    res = thread_create(stack, stacksize, priority, CREATE_STACKTEST,
                        _mstp_master_thread, (void *)ctx, name);
    if (res <= 0) {
        return -EINVAL;
    }
    return res;
}
